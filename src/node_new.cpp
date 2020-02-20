/////////////////////////////////////////////////////////////////////////
#include "node_embed.h"
/////////////////////////////////////////////////////////////////////////

#include "tracing/traced_value.h"

#if HAVE_OPENSSL
#include "node_crypto.h"
#endif

#include "ares.h"
#include "nghttp2/nghttp2ver.h"
#include "tracing/node_trace_writer.h"
#include "zlib.h"

#if defined(__POSIX__)
#include <dlfcn.h>
#endif

#define READONLY_PROPERTY(obj, str, var)                                      \
  do {                                                                        \
    obj->DefineOwnProperty(env->context(),                                    \
                           OneByteString(env->isolate(), str),                \
                           var,                                               \
                           ReadOnly).FromJust();                              \
  } while (0)


namespace node {

	using v8::Context;
	using v8::EscapableHandleScope;
	using v8::HandleScope;
	using v8::Isolate;
	using v8::Local;
	using v8::Locker;
	using v8::MaybeLocal;
	using v8::Object;
	using v8::ReadOnly;
	using v8::Script;
	using v8::ScriptOrigin;
	using v8::SealHandleScope;
	using v8::String;
	using v8::TracingController;
	using v8::TryCatch;
	using v8::V8;
	using v8::Value;

	static bool v8_is_profiling = false;

	static Mutex node_isolate_mutex;
	static Isolate* node_isolate;

	// Ensures that __metadata trace events are only emitted
	// when tracing is enabled.
	class NodeTraceStateObserver :
		public TracingController::TraceStateObserver {
	public:
		void OnTraceEnabled() override;

		void OnTraceDisabled() override;

		explicit NodeTraceStateObserver(TracingController* controller) :
			controller_(controller) {}
		~NodeTraceStateObserver() override {}

	private:
		TracingController* controller_;
	};

	static struct {
#if NODE_USE_V8_PLATFORM
		void Initialize(int thread_pool_size) {
			tracing_agent_.reset(new tracing::Agent());
			auto controller = tracing_agent_->GetTracingController();
			controller->AddTraceStateObserver(new NodeTraceStateObserver(controller));
			tracing::TraceEventHelper::SetTracingController(controller);
			StartTracingAgent();
			// Tracing must be initialized before platform threads are created.
			platform_ = new NodePlatform(thread_pool_size, controller);
			V8::InitializePlatform(platform_);
		}

		void Dispose() {
			platform_->Shutdown();
			delete platform_;
			platform_ = nullptr;
			// Destroy tracing after the platform (and platform threads) have been
			// stopped.
			tracing_agent_.reset(nullptr);
		}

		void DrainVMTasks(Isolate* isolate) {
			platform_->DrainTasks(isolate);
		}

		void CancelVMTasks(Isolate* isolate) {
			platform_->CancelPendingDelayedTasks(isolate);
		}

#if HAVE_INSPECTOR
		bool StartInspector(Environment* env, const char* script_path,
			std::shared_ptr<DebugOptions> options) {
			// Inspector agent can't fail to start, but if it was configured to listen
			// right away on the websocket port and fails to bind/etc, this will return
			// false.
			return env->inspector_agent()->Start(
				script_path == nullptr ? "" : script_path, options, true);
		}

		bool InspectorStarted(Environment* env) {
			return env->inspector_agent()->IsListening();
		}
#endif  // HAVE_INSPECTOR

		void StartTracingAgent() {
			if (per_process_opts->trace_event_categories.empty()) {
				tracing_file_writer_ = tracing_agent_->DefaultHandle();
			}
			else {
				tracing_file_writer_ = tracing_agent_->AddClient(
					ParseCommaSeparatedSet(per_process_opts->trace_event_categories),
					std::unique_ptr<tracing::AsyncTraceWriter>(
						new tracing::NodeTraceWriter(
							per_process_opts->trace_event_file_pattern)),
					tracing::Agent::kUseDefaultCategories);
			}
		}

		void StopTracingAgent() {
			tracing_file_writer_.reset();
		}

		tracing::AgentWriterHandle* GetTracingAgentWriter() {
			return &tracing_file_writer_;
		}

