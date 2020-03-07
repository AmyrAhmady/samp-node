#define NODE_WANT_INTERNALS 1
#define USING_UV_SHARED 1

#include "node.h"
#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "libplatform/libplatform.h"
#include "node.hpp"
#include "functions.hpp"
#include "callbacks.hpp"
#include "events.hpp"
#include "v8impl.hpp"
#include "logger.hpp"

static V8ScriptGlobals g_v8;

v8::Isolate* GetV8Isolate()
{
	return g_v8.GetIsolate();
}

static v8::Platform* GetV8Platform()
{
	return g_v8.GetPlatform();
}

static node::IsolateData* GetNodeIsolate()
{
	return g_v8.GetNodeIsolate();
}

namespace sampnode
{
	v8::UniquePersistent<v8::Context> m_context; 
	node::Environment* m_nodeEnvironment;
	bool node_init()
	{
		const char* argvv[] = { "node", "./index.js" };
		int argc = 2;
		char** argv = uv_setup_args(2, (char**)argvv);
		int exec_argc;
		const char** exec_argv;
		g_v8.Initialize(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);

		v8::Locker locker(GetV8Isolate());
		v8::Isolate::Scope isolateScope(GetV8Isolate());
		v8::HandleScope handleScope(GetV8Isolate());

		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(GetV8Isolate());
		sampnode::functions::init(GetV8Isolate(), global);
		sampnode::callback::add_event_definitions(GetV8Isolate(), global);

		v8::Local<v8::Context> context = v8::Context::New(GetV8Isolate(), nullptr, global);
		m_context.Reset(GetV8Isolate(), context);
		v8::Context::Scope scope(context);

		auto env = node::CreateEnvironment(GetNodeIsolate(), context, 2, argv, exec_argc, exec_argv);
		node::LoadEnvironment(env);
		m_nodeEnvironment = env;

		return true;
	}

	void node_tick()
	{
		v8::platform::PumpMessageLoop(GetV8Platform(), GetV8Isolate());
	}

	void node_stop()
	{
		node::FreeEnvironment(m_nodeEnvironment);
		m_context.Reset();
	}

	v8::Local<v8::Value> node_execute_code(const std::string& source, const std::string& name)
	{
		v8::Isolate* isolate = GetV8Isolate();
		v8::Locker v8Locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope hs(isolate);
		v8::EscapableHandleScope handle_scope(isolate);
		v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, m_context);
		v8::Context::Scope context_scope(ctx);

		auto scriptname = v8::String::NewFromUtf8(isolate, name.c_str());
		v8::ScriptOrigin origin(scriptname, v8::Integer::New(isolate, -1));
		auto sourceCode = v8::String::NewFromUtf8(isolate, source.c_str());

		v8::TryCatch try_catch(isolate);

		auto script = v8::Script::Compile(sourceCode, &origin);

		if (script.IsEmpty()) {
			isolate->CancelTerminateExecution();
			v8::String::Utf8Value exception(try_catch.Exception());
			const char* exception_string = *exception;
			v8::Local<v8::Message> message = try_catch.Message();

			if (message.IsEmpty()) {
				L_ERROR << exception_string;
			}
			else {
				v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
				const char* filename_string = *filename;
				int linenum = message->GetLineNumber();

				L_ERROR << filename_string << ":" << linenum << ": " << exception_string;
				v8::String::Utf8Value sourceline(message->GetSourceLine());
				const char* sourceline_string = *sourceline;
				L_INFO << sourceline_string;
			}
		}
		else {
			try_catch.Reset();
			v8::Local<v8::Value> result = script->Run();
			if (try_catch.HasCaught()) {
				isolate->CancelTerminateExecution();
				v8::String::Utf8Value exception(try_catch.Exception());
				const char* exception_string = *exception;
				v8::Local<v8::Message> message = try_catch.Message();

				if (message.IsEmpty()) {
					L_ERROR << exception_string;
				}
				else {
					v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
					const char* filename_string = *filename;
					int linenum = message->GetLineNumber();

					L_ERROR << filename_string << ":" << linenum << ": " << exception_string;
					v8::String::Utf8Value sourceline(message->GetSourceLine());
					const char* sourceline_string = *sourceline;
					L_INFO << sourceline_string;
				}
				v8::Local<v8::Value> ret;
				return ret;
			}

			return handle_scope.Escape(result);
		}

		return v8::Local<v8::Value>();
	}

	void node_throw_exception(const std::string& text, ExceptionType type)
	{
		v8::Isolate* isolate = GetV8Isolate();
		if (type == ExceptionType::REGULAR_ERROR)
		{
			isolate->ThrowException(
				v8::Exception::Error(
					v8::String::NewFromUtf8(isolate, text.c_str())
				)
			);
		}
		else if (type == ExceptionType::REFERENCE_ERROR)
		{
			isolate->ThrowException(
				v8::Exception::ReferenceError(
					v8::String::NewFromUtf8(isolate, text.c_str())
				)
			);
		}
		else if (type == ExceptionType::RANGE_ERROR)
		{
			isolate->ThrowException(
				v8::Exception::RangeError(
					v8::String::NewFromUtf8(isolate, text.c_str())
				)
			);
		}
		else if (type == ExceptionType::TYPE_ERROR)
		{
			isolate->ThrowException(
				v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, text.c_str())
				)
			);
		}
		else if (type == ExceptionType::SYNTAX_ERROR)
		{
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, text.c_str())
				)
			);
		}

	}

	void v8val::add_definition(const std::string& name, const std::string& value, v8::Local<v8::ObjectTemplate>& global)
	{
		v8::Local<v8::Value> test = v8::String::NewFromUtf8(GetV8Isolate(), value.c_str(), v8::NewStringType::kNormal, static_cast<int>(value.length())).ToLocalChecked();
		global->Set(v8::String::NewFromUtf8(GetV8Isolate(), name.c_str(), v8::NewStringType::kNormal).ToLocalChecked(), test, v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
	}
}

