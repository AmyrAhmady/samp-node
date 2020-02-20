
// uncomment for testing it as an exe file (change build configs too)
//#define TEST


#include <amx/amx.h>
#include <plugincommon.h>
#include "node_api.h"

#include "common.hpp"

bool node_init();
void node_tick();
void node_stop();

logprintf_t logprintf;

#include <iostream>

napi_value double_number(napi_env env, napi_callback_info info) {
	size_t argc = 1;
	int number = 0;
	napi_value argv[1];

	TRY(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
	TRY(napi_get_value_int32(env, argv[0], &number));

	napi_value myNumber;
	number = number * 2;

	TRY(napi_create_int32(env, number, &myNumber));
	return myNumber;
}

napi_value get_position(napi_env env, napi_callback_info info) {

	std::pair<double, double> pos = { 15.3245, 242.58275 };

	napi_value result;
	TRY(napi_create_object(env, &result));

	REGISTER_MEMBER(double, env, result, "x", pos.first);
	REGISTER_MEMBER(double, env, result, "y", pos.second);

	return result;
}

napi_value samp_node_init(napi_env env, napi_value exports) {
	logprintf("FUUUUUUUUUUCK");
	REGISTER_METHOD(env, exports, "doubleNumber", double_number);
	REGISTER_METHOD(env, exports, "get_Position", get_position);
	return exports;
}

// dynmaic lib/testing execuatble switch
#if defined(TEST)

int main(int argc, char* argv[]) {
	node_init();

	while (true) {
		node_tick();
		_sleep(10);
	}

	node_stop();
	return 0;
}

#else

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerConnect(int playerid)
{
	logprintf("[SAMP-NODE] a new player %d has connected\n", playerid);
	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL ProcessTick()
{
	logprintf("SAMP-NODE: Process Tick???");
	node_tick();
	return true;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	logprintf("SAMP-NODE: Supports???");
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	node_init();
	logprintf("SAMP-NODE: Load???");
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	logprintf("SAMP-NODE: AmxLoad???");
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL Unload()
{
	node_stop();
	logprintf("SAMP-NODE: Unload???");
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	logprintf("SAMP-NODE: AmxUnload???");
	return 1;
}

#endif

enum { NM_F_BUILTIN = 1 << 0 };
NAPI_MODULE_X(samp, samp_node_init, NULL, NM_F_BUILTIN)