#include <vector>
#include <string>
#ifndef WIN32
#include <cstring>
#endif

#include "v8.h"
#include "logger.hpp"
#include "resource.hpp"
#include "events.hpp"
#include "callbacks.hpp"
#include "amx/amx.h"
#include "amxhandler.hpp"
#include "sampgdk.h"

namespace sampnode
{
	bool js_calling_public = false;
	static const std::vector<callback::data> sampCallbacks =
	{
		{ "OnGameModeInit", "", "EVENT_GAME_MODE_INIT" },
		{ "OnGameModeExit", "", "EVENT_GAME_MODE_EXIT" },
		{ "OnFilterScriptInit", "", "EVENT_FILTER_SCRIPT_INIT" },
		{ "OnFilterScriptExit", "", "EVENT_FILTER_SCRIPT_EXIT" },
		{ "OnPlayerConnect", "i", "EVENT_PLAYER_CONNECT" },
		{ "OnPlayerDisconnect", "ii", "EVENT_PLAYER_DISCONNECT" },
		{ "OnPlayerSpawn", "i", "EVENT_PLAYER_SPAWN" },
		{ "OnPlayerDeath", "iii", "EVENT_PLAYER_DEATH" },
		{ "OnVehicleSpawn", "i", "EVENT_VEHICLE_SPAWN" },
		{ "OnVehicleDeath", "ii", "EVENT_VEHICLE_DEATH" },
		{ "OnPlayerText", "is", "EVENT_PLAYER_TEXT" },
		{ "OnPlayerCommandText", "is", "EVENT_PLAYER_COMMAND_TEXT" },
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
		{ "OnVehiclePaintjob", "iii", "EVENT_VEHICLE_PAINTJOB" },
		{ "OnVehicleRespray", "iiii", "EVENT_VEHICLE_RESPRAY" },
		{ "OnVehicleDamageStatusUpdate", "ii", "EVENT_VEHICLE_DAMAGE_STATUS_UPDATE" },
		{ "OnUnoccupiedVehicleUpdate", "iiiffffff", "EVENT_UNOCCUPIED_VEHICLE_UPDATE" },
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
		{ "OnPlayerWeaponShot", "iiiifff", "EVENT_PLAYER_WEAPON_SHOT" },
		{ "OnClientCheckResponse", "iiii", "EVENT_CLIENT_CHECK_RESPONSE" },
		{ "OnScriptCash", "iii", "EVENT_SCRIPT_CASH" }
	};

	void callback::init()
	{
		for (auto& callback : sampCallbacks)
		{
			event::register_event(callback.name, callback.param_types);
		}
	}

	void callback::add_event_definitions(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate>& global)
	{
		for (auto& callback : sampCallbacks)
		{
			sampnode::v8val::add_definition(callback.alias, callback.name, global);
		}
	}

