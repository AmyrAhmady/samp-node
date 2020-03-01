#pragma once
#include <vector>
#include <utility>
#include "v8.h"

namespace utils {

	inline const char* check_string_conversion(const v8::String::Utf8Value& value)
	{
		return *value ? *value : "<string conversion failed>";
	}

	inline const char* js_to_cstr(const v8::Local<v8::Value>& val)
	{
		const v8::String::Utf8Value jsString(val);
		const char* str = check_string_conversion(jsString);
		return str;
	}

	inline std::string js_to_string(const v8::Local<v8::Value>& val)
	{
		const char* str;
		const v8::String::Utf8Value jsString(val);
		str = check_string_conversion(jsString);
		return std::string(str);
	}
}
