
#include "node.h"
#include "node_api.h"
#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "libplatform/libplatform.h"
#include "impl.hpp"

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

// Thanks to Hual for mentioning this, SetThreadDescription is a Windows10 only, so.. lets forget about this.

/*
#ifndef _WIN32
void SetThreadName(int dwThreadID, const char* threadName)
{
	std::string shortenedName = std::string(threadName).substr(0, 15);
	pthread_setname_np(pthread_self(), shortenedName.c_str());
}
#else
void SetThreadName(int dwThreadID, const char* threadName)
{
	auto SetThreadDescription = (HRESULT(WINAPI*)(HANDLE, PCWSTR))GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetThreadDescription");

	if (SetThreadDescription)
	{
		HANDLE hThread = (dwThreadID < 0) ? GetCurrentThread() : OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, dwThreadID);

		if (hThread != NULL)
		{
			const WCHAR *pwcsName;
			int nChars = MultiByteToWideChar(CP_ACP, 0, threadName, -1, NULL, 0);
			pwcsName = new WCHAR[nChars];
			MultiByteToWideChar(CP_ACP, 0, threadName, -1, (LPWSTR)pwcsName, nChars);
			SetThreadDescription(hThread, pwcsName);
			delete[] pwcsName;

			if (dwThreadID >= 0)
			{
				CloseHandle(hThread);
			}
		}
	}
}
#endif
*/

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

	//v8::V8::InitializeICUDefaultLocation(v8::ToNarrow(v8::MakeRelativeCitPath(L"dummy")).c_str());

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
	v8::Locker locker(m_isolate);
	v8::Isolate::Scope isolateScope(m_isolate);
	v8::HandleScope handle_scope(m_isolate);

	node::Init(argc, argv, exec_argc, exec_argv);

	UvLoopHolder* _loop = new UvLoopHolder("dope");

	m_nodeData = node::CreateIsolateData(m_isolate, _loop->GetLoop());

}

V8ScriptGlobals::~V8ScriptGlobals()
{

}

UvLoopHolder::UvLoopHolder(const std::string& loopTag)
	: m_shouldExit(false), m_loopTag(loopTag)
{

	uv_loop_init(&m_loop);

	m_loop.data = this;

	m_thread = std::thread([=]()
	{
		while (!m_shouldExit)
		{
			uv_run(&m_loop, UV_RUN_DEFAULT);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		uv_loop_close(&m_loop);
	});
}

UvLoopHolder::~UvLoopHolder()
{
	m_shouldExit = true;

	uv_stop(&m_loop);

	uv_async_t async;

	uv_async_init(&m_loop, &async, [](uv_async_t*)
	{

	});

	uv_async_send(&async);

	if (m_thread.joinable())
	{
		m_thread.join();
	}

	uv_close(reinterpret_cast<uv_handle_t*>(&async), nullptr);
}

void UvLoopHolder::AssertThread()
{
#if _DEBUG
	assert(std::this_thread::get_id() == m_thread.get_id());
#endif
}