	void callback::call(const v8::FunctionCallbackInfo<v8::Value>& info)
	{
		v8::Isolate* isolate = info.GetIsolate();
		v8::TryCatch eh(isolate);
		int returnValue = 0;

		if (info.Length() > 0)
		{
			v8::String::Utf8Value str(isolate, info[0]);
			std::string name(*str);

			v8::String::Utf8Value str2(isolate, info[1]);
			std::string format(*str2);

			int k = 2;
			cell _params[32];
			std::vector<void*> params;
			int numberOfStrings = 0;
			int arraySizes[32];
			for (int i = 0; i < static_cast<int>(format.length()); i++)
			{
				switch (format[i])
				{
				case 'i':
				case 'd':
				{
					_params[i] = info[k]->Int32Value();
					params.push_back(static_cast<void*>(&_params[i]));
					k++;
				}
				break;
				case 'f':
				{
					float val = 0.0;
					if (!info[k]->IsUndefined()) val = static_cast<float>(info[k]->NumberValue());
					_params[i] = amx_ftoc(val);
					params.push_back(static_cast<void*>(&_params[i]));
					k++;
				}
				break;
				case 's':
				{
					v8::String::Utf8Value _str(isolate, info[k]);
					const char* str(*_str);
					size_t slen = strlen(str);
					char* mystr = new char[slen + 1];
					for (size_t x = 0; x < slen; x++)
					{
						mystr[x] = str[x];
					}
					mystr[slen] = '\0';
					params.push_back(static_cast<void*>(mystr));
					numberOfStrings++;
					k++;
				}
				break;
				case 'a':
				{
					if (!info[k]->IsArray())
					{
						info.GetReturnValue().Set(false);
						L_ERROR << "callPublic: '" << name << "', parameter " << k << "must be an array";
						return;
					}

					v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(info[k]);
					size_t size = a->Length();

					cell* value = new cell[size];

					for (size_t b = 0; b < size; b++)
					{
						value[b] = a->Get(b)->Int32Value();
					}

					params.push_back(static_cast<void*>(value));
					arraySizes[i] = size;
					numberOfStrings++;
					k++;
				}
				break;
				case 'v':
				{
					if (!info[k]->IsArray())
					{
						info.GetReturnValue().Set(false);
						L_ERROR << "callPublic: '" << name << "', parameter " << k << "must be an array";
						return;
					}

					v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(info[k]);
					size_t size = a->Length();

					cell* value = new cell[size];

					for (size_t b = 0; b < size; b++)
					{
						float val = static_cast<float>(a->Get(b)->NumberValue());
						value[b] = amx_ftoc(val);
					}

					params.push_back(static_cast<void*>(value));
					arraySizes[i] = size;
					numberOfStrings++;
					k++;
				}
				break;
				default:
					break;
				}
			}

			std::vector<cell> amx_addr;
			for (int i = 0; i < numberOfStrings; i++)
			{
				amx_addr.push_back(0);
			}
			numberOfStrings = 0;

			for (auto& amx : amx::amx_list)
			{
				int callback = 0;
				if (amx_FindPublic(amx.second->get(), name.c_str(), &callback))
				{
					continue;
				}

				if (callback < -10000)
				{
					continue;
				}

				for (int i = format.length() - 1; i >= 0; i--)
				{
					switch (format[i])
					{
					case 'd':
					case 'i':
					{
						amx_Push(amx.second->get(), *reinterpret_cast<cell*>(params[i]));
					}
					break;
					case 'f':
					{
						amx_Push(amx.second->get(), *reinterpret_cast<cell*>(params[i]));
					}
					break;
					case 's':
					{
						char* string = static_cast<char*>(params[i]);
						if (string != 0 && strlen(string) > 0)
						{
							amx_PushString(amx.second->get(), &amx_addr[numberOfStrings], 0, string, 0, 0);
						}
						else
						{
							*string = 1;
							*(string + 1) = 0;
							amx_PushString(amx.second->get(), &amx_addr[numberOfStrings], 0, string, 0, 0);
						}

						numberOfStrings++;
					}
					break;
					case 'a':
					case 'v':
					{
						cell* amxArray = static_cast<cell*>(params[i]);
						amx_PushArray(amx.second->get(), &amx_addr[numberOfStrings], 0, amxArray, arraySizes[i]);
						numberOfStrings++;
					}
					break;
					default:
						break;
					}
				}

				js_calling_public = true;
				amx_Exec(amx.second->get(), reinterpret_cast<cell*>(&returnValue), callback);
				js_calling_public = false;

				while (numberOfStrings)
				{
					numberOfStrings--;
					amx_Release(amx.second->get(), amx_addr[numberOfStrings]);
				}
			}
		}
		info.GetReturnValue().Set(returnValue);
	}

