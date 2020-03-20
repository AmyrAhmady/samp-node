#pragma once

namespace sampnode
{
	namespace functions
	{
		void init(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate>& global);
		void logprint(const v8::FunctionCallbackInfo<v8::Value>& info);
	}
}