		NodePlatform* Platform() {
			return platform_;
		}

		std::unique_ptr<tracing::Agent> tracing_agent_;
		tracing::AgentWriterHandle tracing_file_writer_;
		NodePlatform* platform_;
#else  // !NODE_USE_V8_PLATFORM
		void Initialize(int thread_pool_size) {}
		void Dispose() {}
		void DrainVMTasks(Isolate* isolate) {}
		void CancelVMTasks(Isolate* isolate) {}

		void StartTracingAgent() {
			if (!trace_enabled_categories.empty()) {
				fprintf(stderr, "Node compiled with NODE_USE_V8_PLATFORM=0, "
					"so event tracing is not available.\n");
			}
		}
		void StopTracingAgent() {}

		tracing::AgentWriterHandle* GetTracingAgentWriter() {
			return nullptr;
		}

		NodePlatform* Platform() {
			return nullptr;
		}
#endif  // !NODE_USE_V8_PLATFORM

#if !NODE_USE_V8_PLATFORM || !HAVE_INSPECTOR
		bool InspectorStarted(Environment* env) {
			return false;
		}
#endif  //  !NODE_USE_V8_PLATFORM || !HAVE_INSPECTOR
	} v8_platform;


#ifdef __POSIX__
	static const unsigned kMaxSignal = 32;
#endif


	void RunBeforeExit(Environment* env);

	static void StartInspector(Environment* env, const char* path,
		std::shared_ptr<DebugOptions> debug_options) {
#if HAVE_INSPECTOR
		CHECK(!env->inspector_agent()->IsListening());
		v8_platform.StartInspector(env, path, debug_options);
#endif  // HAVE_INSPECTOR
	}

	static void ReportException(Environment* env, const TryCatch& try_catch) {
		ReportException(env, try_catch.Exception(), try_catch.Message());
	}

	static void WaitForInspectorDisconnect(Environment* env) {
#if HAVE_INSPECTOR
		if (env->inspector_agent()->IsActive()) {
			// Restore signal dispositions, the app is done and is no longer
			// capable of handling signals.
#if defined(__POSIX__) && !defined(NODE_SHARED_MODE)
			struct sigaction act;
			memset(&act, 0, sizeof(act));
			for (unsigned nr = 1; nr < kMaxSignal; nr += 1) {
				if (nr == SIGKILL || nr == SIGSTOP || nr == SIGPROF)
					continue;
				act.sa_handler = (nr == SIGPIPE) ? SIG_IGN : SIG_DFL;
				CHECK_EQ(0, sigaction(nr, &act, nullptr));
			}
#endif
			env->inspector_agent()->WaitForDisconnect();
		}
#endif
	}


	// Executes a str within the current v8 context.
	static MaybeLocal<Value> ExecuteString(Environment* env,
		Local<String> source,
		Local<String> filename) {
		EscapableHandleScope scope(env->isolate());
		TryCatch try_catch(env->isolate());

		// try_catch must be nonverbose to disable FatalException() handler,
		// we will handle exceptions ourself.
		try_catch.SetVerbose(false);

		ScriptOrigin origin(filename);
		MaybeLocal<Script> script =
			Script::Compile(env->context(), source, &origin);
		if (script.IsEmpty()) {
			ReportException(env, try_catch);
			env->Exit(3);
			return MaybeLocal<Value>();
		}

		MaybeLocal<Value> result = script.ToLocalChecked()->Run(env->context());
		if (result.IsEmpty()) {
			if (try_catch.HasTerminated()) {
				env->isolate()->CancelTerminateExecution();
				return MaybeLocal<Value>();
			}
			ReportException(env, try_catch);
			env->Exit(4);
			return MaybeLocal<Value>();
		}

		return scope.Escape(result.ToLocalChecked());
	}

	/////////////////////////////////////////////////////////////////////////
	// Code above this line is excerpted from src/node.cc, with the notable
	// exception of the definition of node_context_struct.
	//
	// Code below this line is generally new and/or rearranged.
	/////////////////////////////////////////////////////////////////////////

