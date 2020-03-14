#pragma once
#include <vector>
#include <string>
#include <sstream>
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

	inline std::vector<std::string> split(const std::string& s, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}

}
