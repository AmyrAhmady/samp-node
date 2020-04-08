#include <string>
#include <fstream>
#include <vector>
#include <yaml-cpp/yaml.h>
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
			if (!ParseYamlFile(path))
			{
				return false;
			}
			else
			{
				usingJson = false;
				L_INFO << "plugin is using " << path << ".yml config file";
			}
		}
		else
		{
			usingJson = true;
			L_INFO << "plugin is using " << path << ".json config file";
		}
		return true;
	}

	bool Config::ParseJsonFile(const std::string& path)
	{
		std::ifstream jsonFile(path + ".json");

		if (!jsonFile.good())
		{
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

	bool Config::ParseYamlFile(const std::string& path)
	{
		YAML::Node object;

		try
		{
			object = YAML::LoadFile(path + ".yml");
		}
		catch (const YAML::ParserException & e)
		{
			L_ERROR << "Could not parse the config file: " << e.what();
			return false;
		}
		catch (const YAML::BadFile&)
		{
			return false;
		}

		if (object)
		{
			yamlObject = object;
			return true;
		}
		return false;
	}

	Props_t Config::ReadAsMainConfig()
	{
		return Props_t{
			get_as<std::string>("entry_file"),
			get_as<std::string>("workspace_path"),
			get_as<std::string>("resources_path"),
			get_as<std::vector<std::string>>("resources"),
			get_as<std::vector<std::string>>("node_flags"),
			static_cast<LogLevel>(get_as<int>("log_level"))
		};
	}

	template<typename T, typename... args>
	T Config::get_as(const args&... keys)
	{
		std::vector<std::string> _keys = { keys... };

		if (usingJson)
		{
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
		else
		{

			YAML::Node tempYamlObj = YAML::Clone(yamlObject);
			for (auto& key : _keys)
			{
				if (!tempYamlObj[key].IsNull())
				{
					tempYamlObj = tempYamlObj[key];
				}
				else
				{
					return T();
				}
			}
			return tempYamlObj.as<T>();
		}
	}

	Config::Config()
	{}

	Config::~Config()
	{}
};