	inline node_context *Setup(Isolate* isolate, IsolateData* isolate_data,
		const std::vector<std::string>& args,
		const std::vector<std::string>& exec_args) {
		HandleScope handle_scope(isolate);
		Local<Context> context = NewContext(isolate);
		Context::Scope context_scope(context);
		Environment *env =
			new Environment(isolate_data, context, v8_platform.GetTracingAgentWriter());
		env->Start(args, exec_args, v8_is_profiling);

		Local<Object> process = env->process_object();
		READONLY_PROPERTY(process, "_embed", True(env->isolate()));

		const char* path = args.size() > 1 ? args[1].c_str() : nullptr;
		StartInspector(env, path, env->options()->debug_options);

		if (env->options()->debug_options->inspector_enabled &&
			!v8_platform.InspectorStarted(env)) {
			return NULL;  // Signal internal error.
		}

		auto context_struct = new node_context({ env, NULL });

		{
			Environment::AsyncCallbackScope callback_scope(env);
			env->async_hooks()->push_async_ids(1, 0);
			LoadEnvironment(env);
			env->async_hooks()->pop_async_id(1);
		}

		return context_struct;
	}

	int Teardown(Environment *env) {
		env->set_trace_sync_io(false);

		const int exit_code = EmitExit(env);

		WaitForInspectorDisconnect(env);

		env->set_can_call_into_js(false);
		env->stop_sub_worker_contexts();
		uv_tty_reset_mode();
		env->RunCleanup();
		RunAtExit(env);
		delete env;

		return exit_code;
	}


	inline node_context *Setup(uv_loop_t* event_loop,
		const std::vector<std::string>& args,
		const std::vector<std::string>& exec_args) {
		ArrayBufferAllocator *allocator = CreateArrayBufferAllocator();
		Isolate* const isolate = NewIsolate(allocator, event_loop);
		if (isolate == nullptr)
			return NULL;  // Signal internal error.

		{
			Mutex::ScopedLock scoped_lock(node_isolate_mutex);
			CHECK_NULL(node_isolate);
			node_isolate = isolate;
		}

		IsolateData *isolate_data;
		node_context *context_struct;

		{
			Locker locker(isolate);
			Isolate::Scope isolate_scope(isolate);
			HandleScope handle_scope(isolate);
			isolate_data = CreateIsolateData(
				isolate,
				event_loop,
				v8_platform.Platform(),
				allocator);
			// TODO(addaleax): This should load a real per-Isolate option, currently
			// this is still effectively per-process.
			if (isolate_data->options()->track_heap_objects) {
				isolate->GetHeapProfiler()->StartTrackingHeapObjects(true);
			}
			context_struct = Setup(isolate, isolate_data, args, exec_args);
			context_struct->allocator = allocator;
		}
		return context_struct;
	}

	inline int Teardown(node_context *context_struct) {
		int exit_code;
		Isolate *isolate = context_struct->env->isolate();

		{
			Locker locker(isolate);
			exit_code = Teardown(context_struct->env);

			v8_platform.DrainVMTasks(isolate);
			v8_platform.CancelVMTasks(isolate);
#if defined(LEAK_SANITIZER)
			__lsan_do_leak_check();
#endif
		}

		{
			Mutex::ScopedLock scoped_lock(node_isolate_mutex);
			CHECK_EQ(node_isolate, isolate);
			node_isolate = nullptr;
		}

		isolate->Dispose();
		v8_platform.Platform()->UnregisterIsolate(isolate);
		FreeIsolateData(context_struct->env->isolate_data());
		FreeArrayBufferAllocator(context_struct->allocator);
		delete context_struct;

		return exit_code;
	}

	extern void PlatformInit();

	void Init(std::vector<std::string>* argv,
		std::vector<std::string>* exec_argv);

	void Dispose() {
		v8_platform.StopTracingAgent();
		v8_initialized = false;
		V8::Dispose();

		// uv_run cannot be called from the time before the beforeExit callback
		// runs until the program exits unless the event loop has any referenced
		// handles after beforeExit terminates. This prevents unrefed timers
		// that happen to terminate during shutdown from being run unsafely.
		// Since uv_run cannot be called, uv_async handles held by the platform
		// will never be fully cleaned up.
		v8_platform.Dispose();
	}

}  // namespace node

