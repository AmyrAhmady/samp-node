#define NODE_WANT_INTERNALS 1
#define USING_UV_SHARED 1

#include "node.h"
#include "env.h"
#include "env-inl.h"
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "libplatform/libplatform.h"

#include "impl.hpp"

v8::UniquePersistent<v8::Context> m_context;
node::Environment* m_nodeEnvironment;

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

static void LMAO(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool first = true;

	for (int i = 0; i < args.Length(); i++)
	{
		v8::Locker locker(args.GetIsolate());
		v8::Isolate::Scope isolateScope(args.GetIsolate());
		v8::HandleScope handle_scope(args.GetIsolate());

		if (first)
		{
			first = false;
		}
		else
		{
			printf(" ");
		}

		v8::String::Utf8Value str(GetV8Isolate(), args[i]);
		printf("%s", *str);
	}
}

bool node_init()
{
	const char* argvv[] = { "node", "./index.js" };
	int argc = 2;
	char** argv = uv_setup_args(2, (char **)argvv);
	int exec_argc;
	const char** exec_argv;
	g_v8.Initialize(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);

	v8::Locker locker(GetV8Isolate());
	v8::Isolate::Scope isolateScope(GetV8Isolate());
	v8::HandleScope handleScope(GetV8Isolate());

	v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(GetV8Isolate());

	global->Set(v8::String::NewFromUtf8(GetV8Isolate(), "print", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(GetV8Isolate(), LMAO));


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