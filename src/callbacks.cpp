#include <vector>
#include <string>
#include "v8.h"
#include "logger.hpp"
#include "node.hpp"
#include "events.hpp"
#include "callbacks.hpp"

namespace sampnode
{
	static const std::vector<callback::data> sampCallbacks =
	{
		{ "OnGameModeInit", "", "EVENT_GAME_MODE_INIT" },
		{ "OnGameModeExit", "", "EVENT_GAME_MODE_EXIT" },
		{ "OnFilterScriptInit", "", "EVENT_FILTERSCRIPT_INIT" },
		{ "OnFilterScriptExit", "", "EVENT_FILTERSCRIPT_EXIT" },
		{ "OnPlayerConnect", "i", "EVENT_PLAYER_CONNECT" },
		{ "OnPlayerDisconnect", "ii", "EVENT_PLAYER_DISCONNECT" },
		{ "OnPlayerSpawn", "i", "EVENT_PLAYER_SPAWN" },
		{ "OnPlayerDeath", "iii", "EVENT_PLAYER_DEATH" },
		{ "OnVehicleSpawn", "i", "EVENT_VEHICLE_SPAWN" },
		{ "OnVehicleDeath", "ii", "EVENT_VEHICLE_DEATH" },
		{ "OnPlayerText", "is", "EVENT_PLAYER_TEXT" },
		{ "OnPlayerCommandText", "is", "EVENT_PLAYER_COMMAND" },
		{ "OnPlayerRequestClass", "ii", "EVENT_PLAYER_REQUEST_CLASS" },
		{ "OnPlayerEnterVehicle", "iii", "EVENT_PLAYER_ENTER_VEHICLE" },
		{ "OnPlayerExitVehicle", "ii", "EVENT_PLAYER_EXIT_VEHICLE" },
		{ "OnPlayerStateChange", "iii", "EVENT_PLAYER_STATE_CHANGE" },
		{ "OnPlayerEnterCheckpoint", "i", "EVENT_PLAYER_ENTER_CHECKPOINT" },
		{ "OnPlayerLeaveCheckpoint", "i", "EVENT_PLAYER_LEAVE_CHECKPOINT" },
		{ "OnPlayerEnterRaceCheckpoint", "i", "EVENT_PLAYER_ENTER_RACE_CHECKPOINT" },
		{ "OnPlayerLeaveRaceCheckpoint", "i", "EVENT_PLAYER_LEAVE_RACE_CHECKPOINT" },
		{ "OnRconCommand", "s", "EVENT_RCON_COMMAND" },
		{ "OnPlayerRequestSpawn", "i", "EVENT_PLAYER_REQUEST_SPAWN" },
		{ "OnObjectMoved", "i", "EVENT_OBJECT_MOVED" },
		{ "OnPlayerObjectMoved", "ii", "EVENT_PLAYER_OBJECT_MOVED" },
		{ "OnPlayerPickUpPickup", "ii", "EVENT_PLAYER_PICK_UP_PICKUP" },
		{ "OnVehicleMod", "iii", "EVENT_VEHICLE_MOD" },
		{ "OnEnterExitModShop", "iii", "EVENT_ENTER_EXIT_MOD_SHOP" },
		{ "OnVehiclePaintjob", "iii", "EVENT_VEHICLE_PAINT_JOB" },
		{ "OnVehicleRespray", "iiii", "EVENT_VEHICLE_RESPRAY" },
		{ "OnVehicleDamageStatusUpdate", "ii", "EVENT_VEHICLE_DAMAGE_UPDATE" },
		{ "OnUnoccupiedVehicleUpdate", "iiiffffff", "EVENT_ONOCCUPIED_VEHICLE_UPDATE" },
		{ "OnPlayerSelectedMenuRow", "ii", "EVENT_PLAYER_SELECTED_MENU_ROW" },
		{ "OnPlayerExitedMenu", "i", "EVENT_PLAYER_EXITED_MENU" },
		{ "OnPlayerInteriorChange", "iii", "EVENT_PLAYER_INTERIOR_CHANGE" },
		{ "OnPlayerKeyStateChange", "iii", "EVENT_PLAYER_KEY_STATE_CHANGE" },
		{ "OnRconLoginAttempt", "ssi", "EVENT_RCON_LOGIN_ATTEMPT" },
		{ "OnPlayerUpdate", "i", "EVENT_PLAYER_UPDATE" },
		{ "OnPlayerStreamIn", "ii", "EVENT_PLAYER_STREAM_IN" },
		{ "OnPlayerStreamOut", "ii", "EVENT_PLAYER_STREAM_OUT" },
		{ "OnVehicleStreamIn", "ii", "EVENT_VEHICLE_STREAM_IN" },
		{ "OnVehicleStreamOut", "ii", "EVENT_VEHICLE_STREAM_OUT" },
		{ "OnActorStreamIn", "ii", "EVENT_ACTOR_STREAM_IN" },
		{ "OnActorStreamOut", "ii", "EVENT_ACTOR_STREAM_OUT" },
		{ "OnDialogResponse", "iiiis", "EVENT_DIALOG_RESPONSE" },
		{ "OnPlayerTakeDamage", "iifii", "EVENT_PLAYER_TAKE_DAMAGE" },
		{ "OnPlayerGiveDamage", "iifii", "EVENT_PLAYER_GIVE_DAMAGE" },
		{ "OnPlayerGiveDamageActor", "iifii", "EVENT_PLAYER_GIVE_DAMAGE_ACTOR" },
		{ "OnPlayerClickMap", "ifff", "EVENT_PLAYER_CLICK_MAP" },
		{ "OnPlayerClickTextDraw", "ii", "EVENT_PLAYER_CLICK_TEXT_DRAW" },
		{ "OnPlayerClickPlayerTextDraw", "ii", "EVENT_PLAYER_CLICK_PLAYER_TEXT_DRAW" },
		{ "OnIncomingConnection", "isi", "EVENT_INCOMING_CONNECTION" },
		{ "OnTrailerUpdate", "ii", "EVENT_TRAILER_UPDATE" },
		{ "OnVehicleSirenStateChange", "iii", "EVENT_VEHICLE_SIREN_STATE_CHANGE" },
		{ "OnPlayerFinishedDownloading", "ii", "EVENT_PLAYER_FINISHED_DOWNLOADING" },
		{ "OnPlayerRequestDownload", "iii", "EVENT_PLAYER_REQUEST_DOWNLOAD" },
		{ "OnPlayerClickPlayer", "iii", "EVENT_PLAYER_CLICK_PLAYER" },
		{ "OnPlayerEditObject", "iiiiffffff", "EVENT_PLAYER_EDIT_OBJECT" },
		{ "OnPlayerEditAttachedObject", "iiiiifffffffff", "EVENT_PLAYER_EDIT_ATTACHED_OBJECT" },
		{ "OnPlayerSelectObject", "iiiifff", "EVENT_PLAYER_SELECT_OBJECT" },
		{ "OnPlayerWeaponShot", "iiiifff", "EVENT_PLAYER_WEAPON_SHOT" }
	};

	void callback::init()
	{
		for (auto& callback : sampCallbacks)
		{
			event::register_event(callback.name, callback.param_types);
			L_DEBUG << "registered samp callback '" << callback.name << "' in event pool";
		}
	}

	void callback::add_event_definitions(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate>& global)
	{
		for (auto& callback : sampCallbacks)
		{
			sampnode::v8val::add_definition(callback.alias, callback.name, global);
		}
	}
}