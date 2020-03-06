#include <vector>
#include <string>
#include "log.hpp"
#include "events.hpp"
#include "callbacks.hpp"

namespace sampnode
{
	static const std::vector<std::pair<std::string, std::string>> sampCallbacks =
	{
		{ "OnGameModeInit", "" },
		{ "OnGameModeExit", "" },
		{ "OnFilterScriptInit", "" },
		{ "OnFilterScriptExit", "" },
		{ "OnPlayerConnect", "i" },
		{ "OnPlayerDisconnect", "ii" },
		{ "OnPlayerSpawn", "i" },
		{ "OnPlayerDeath", "iii" },
		{ "OnVehicleSpawn", "i" },
		{ "OnVehicleDeath", "ii" },
		{ "OnPlayerText", "is" },
		{ "OnPlayerCommandText", "is" },
		{ "OnPlayerRequestClass", "ii" },
		{ "OnPlayerEnterVehicle", "iii" },
		{ "OnPlayerExitVehicle", "ii" },
		{ "OnPlayerStateChange", "iii" },
		{ "OnPlayerEnterCheckpoint", "i" },
		{ "OnPlayerLeaveCheckpoint", "i" },
		{ "OnPlayerEnterRaceCheckpoint", "i" },
		{ "OnPlayerLeaveRaceCheckpoint", "i" },
		{ "OnRconCommand", "s" },
		{ "OnPlayerRequestSpawn", "i" },
		{ "OnObjectMoved", "i" },
		{ "OnPlayerObjectMoved", "ii" },
		{ "OnPlayerPickUpPickup", "ii" },
		{ "OnVehicleMod", "iii" },
		{ "OnEnterExitModShop", "iii" },
		{ "OnVehiclePaintjob", "iii" },
		{ "OnVehicleRespray", "iiii" },
		{ "OnVehicleDamageStatusUpdate", "ii" },
		{ "OnUnoccupiedVehicleUpdate", "iiiffffff" },
		{ "OnPlayerSelectedMenuRow", "ii" },
		{ "OnPlayerExitedMenu", "i" },
		{ "OnPlayerInteriorChange", "iii" },
		{ "OnPlayerKeyStateChange", "iii" },
		{ "OnRconLoginAttempt", "ssi" },
		{ "OnPlayerUpdate", "i" },
		{ "OnPlayerStreamIn", "ii" },
		{ "OnPlayerStreamOut", "ii" },
		{ "OnVehicleStreamIn", "ii" },
		{ "OnVehicleStreamOut", "ii" },
		{ "OnActorStreamIn", "ii" },
		{ "OnActorStreamOut", "ii" },
		{ "OnDialogResponse", "iiiis" },
		{ "OnPlayerTakeDamage", "iifii" },
		{ "OnPlayerGiveDamage", "iifii" },
		{ "OnPlayerGiveDamageActor", "iifii" },
		{ "OnPlayerClickMap", "ifff" },
		{ "OnPlayerClickTextDraw", "ii" },
		{ "OnPlayerClickPlayerTextDraw", "ii" },
		{ "OnIncomingConnection", "isi" },
		{ "OnTrailerUpdate", "ii" },
		{ "OnVehicleSirenStateChange", "iii" },
		{ "OnPlayerFinishedDownloading", "ii" },
		{ "OnPlayerRequestDownload", "iii" },
		{ "OnPlayerClickPlayer", "iii" },
		{ "OnPlayerEditObject", "iiiiffffff" },
		{ "OnPlayerEditAttachedObject", "iiiiifffffffff" },
		{ "OnPlayerSelectObject", "iiiifff" },
		{ "OnPlayerWeaponShot", "iiiifff" }
	};

	void callback::init()
	{
		for (auto& callback : sampCallbacks)
		{
			event::register_event(callback.first, callback.second);
			L_DEBUG << "registered samp callback '" << callback.first << "' in event pool";
		}
	}
}