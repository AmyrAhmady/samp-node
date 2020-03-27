#include <vector>
#include "singleton.hpp"
#include "logger.hpp"

namespace sampnode
{
	struct Props_t
	{
		std::string entry_file;
		std::string working_dir;
		std::string resource_folder;
		std::vector<std::string> node_flags;
		std::vector<std::string> resources;
		LogLevel log_level = LogLevel::LOG_INFO;
	};

	class Config : public Singleton<Config>
	{
		friend class Singleton<Config>;
	public:
		Config();
		~Config();

		bool ParseFile();
		bool ParseYamlFile();
		bool ParseJsonFile();
		Props_t& Props();

	private:
		Props_t props;

		bool usingJson = true;
	};
};