#include <utility>
#include <string>
#include "v8.h"
#include "common.hpp"
#include "functions.hpp"
#include "events.hpp"
#include "natives.hpp"
#include "callbacks.hpp"


static std::pair<std::string, v8::FunctionCallback> sampnodeSpecificFunctions[] =
{
	{ "on", sampnode::event::on },
	{ "addEventListener", sampnode::event::on },
	{ "addListener", sampnode::event::on },
	{ "removeListener", sampnode::event::remove_listener },
	{ "removeEventListener", sampnode::event::remove_listener },
	{ "fire", sampnode::event::fire },
	{ "registerEvent", sampnode::event::register_event },
	{ "callNative", sampnode::native::call },
	{ "callNativeFloat", sampnode::native::call_float },
	{ "callPublic", sampnode::callback::call },
	{ "callPublicFloat", sampnode::callback::call_float },
	{ "logprint", sampnode::functions::logprint }
};

namespace sampnode
{
	void functions::init(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate>& global)
	{
		v8::Local<v8::ObjectTemplate> sampObject = v8::ObjectTemplate::New(isolate);
		for (auto& routine : sampnodeSpecificFunctions)
		{
			sampObject->Set(v8::String::NewFromUtf8(isolate, routine.first.c_str(), v8::NewStringType::kNormal).ToLocalChecked(),
				v8::FunctionTemplate::New(isolate, routine.second));
		}

		global->Set(v8::String::NewFromUtf8(isolate, "samp", v8::NewStringType::kNormal).ToLocalChecked(), sampObject);
	}

	void functions::logprint(const v8::FunctionCallbackInfo<v8::Value>& info)
	{
		if (info.Length() > 0)
		{
			v8::HandleScope scope(info.GetIsolate());
			if (!info[0]->IsString())
				return;
			v8::String::Utf8Value _str(info.GetIsolate(), info[0]);
			logprintf(*_str);
		}
	}
}