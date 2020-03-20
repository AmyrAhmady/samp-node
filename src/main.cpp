#include <iostream>
#include <fstream>
#include <algorithm>
#include "callbacks.hpp"
#include "events.hpp"
#include "amxhandler.hpp"
#include "node.hpp"
#include <sampgdk.h>
#include "common.hpp"

logprintf_t logprintf;

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
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// this piece of code is taken from maddinat0r's samp-discord-connector
	// https://github.com/maddinat0r/samp-discord-connector/blob/master/src/SampConfigReader.cpp
	std::string node_flags;
	std::vector<std::string> file_content;
	std::ifstream config_file("server.cfg");
	while (config_file.good())
	{
		std::string line_buffer;
		std::getline(config_file, line_buffer);

		size_t cr_pos = line_buffer.find_first_of("\r\n");
		if (cr_pos != std::string::npos)
			line_buffer.erase(cr_pos);

		file_content.push_back(std::move(line_buffer));
	}

	std::string varname = "node_flags ";
	for (auto& i : file_content)
	{
		if (i.find(varname) == 0)
		{
			node_flags = i.substr(varname.length());
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////

	logprintf = (logprintf_t)(ppData[PLUGIN_DATA_LOGPRINTF]);
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	sampgdk::Load(ppData);
	sampnode::callback::init();
	sampnode::node_init(node_flags);
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