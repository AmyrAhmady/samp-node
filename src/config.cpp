#include <string>
#include <fstream>
#include <vector>
#include "json.hpp"
#include "logger.hpp"
#include "config.hpp"

using json = nlohmann::json;

namespace sampnode
{
	bool Config::ParseFile(const std::string& path)
	{
		if (!ParseJsonFile(path))
		{
			return false;
		}
		else
		{
			L_INFO << "plugin is using " << path << ".json config file";
		}
		return true;
	}

	bool Config::ParseJsonFile(const std::string& path)
	{
		std::ifstream jsonFile(path + ".json");

		if (!jsonFile.good())
		{
			L_INFO << "unable to locate JSON config file at " << path + ".json.";
			return false;
		}

		json object;
		jsonFile >> object;

		if (!object.is_null())
		{
			jsonObject = object;
			return true;
		}
		return false;
	}

	Props_t Config::ReadAsMainConfig()
	{
		return Props_t{
			get_as<std::string>("entry_file"),
			get_as<bool>("enable_resources"),
			get_as<std::string>("resources_path"),
			get_as<std::vector<std::string>>("node_flags"),
			get_as<std::vector<std::string>>("resources"),
			static_cast<LogLevel>(get_as<int>("log_level"))
		};
	}

	template<typename T, typename... args>
	T Config::get_as(const args&... keys)
	{
		std::vector<std::string> _keys = { keys... };

		json tempJsonObj = jsonObject;
		for (auto& key : _keys)
		{
			if (!tempJsonObj[key].is_null())
			{
				tempJsonObj = tempJsonObj[key];
			}
			else
			{
				return T();
			}
		}
		return tempJsonObj.get<T>();
	}

	Config::Config()
	{}

	Config::~Config()
	{}
};