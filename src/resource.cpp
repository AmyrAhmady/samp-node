#include "resource.hpp"
#include "functions.hpp"
#include "callbacks.hpp"
#include "events.hpp"
#include "nodeimpl.hpp"

v8::Isolate* GetV8Isolate()
{
	return sampnode::nodeImpl.GetIsolate();
}

static v8::Platform* GetV8Platform()
{
	return sampnode::nodeImpl.GetPlatform();
}

static node::IsolateData* GetNodeIsolate()
{
	return sampnode::nodeImpl.GetNodeIsolate();
}

namespace sampnode
{

	Resource::Resource(const std::string& name, const std::string& path) : name(name), path(path), nodeEnvironment(nullptr, node::FreeEnvironment)
	{}

	Resource::Resource() : nodeEnvironment(nullptr, node::FreeEnvironment)
	{}

	Resource::~Resource()
	{
		Stop();
	}

	void Resource::Init()
	{
		std::string entryFile;
		std::vector<std::string> node_flags;
		Props_t& mainConfig = nodeImpl.GetMainConfig();

		bool useInspector;

		if (mainConfig.enable_resources && path != "no_resource")
		{
			Config configFile;
			if (!configFile.ParseFile(path + "/resource-config"))
			{
				L_ERROR << "Unable to load config file of resource " << name;
				return;
			}

			entryFile = path + "/" + configFile.get_as<std::string>("entry_file");
			node_flags = configFile.get_as<std::vector<std::string>>("node_flags");
			useInspector = mainConfig.resources.size() == 1;
		}
		else
		{
			entryFile = mainConfig.entry_file;
			node_flags = mainConfig.node_flags;
			useInspector = true;
		}

		std::vector<std::string> args;
		args.emplace_back("node");

		for (auto& flag : node_flags)
		{
			args.emplace_back(flag.c_str());
		}

		args.emplace_back(entryFile.c_str());

		for (auto& flag : args)
		{
			L_DEBUG << "node flags: " << flag;
		}

		std::vector<std::string> exec_args;

		v8::Locker locker(GetV8Isolate());
		v8::Isolate::Scope isolateScope(GetV8Isolate());
		v8::HandleScope handleScope(GetV8Isolate());

		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(GetV8Isolate());
		sampnode::functions::init(GetV8Isolate(), global);
		sampnode::callback::add_event_definitions(GetV8Isolate(), global);

		// create a global variable for resource
		v8val::add_definition("__resname", name, global);

		v8::Local<v8::Context> _context = node::NewContext(GetV8Isolate(), global);
		context.Reset(GetV8Isolate(), _context);
		v8::Context::Scope scope(_context);

		node::EnvironmentFlags::Flags flags = node::EnvironmentFlags::kOwnsProcessState;

		if (useInspector) {
			flags = static_cast<node::EnvironmentFlags::Flags>(flags | node::EnvironmentFlags::kOwnsInspector);
		}

		auto env = node::CreateEnvironment(GetNodeIsolate(), _context, args, exec_args, flags);

		node::LoadEnvironment(env, node::StartExecutionCallback{});

		nodeEnvironment.reset(env);

		return;
	}

	void Resource::Stop()
	{
		node::Stop(nodeEnvironment.get());
		node::FreeEnvironment(nodeEnvironment.get());
		context.Reset();
	}

	void Resource::RunCode(const std::string& source)
	{
		v8::Locker locker(GetV8Isolate());
		v8::Isolate::Scope isolateScope(GetV8Isolate());

		auto _context = context.Get(GetV8Isolate());

		v8::Context::Scope contextScope(_context);

		const v8::Local<v8::String>& sourceV8String = v8::String::NewFromUtf8(GetV8Isolate(), source.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
		v8::Local<v8::Script> script = v8::Script::Compile(_context, sourceV8String).ToLocalChecked();
		script->Run(_context);
	}

	v8::Local<v8::Value> Resource::AddModule(const std::string& source, const std::string& name)
	{
		v8::Isolate* isolate = GetV8Isolate();
		v8::Locker v8Locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope hs(isolate);
		v8::EscapableHandleScope handle_scope(isolate);
		v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
		v8::Context::Scope context_scope(ctx);

		auto scriptname = v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
		v8::ScriptOrigin origin(scriptname, v8::Integer::New(isolate, -1), v8::Integer::New(isolate, 0));
		auto sourceCode = v8::String::NewFromUtf8(isolate, source.c_str());

		v8::TryCatch try_catch(isolate);

		auto script = v8::Script::Compile(ctx, sourceCode.ToLocalChecked(), &origin);

		if (script.IsEmpty())
		{
			isolate->CancelTerminateExecution();
			v8::String::Utf8Value exception(isolate, try_catch.Exception());
			const char* exception_string = *exception;
			v8::Local<v8::Message> message = try_catch.Message();

			if (message.IsEmpty())
			{
				L_ERROR << exception_string;
			}
			else
			{
				v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
				const char* filename_string = *filename;
				int linenum = message->GetLineNumber(ctx).ToChecked();

				L_ERROR << filename_string << ":" << linenum << ": " << exception_string;
				v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(ctx).ToLocalChecked());
				const char* sourceline_string = *sourceline;
				L_INFO << sourceline_string;
			}
		}
		else
		{
			try_catch.Reset();
			v8::Local<v8::Value> result = script.ToLocalChecked()->Run(ctx).ToLocalChecked();
			if (try_catch.HasCaught())
			{
				isolate->CancelTerminateExecution();
				v8::String::Utf8Value exception(isolate, try_catch.Exception());
				const char* exception_string = *exception;
				v8::Local<v8::Message> message = try_catch.Message();

				if (message.IsEmpty())
				{
					L_ERROR << exception_string;
				}
				else
				{
					v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
					const char* filename_string = *filename;
					int linenum = message->GetLineNumber(ctx).ToChecked();

					L_ERROR << filename_string << ":" << linenum << ": " << exception_string;
					v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(ctx).ToLocalChecked());
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

	void v8val::add_definition(const std::string& name, const std::string& value, v8::Local<v8::ObjectTemplate>& global)
	{
		v8::Local<v8::Value> test = v8::String::NewFromUtf8(GetV8Isolate(), value.c_str(), v8::NewStringType::kNormal, static_cast<int>(value.length())).ToLocalChecked();
		global->Set(v8::String::NewFromUtf8(GetV8Isolate(), name.c_str(), v8::NewStringType::kNormal).ToLocalChecked(), test, v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
	}
}

