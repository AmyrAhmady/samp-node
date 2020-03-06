#pragma once
#include "v8.h"

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
	};
}