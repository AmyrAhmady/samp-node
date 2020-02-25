#include <iostream>
#include <amx/amx.h>
#include <plugincommon.h>
#include "node_api.h"

#include "common.hpp"

bool node_init();
void node_tick();
void node_stop();

napi_value samp_node_init(napi_env env, napi_value exports) 
{
	return exports;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerConnect(int playerid)
{
	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL ProcessTick()
{
	node_tick();
	return true;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	node_init();
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL Unload()
{
	node_stop();
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	return 1;
}

#endif

enum { NM_F_BUILTIN = 1 << 0 };
NAPI_MODULE_X(samp, samp_node_init, NULL, NM_F_BUILTIN)