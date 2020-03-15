#include <cstring>
#include "node.h"
#include "node_api.h"
#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "libplatform/libplatform.h"
#include "v8impl.hpp"

// Utils
class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:
	virtual void* Allocate(size_t length) override
	{
		void* data = AllocateUninitialized(length);
		return data == NULL ? data : memset(data, 0, length);
	}
	virtual void* AllocateUninitialized(size_t length) override { return malloc(length); }
	virtual void Free(void* data, size_t) override { free(data); }
};

V8ScriptGlobals::V8ScriptGlobals()
{
}

void V8ScriptGlobals::Initialize(int* argc,
	const char** argv,
	int* exec_argc,
	const char*** exec_argv)
{

	auto platform = node::InitializeV8Platform(4);
	m_platform = std::unique_ptr<v8::Platform>(platform);

	const char* flags = "--expose_gc";
	v8::V8::SetFlagsFromString(flags, strlen(flags));

	v8::V8::Initialize();

	m_arrayBufferAllocator = std::make_unique<ArrayBufferAllocator>();

	v8::Isolate::CreateParams params;
	params.array_buffer_allocator = m_arrayBufferAllocator.get();

	m_isolate = v8::Isolate::New(params);
	m_isolate->SetFatalErrorHandler([](const char* location, const char* message)
		{
			exit(0);
		});

	m_isolate->SetCaptureStackTraceForUncaughtExceptions(true);

	// initialize Node.js
	//v8::Locker locker(m_isolate);
	v8::Isolate::Scope isolateScope(m_isolate);
	v8::HandleScope handle_scope(m_isolate);

	int eac;
	const char** eav;

	/*std::vector<const char*> args{
		"",
		"--expose-internals",
	};

	for (int i = 0; i < g_argc; i++)
	{
		if (g_argv[i] && g_argv[i][0] == '-')
		{
			args.push_back(g_argv[i]);
		}
	}*/

	//int argc2 = args.size();

	node::Init(argc, argv, &eac, &eav);

	m_loop = std::make_unique<UvLoopHolder>("dope");

	m_nodeData = node::CreateIsolateData(m_isolate, m_loop->GetLoop(), platform, (node::ArrayBufferAllocator*)m_arrayBufferAllocator.get());

}

V8ScriptGlobals::~V8ScriptGlobals()
{

}

UvLoopHolder::UvLoopHolder(const std::string& loopTag)
	: m_loopTag(loopTag)
{

	uv_loop_init(&m_loop);

	m_loop.data = this;

	uv_run(&m_loop, UV_RUN_NOWAIT);
}

UvLoopHolder::~UvLoopHolder()
{
	uv_stop(&m_loop);

	uv_async_t async;

	uv_async_init(&m_loop, &async, [](uv_async_t*)
		{

		});

	uv_async_send(&async);

	uv_close(reinterpret_cast<uv_handle_t*>(&async), nullptr);
}

void UvLoopHolder::AssertThread()
{
#if _DEBUG
	assert(std::this_thread::get_id() == m_thread.get_id());
#endif
}
