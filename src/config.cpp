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
	static const std::string CONFIG_FILE_NAME = "samp-node";

	bool Config::ParseFile()
	{
		if (!ParseJsonFile())
		{
			if (!ParseYamlFile())
			{
				return false;
			}
			else
			{
				usingJson = false;
				L_INFO << "plugin is using " << CONFIG_FILE_NAME << ".yml config file";
			}
		}
		else
		{
			usingJson = true;
			L_INFO << "plugin is using " << CONFIG_FILE_NAME << ".json config file";
		}

		props.entry_file = get_as<std::string>("entry_file");
		props.workspace_path = get_as<std::string>("workspace_path");
		props.resources_path = get_as<std::string>("resources_path");
		props.resources = get_as<std::vector<std::string>>("resources");
		props.node_flags = get_as<std::vector<std::string>>("node_flags");
		props.log_level = static_cast<LogLevel>(get_as<int>("log_level"));

		return true;
	}

	bool Config::ParseJsonFile()
	{
		std::ifstream jsonFile(CONFIG_FILE_NAME + ".json");

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

	bool Config::ParseYamlFile()
	{
		YAML::Node object;

		try
		{
			object = YAML::LoadFile(CONFIG_FILE_NAME + ".yml");
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

	Props_t& Config::Props()
	{
		return props;
	}

	Config::Config()
	{}

	Config::~Config()
	{}
};