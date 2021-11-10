#pragma once
#include <unordered_map>
#include <map>
#include "amx/amx.h"
#include "node.h"
#include "v8.h"
#include "uv.h"

namespace sampnode
{
	class event
	{
	public:
		struct EventListener_t
		{
			v8::Isolate* isolate;
			v8::UniquePersistent<v8::Context> context;
			v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> function;

			EventListener_t(
				v8::Isolate* _isolate,
				const v8::UniquePersistent<v8::Context>& _context,
				const v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>& _function
			)
			{
				isolate = _isolate;
				v8::Locker locker(isolate);
				context.Reset(_isolate, _context);
				function.Reset(_isolate, _function);
			}

			EventListener_t(
				v8::Isolate* _isolate,
				const v8::Local<v8::Context>& _context,
				const v8::Local<v8::Function>& _function
			)
			{
				isolate = _isolate;
				v8::Locker locker(isolate);
				context.Reset(_isolate, _context);
				function.Reset(_isolate, _function);
			}

			bool operator==(const EventListener_t& a) const {
				return (this->function == a.function && this->context == a.context);
			}

		};

		static void on(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void remove_listener(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void fire(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void register_event(const v8::FunctionCallbackInfo<v8::Value>& info);
		static bool register_event(const std::string& eventName, const std::string& param_types);
		static cell pawn_call_event(AMX* amx, cell* params);

		event(const std::string& eventName, const std::string& param_types);
		event();
		~event();

		void append(const v8::Local<v8::Function>& function);
		void remove(const v8::Local<v8::Function>& function);
		void remove_all();
		void call(AMX* amx, cell* params, cell* retval);
		void call_from_pawn_native(AMX* amx, cell* params, cell* retval);
		void call(v8::Local<v8::Value>* args, int argCount);

		std::string get_param_types()
		{
			return paramTypes;
		}

	private:
		std::string name;
		std::string paramTypes;
		std::vector<EventListener_t> functionList;
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> listener;

	};

	typedef std::unordered_map<std::string, sampnode::event*> eventsContainer;
	extern eventsContainer events;
}

