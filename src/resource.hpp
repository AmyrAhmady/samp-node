#pragma once
#define NODE_WANT_INTERNALS 1
#define USING_UV_SHARED 1

#include "node.h"
#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "libplatform/libplatform.h"
#include "utils.hpp"

namespace sampnode
{
	class Resource
	{
	public:
		Resource(const std::string& name, const std::string& path);
		Resource();
		~Resource();

		void Init();
		void Stop();

		void RunCode(const std::string& source);
		v8::Local<v8::Value> AddModule(const std::string& source, const std::string& name);

		inline v8::UniquePersistent<v8::Context>& GetContext()
		{
			return context;
		}

		node::Environment* GetEnv()
		{
			return nodeEnvironment;
		}

	private:
		v8::UniquePersistent<v8::Context> context;
		node::Environment* nodeEnvironment;
		std::string path;
		std::string name;
	};

	namespace v8val
	{
		inline std::string to_string(const v8::Local<v8::Value>& val) { return utils::js_to_string(val); }
		inline const char* to_cstring(const v8::Local<v8::Value>& val) { return utils::js_to_cstr(val); }
		void add_definition(const std::string& name, const std::string& value, v8::Local<v8::ObjectTemplate>& global);
	}
}