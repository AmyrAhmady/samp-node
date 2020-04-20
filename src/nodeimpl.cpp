#include <cstring>
#include <unordered_map>
#include "config.hpp"
#include "resource.hpp"
#include "nodeimpl.hpp"

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

namespace sampnode
{
	NodeImpl nodeImpl;
	std::unordered_map<node::Environment*, std::shared_ptr<Resource>> NodeImpl::resourcesPool;

	void NodeImpl::LoadAllResources(const std::vector<std::string>& resources, bool enable_resources)
	{
		if (enable_resources)
		{
			for (auto& resource : resources)
			{
				nodeImpl.LoadResource(resource);
			}
		}
		else
		{
			nodeImpl.LoadResource("");
		}
	}

	NodeImpl::NodeImpl()
	{}

	NodeImpl::~NodeImpl()
	{}

	void NodeImpl::Initialize(const Props_t& config)
	{
		mainConfig = config;
		auto platform = node::InitializeV8Platform(4);
		v8Platform = std::unique_ptr<v8::Platform>(platform);

		const char* flags = "--expose_gc";
		v8::V8::SetFlagsFromString(flags, strlen(flags));

		v8::V8::Initialize();

		arrayBufferAllocator = std::make_unique<ArrayBufferAllocator>();

		v8::Isolate::CreateParams params;
		params.array_buffer_allocator = arrayBufferAllocator.get();

		v8Isolate = v8::Isolate::New(params);
		v8Isolate->SetFatalErrorHandler([](const char* location, const char* message)
			{
				exit(0);
			});

		v8Isolate->SetCaptureStackTraceForUncaughtExceptions(true);

		//v8::Locker locker(m_isolate);
		v8::Isolate::Scope isolateScope(v8Isolate);
		v8::HandleScope handle_scope(v8Isolate);

		int eac;
		const char** eav;

		std::vector<const char*> args{
			"",
			"--expose-internals"
		};

		int argc = args.size();

		node::Init(&argc, args.data(), &eac, &eav);
		nodeLoop = std::make_unique<UvLoop>("mainNode");
		nodeData = node::CreateIsolateData(v8Isolate, nodeLoop->GetLoop(), platform, (node::ArrayBufferAllocator*)arrayBufferAllocator.get());
	}

	bool NodeImpl::LoadResource(const std::string& name)
	{
		if (mainConfig.enable_resources)
		{
			std::shared_ptr<Resource> resource = std::make_shared<Resource>(name, "./" + mainConfig.resources_path + "/" + name);
			resource->Init();
			resourcesPool.insert({ resource->GetEnv(), resource });
			resourceNamesPool.insert({ name, resource->GetEnv() });
			return true;
		}
		else
		{
			std::shared_ptr<Resource> resource = std::make_shared<Resource>("main", "no_resource");
			resource->Init();
			resourcesPool.insert({ resource->GetEnv(), resource });
			resourceNamesPool.insert({ name, resource->GetEnv() });
			return true;
		}
	}

	bool NodeImpl::UnloadResource(const std::string& name)
	{
		node::Environment* nodeEnv = resourceNamesPool[name];
		resourcesPool.erase(nodeEnv);
		resourceNamesPool.erase(name);
		return true;
	}

	bool NodeImpl::ReloadResource(const std::string& name)
	{
		return true;
	}
}