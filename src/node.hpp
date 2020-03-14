#pragma once
#include "utils.hpp"

namespace sampnode
{
	enum class ExceptionType {
		REGULAR_ERROR,
		TYPE_ERROR,
		SYNTAX_ERROR,
		RANGE_ERROR,
		REFERENCE_ERROR
	};

	bool node_init(const std::string& node_flags);
	void node_tick();
	void node_stop();
	v8::Local<v8::Value> node_execute_code(const std::string& source, const std::string& name);
	void node_throw_exception(const std::string& text);
	namespace v8val
	{
		inline std::string to_string(const v8::Local<v8::Value>& val) { return utils::js_to_string(val); }
		inline const char* to_cstring(const v8::Local<v8::Value>& val) { return utils::js_to_cstr(val); }
		void add_definition(const std::string& name, const std::string& value, v8::Local<v8::ObjectTemplate>& global);
	}

	extern v8::UniquePersistent<v8::Context> m_context;
}