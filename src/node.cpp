#define NODE_WANT_INTERNALS 1
#define USING_UV_SHARED 1

#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "libplatform/libplatform.h"
#include "tracing/trace_event.h"

node::Environment* m_env;
v8::Persistent<v8::Context> m_context;
v8::Isolate* m_isolate;
v8::Platform* m_platform;
uv_loop_t* m_loop;

bool node_init() {
	int argc = 2;
	const char* argvv[] = { "node", "./index.js" };
	char** argv = uv_setup_args(argc, (char **)argvv);

	uv_loop_t* event_loop = uv_default_loop();
	if (!event_loop) {
		return 1;
	}

	v8::Platform* platform = v8::platform::CreateDefaultPlatform(4);
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();

	int exec_argc;
	const char** exec_argv;
	node::Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);

	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate* isolate = v8::Isolate::New(create_params);
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handleScope(isolate);

#ifndef _WIN32
	// node::tracing::TraceEventHelper::SetTracingController(new v8::TracingController());
#endif

	v8::Local<v8::Context> context = v8::Context::New(isolate);
	v8::Context::Scope context_scope(context);

	m_context.Reset(isolate, context);

	node::IsolateData *isolate_data = node::CreateIsolateData(isolate, event_loop);

	node::Environment* env = node::CreateEnvironment(isolate_data, context, argc, argv, exec_argc, exec_argv);
	context->GetIsolate()->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);

	{
		node::Environment::AsyncCallbackScope callback_scope(env);
		node::LoadEnvironment(env);
	}

	m_env = env;
	m_platform = platform;
	m_isolate = isolate;
	m_loop = event_loop;
	return true;
}

void node_tick() {
	v8::Isolate::Scope isolateScope(m_isolate);
	v8::SealHandleScope seal(m_isolate);
	v8::platform::PumpMessageLoop(m_platform, m_env->isolate());
	uv_run(uv_default_loop(), UV_RUN_NOWAIT);
}

void node_stop() {
	{
		v8::Isolate::Scope isolateScope(m_isolate);
		v8::SealHandleScope seal(m_isolate);
		v8::platform::PumpMessageLoop(m_platform, m_env->isolate());
		node::EmitBeforeExit(m_env);

		if (uv_loop_alive(m_env->event_loop())) {
			uv_run(m_env->event_loop(), UV_RUN_NOWAIT);
		}
	}

	node::EmitExit(m_env);
	node::RunAtExit(m_env);
	node::FreeEnvironment(m_env);

	m_isolate->Dispose();
	v8::V8::Dispose();
}
