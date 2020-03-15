#pragma once
#include <thread>

class UvLoopHolder
{
private:
	uv_loop_t m_loop;
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

class V8ScriptGlobals
{
private:
	v8::Isolate* m_isolate;
	node::IsolateData* m_nodeData;
	std::unique_ptr<v8::Platform> m_platform;
	std::unique_ptr<v8::ArrayBuffer::Allocator> m_arrayBufferAllocator;
	std::unique_ptr<UvLoopHolder> m_loop;

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

	inline UvLoopHolder* GetUVLoop()
	{
		return m_loop.get();
	}
};
