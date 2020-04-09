#pragma once
#include <vector>
#include <yaml-cpp/yaml.h>
#include "json.hpp"
#include "logger.hpp"

using json = nlohmann::json;

namespace sampnode
{
	struct Props_t
	{
		std::string entry_file;
		std::string resources_path;
		std::vector<std::string> node_flags;
		std::vector<std::string> resources;
		LogLevel log_level = LogLevel::LOG_FULL;
	};

	class Config
	{
	public:
		Config();
		~Config();

		bool ParseFile(const std::string& path);
		bool ParseYamlFile(const std::string& path);
		bool ParseJsonFile(const std::string& path);

		template<typename T, typename... args>
		T get_as(const args&... keys);

		Props_t ReadAsMainConfig();

	private:
		json jsonObject;
		YAML::Node yamlObject;
		bool usingJson = true;
	};
};