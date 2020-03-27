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
				L_INFO << "plugin is using " << CONFIG_FILE_NAME << ".yml config file";
				return true;
			}
		}
		else
		{
			L_INFO << "plugin is using " << CONFIG_FILE_NAME << ".josn config file";
			return true;
		}
	}

	bool Config::ParseJsonFile()
	{
		std::ifstream jsonFile(CONFIG_FILE_NAME + ".json");
		if (!jsonFile.good())
		{
			return false;
		}
		json jsonObject;
		jsonFile >> jsonObject;
		if (!jsonObject.is_null())
		{
			if (!jsonObject["entry_file"].is_null())
			{
				props.entry_file = jsonObject["entry_file"].get<std::string>();
			}
			else
			{
				props.entry_file = "./index.js";
			}

			if (!jsonObject["working_dir"].is_null())
			{
				props.working_dir = jsonObject["working_dir"].get<std::string>();
			}

			if (!jsonObject["resource_folder"].is_null())
			{
				props.resource_folder = jsonObject["resource_folder"].get<std::string>();
			}

			if (!jsonObject["node_flags"].is_null())
			{
				const std::vector<std::string>& node_flags = jsonObject["node_flags"];

				for (auto& flag : node_flags)
				{
					if (flag.empty())
					{
						continue;
					}
					props.node_flags.push_back(flag);
				}
			}

			if (!jsonObject["resources"].is_null())
			{
				const std::vector<std::string> resources = jsonObject["resources"];

				for (auto& resource : resources)
				{
					if (resource.empty())
					{
						continue;
					}
					props.resources.push_back(resource);
				}
			}
			return true;
		}
		return false;
	}

	bool Config::ParseYamlFile()
	{
		YAML::Node yamlObject;

		try
		{
			yamlObject = YAML::LoadFile(CONFIG_FILE_NAME + ".yml");
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

		if (yamlObject["plugin"])
		{
			if (yamlObject["plugin"]["entry_file"])
			{
				props.entry_file = yamlObject["plugin"]["entry_file"].as<std::string>();
			}
			else
			{
				props.entry_file = "./index.js";
			}

			if (yamlObject["plugin"]["working_dir"])
			{
				props.working_dir = yamlObject["plugin"]["working_dir"].as<std::string>();
			}

			if (yamlObject["plugin"]["resource_folder"])
			{
				props.resource_folder = yamlObject["plugin"]["resource_folder"].as<std::string>();
			}

			if (yamlObject["plugin"]["node_flags"])
			{
				YAML::Node const& node_flags = yamlObject["plugin"]["node_flags"];

				for (YAML::const_iterator y_it = node_flags.begin(); y_it != node_flags.end(); ++y_it)
				{
					std::string flag = y_it->as<std::string>();

					if (flag.empty())
					{
						continue;
					}

					props.node_flags.push_back(flag);
				}
			}

			if (yamlObject["plugin"]["resources"])
			{
				YAML::Node const& resources = yamlObject["plugin"]["resources"];

				for (YAML::const_iterator y_it = resources.begin(); y_it != resources.end(); ++y_it)
				{
					std::string resource_name = y_it->as<std::string>();

					if (resource_name.empty())
					{
						continue;
					}

					props.resources.push_back(resource_name);
				}
			}
			return true;
		}
		return false;
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