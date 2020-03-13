#pragma once
#include "v8.h"
#include "amx/amx.h"

namespace sampnode
{
	class callback
	{
	public:

		struct data {
			std::string name;
			std::string param_types;
			std::string alias;
		};

		static void init();
		static void add_event_definitions(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate>& global);
		static void call(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void call_float(const v8::FunctionCallbackInfo<v8::Value>& info);
	};

	extern bool js_calling_public;
}