#if !HAVE_INSPECTOR
void Initialize() {}

NODE_BUILTIN_MODULE_CONTEXT_AWARE(inspector, Initialize)
#endif  // !HAVE_INSPECTOR


#ifdef __cplusplus
extern "C" {
#endif

	node_context *nodeSetup(int argc, char** argv) {
		atexit([]() { uv_tty_reset_mode(); });
		node::PlatformInit();
		node::performance::performance_node_start = PERFORMANCE_NOW();

		CHECK_GT(argc, 0);

		// Hack around with the argv pointer. Used for process.title = "blah".
		argv = uv_setup_args(argc, argv);

		std::vector<std::string> args(argv, argv + argc);
		std::vector<std::string> exec_args;

		// add "-e ''" to args if no execute is provided and not explicitly
		// interactive
		bool default_interactive = true;
		for (auto const& arg : args) {
			if (!arg.compare("-") && !arg.compare("-i") &&
				!arg.compare("--interactive") && !arg.compare("-e") &&
				!arg.compare("--eval"))
				default_interactive = false;
		}
		if (default_interactive) {
			args.push_back(std::string("-e"));
			args.push_back(std::string(""));
		}

		// This needs to run *before* V8::Initialize().
		node::Init(&args, &exec_args);

#if HAVE_OPENSSL
		{
			std::string extra_ca_certs;
			if (node::SafeGetenv("NODE_EXTRA_CA_CERTS", &extra_ca_certs))
				node::crypto::UseExtraCaCerts(extra_ca_certs);
		}
#ifdef NODE_FIPS_MODE
		// In the case of FIPS builds we should make sure
		// the random source is properly initialized first.
		OPENSSL_init();
#endif  // NODE_FIPS_MODE
		// V8 on Windows doesn't have a good source of entropy. Seed it from
		// OpenSSL's pool.
		v8::V8::SetEntropySource(node::crypto::EntropySource);
#endif  // HAVE_OPENSSL

		// FIXME(rubys) - need to update static struct too
		node::v8_platform.platform_ = (node::NodePlatform *)
			node::InitializeV8Platform(node::per_process_opts->v8_thread_pool_size);
		v8::V8::Initialize();
		node::performance::performance_v8_start = PERFORMANCE_NOW();
		node::v8_initialized = true;

		node_context* context = node::Setup(uv_default_loop(), args, exec_args);

		if (!context) node::Dispose();

		return context;
	}

	void nodeExecuteString(node_context *node_context,
		const char *source,
		const char *filename) {
		node::Environment *env = node_context->env;
		v8::Isolate *isolate = env->isolate();
		v8::Locker locker(isolate);

		v8::HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = node::NewContext(isolate);
		v8::Context::Scope context_scope(context);

		v8::Local<v8::String> sourceString =
			v8::String::NewFromUtf8(isolate, source, v8::NewStringType::kNormal)
			.ToLocalChecked();
		v8::Local<v8::String> filenameString =
			v8::String::NewFromUtf8(isolate, filename, v8::NewStringType::kNormal)
			.ToLocalChecked();
		// TODO(rubys) capture and return result:
		ExecuteString(env, sourceString, filenameString);

		{
			v8::SealHandleScope seal(isolate);
			bool more;
			env->performance_state()->Mark(
				node::performance::NODE_PERFORMANCE_MILESTONE_LOOP_START);
			do {
				uv_run(env->event_loop(), UV_RUN_DEFAULT);

				node::v8_platform.DrainVMTasks(isolate);

				more = uv_loop_alive(env->event_loop());
				if (more)
					continue;

				RunBeforeExit(env);

				// Emit `beforeExit` if the loop became alive either after emitting
				// event, or after running some callbacks.
				more = uv_loop_alive(env->event_loop());
			} while (more == true);
			env->performance_state()->Mark(
				node::performance::NODE_PERFORMANCE_MILESTONE_LOOP_EXIT);
		}
	}

	extern int nodeTeardown(node_context *context_struct) {
		int exit_code = node::Teardown(context_struct);
		node::Dispose();
		return exit_code;
	}

#ifdef __cplusplus
}
#endif