	void callback::call_float(const v8::FunctionCallbackInfo<v8::Value>& info)
	{
		v8::Isolate* isolate = info.GetIsolate();
		v8::TryCatch eh(isolate);
		int returnValue = 0;

		if (info.Length() > 0)
		{
			v8::String::Utf8Value str(isolate, info[0]);
			std::string name(*str);

			v8::String::Utf8Value str2(isolate, info[1]);
			std::string format(*str2);

			int k = 2;
			cell _params[32];
			std::vector<void*> params;
			int numberOfStrings = 0;
			int arraySizes[32];
			for (int i = 0; i < static_cast<int>(format.length()); i++)
			{
				switch (format[i])
				{
				case 'i':
				case 'd':
				{
					_params[i] = info[k]->Int32Value();
					params.push_back(static_cast<void*>(&_params[i]));
					k++;
				}
				break;
				case 'f':
				{
					float val = 0.0;
					if (!info[k]->IsUndefined()) val = static_cast<float>(info[k]->NumberValue());
					_params[i] = amx_ftoc(val);
					params.push_back(static_cast<void*>(&_params[i]));
					k++;
				}
				break;
				case 's':
				{
					v8::String::Utf8Value _str(isolate, info[k]);
					const char* str(*_str);
					size_t slen = strlen(str);
					char* mystr = new char[slen + 1];
					for (size_t x = 0; x < slen; x++)
					{
						mystr[x] = str[x];
					}
					mystr[slen] = '\0';
					params.push_back(static_cast<void*>(mystr));
					numberOfStrings++;
					k++;
				}
				break;
				case 'a':
				{
					if (!info[k]->IsArray())
					{
						info.GetReturnValue().Set(false);
						L_ERROR << "callPublic: '" << name << "', parameter " << k << "must be an array";
						return;
					}

					v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(info[k]);
					size_t size = a->Length();

					cell* value = new cell[size];

					for (size_t b = 0; b < size; b++)
					{
						value[b] = a->Get(b)->Int32Value();
					}

					params.push_back(static_cast<void*>(value));
					arraySizes[i] = size;
					numberOfStrings++;
					k++;
				}
				break;
				case 'v':
				{
					if (!info[k]->IsArray())
					{
						info.GetReturnValue().Set(false);
						L_ERROR << "callPublic: '" << name << "', parameter " << k << "must be an array";
						return;
					}

					v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(info[k]);
					size_t size = a->Length();

					cell* value = new cell[size];

					for (size_t b = 0; b < size; b++)
					{
						float val = static_cast<float>(a->Get(b)->NumberValue());
						value[b] = amx_ftoc(val);
					}

					params.push_back(static_cast<void*>(value));
					arraySizes[i] = size;
					numberOfStrings++;
					k++;
				}
				break;
				default:
					break;
				}
			}

			std::vector<cell> amx_addr;
			for (int i = 0; i < numberOfStrings; i++)
			{
				amx_addr.push_back(0);
			}
			numberOfStrings = 0;

			for (auto& amx : amx::amx_list)
			{
				int callback = 0;
				if (amx_FindPublic(amx.second->get(), name.c_str(), &callback))
				{
					continue;
				}

				if (callback < -10000)
				{
					continue;
				}

				for (int i = format.length() - 1; i >= 0; i--)
				{
					switch (format[i])
					{
					case 'd':
					case 'i':
					{
						amx_Push(amx.second->get(), *reinterpret_cast<cell*>(params[i]));
					}
					break;
					case 'f':
					{
						amx_Push(amx.second->get(), *reinterpret_cast<cell*>(params[i]));
					}
					break;
					case 's':
					{
						char* string = static_cast<char*>(params[i]);
						if (string != 0 && strlen(string) > 0)
						{
							amx_PushString(amx.second->get(), &amx_addr[numberOfStrings], 0, string, 0, 0);
						}
						else
						{
							*string = 1;
							*(string + 1) = 0;
							amx_PushString(amx.second->get(), &amx_addr[numberOfStrings], 0, string, 0, 0);
						}

						numberOfStrings++;
					}
					break;
					case 'a':
					case 'v':
					{
						cell* amxArray = static_cast<cell*>(params[i]);
						amx_PushArray(amx.second->get(), &amx_addr[numberOfStrings], 0, amxArray, arraySizes[i]);
						numberOfStrings++;
					}
					break;
					default:
						break;
					}
				}

				js_calling_public = true;
				amx_Exec(amx.second->get(), reinterpret_cast<cell*>(&returnValue), callback);
				js_calling_public = false;

				while (numberOfStrings)
				{
					numberOfStrings--;
					amx_Release(amx.second->get(), amx_addr[numberOfStrings]);
				}
			}
		}

		info.GetReturnValue().Set(v8::Number::New(isolate, amx_ctof(returnValue)));
	}
}