#include <string>
#include "logger.hpp"
#include "config.hpp"

namespace sampnode
{
	static const std::string CONFIG_FILE_NAME = "samp-node";

	bool Config::ParseFile()
	{
		YAML::Node root;

		try
		{
			root = YAML::LoadFile(CONFIG_FILE_NAME + ".yml");
		}
		catch (const YAML::ParserException & e)
		{
			L_ERROR << "Could not parse the config file: " << e.what();
			return false;
		}
		catch (const YAML::BadFile&)
		{
			L_ERROR << "Could not find the config file.";
			return false;
		}

		if (root["plugin"])
		{
			if (root["plugin"]["entry_file"])
			{
				props.entry_file = root["plugin"]["entry_file"].as<std::string>();
			}
			else
			{
				props.entry_file = "./index.js";
			}

			if (root["plugin"]["working_dir"])
			{
				props.working_dir = root["plugin"]["working_dir"].as<std::string>();
			}

			if (root["plugin"]["resource_folder"])
			{
				props.resource_folder = root["plugin"]["resource_folder"].as<std::string>();
			}

			if (root["plugin"]["node_flags"])
			{
				YAML::Node const& node_flags = root["plugin"]["node_flags"];

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

			if (root["plugin"]["resources"])
			{
				YAML::Node const& resources = root["plugin"]["resources"];

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
		}

		return true;
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