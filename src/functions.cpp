#include <utility>
#include <string>
#include "v8.h"
#include "functions.hpp"
#include "events.hpp"


static std::pair<std::string, v8::FunctionCallback> sampnodeSpecificFunctions[] =
{
	{ "on", sampnode::event::on },
	{ "addEventListener", sampnode::event::on },
	{ "addListener", sampnode::event::on },
	{ "removeListener", sampnode::event::remove_listener },
	{ "removeEventListener", sampnode::event::remove_listener },
	{ "registerEvent", sampnode::event::register_event	},
};

namespace sampnode
{
	void functions::init(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate>& global)
	{
		for (auto& routine : sampnodeSpecificFunctions)
		{
			global->Set(v8::String::NewFromUtf8(isolate, routine.first.c_str(), v8::NewStringType::kNormal).ToLocalChecked(),
				v8::FunctionTemplate::New(isolate, routine.second));
		}
	}
}