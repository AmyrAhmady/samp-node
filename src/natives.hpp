#pragma once

#include <unordered_map>
#include <string>
#include "amx/amx.h"

namespace sampnode
{
	namespace native
	{
		void call_pawn_native(const v8::FunctionCallbackInfo<v8::Value>& args);
		AMX_NATIVE get_address(const std::string& name);
	}
}