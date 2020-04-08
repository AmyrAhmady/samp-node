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
		std::string working_dir;
		std::string resource_folder;
		std::vector<std::string> node_flags;
		std::vector<std::string> resources;
		LogLevel log_level = LogLevel::LOG_FULL;
	};

	class Config
	{
	public:
		Config();
		~Config();

		bool ParseFile();
		bool ParseYamlFile();
		bool ParseJsonFile();

		template<typename T, typename... args>
		T get_as(const args&... keys);

		Props_t& Props();

	private:
		Props_t props;
		json jsonObject;
		YAML::Node yamlObject;
		bool usingJson = true;
	};
};