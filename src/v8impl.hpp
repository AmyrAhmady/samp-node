#pragma once
#include <thread>

class V8ScriptGlobals
{
private:
	v8::Isolate* m_isolate;

	node::IsolateData* m_nodeData;

	std::vector<char> m_nativesBlob;

	std::vector<char> m_snapshotBlob;

	std::unique_ptr<v8::Platform> m_platform;

	std::unique_ptr<v8::ArrayBuffer::Allocator> m_arrayBufferAllocator;

public:
	V8ScriptGlobals();

	~V8ScriptGlobals();

	void Initialize(int* argc,
		const char** argv,
		int* exec_argc,
		const char*** exec_argv);

	inline v8::Platform* GetPlatform()
	{
		return m_platform.get();
	}

	inline v8::Isolate* GetIsolate()
	{
		return m_isolate;
	}

	inline node::IsolateData* GetNodeIsolate()
	{
		return m_nodeData;
	}
};

class UvLoopHolder
{
private:
	uv_loop_t m_loop;

	std::thread m_thread;

	bool m_shouldExit;

	std::string m_loopTag;

public:
	UvLoopHolder(const std::string& loopTag);

	virtual ~UvLoopHolder();

	void AssertThread();

	inline uv_loop_t* GetLoop()
	{
		return &m_loop;
	}

	inline const std::string& GetLoopTag() const
	{
		return m_loopTag;
	}
};
