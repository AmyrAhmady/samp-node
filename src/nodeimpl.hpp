#pragma once
#include <unordered_map>
#include "node.h"
#include "node_api.h"
#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "libplatform/libplatform.h"
#include "config.hpp"
#include "resource.hpp"
#include "uvloop.hpp"

namespace sampnode
{
	class NodeImpl
	{
	public:
		static std::unordered_map<node::Environment*, std::shared_ptr<Resource>> resourcesPool;

		static void LoadAllResources(const std::vector<std::string>& resources, bool enable_resources = true);

		NodeImpl();
		~NodeImpl();

		void Initialize(const Props_t& config);
		bool LoadResource(const std::string& name);
		bool UnloadResource(const std::string& name);
		bool ReloadResource(const std::string& name);

		inline v8::Platform* GetPlatform()
		{
			return v8Platform.get();
		}

		inline v8::Isolate* GetIsolate()
		{
			return v8Isolate;
		}

		inline node::IsolateData* GetNodeIsolate()
		{
			return nodeData;
		}

		inline UvLoop* GetUVLoop()
		{
			return nodeLoop.get();
		}

		inline Props_t& GetMainConfig()
		{
			return mainConfig;
		}

		inline void Tick()
		{
			uv_run(nodeLoop->GetLoop(), UV_RUN_NOWAIT);
		}

		inline void Stop()
		{
			for (auto& resource : resourceNamesPool)
			{
				UnloadResource(resource.first);
			}
		}

	private:
		v8::Isolate* v8Isolate;
		node::IsolateData* nodeData;
		std::unique_ptr<v8::Platform> v8Platform;
		std::unique_ptr<v8::ArrayBuffer::Allocator> arrayBufferAllocator;
		std::unique_ptr<UvLoop> nodeLoop;
		Props_t mainConfig;
		std::unordered_map<std::string, node::Environment*> resourceNamesPool;
	};

	extern NodeImpl nodeImpl;
}