#include <iostream>
#include "callbacks.hpp"
#include "events.hpp"
#include "amxhandler.hpp"
#include "node.hpp"
#include <sampgdk.h>
#include "common.hpp"

PLUGIN_EXPORT bool PLUGIN_CALL OnPublicCall(AMX* amx, const char* name, cell* params, cell* retval)
{
	if (sampnode::js_calling_public)
		return true;
	auto iter = sampnode::events.find(name);
	if (iter != sampnode::events.end())
		iter->second->call(amx, params, retval);
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	sampgdk::ProcessTick();
	sampnode::node_tick();
	return;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	sampgdk::Load(ppData);
	sampnode::callback::init();
	sampnode::node_init();
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	sampnode::amx::load(amx);
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	sampgdk::Unload();
	sampnode::node_stop();
	return;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	sampnode::amx::unload(amx);
	return 1;
}