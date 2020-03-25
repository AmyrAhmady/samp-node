#include <vector>
#include "singleton.hpp"
#include <yaml-cpp/yaml.h>

namespace sampnode
{
    class Config : public Singleton<Config>
    {
        friend class Singleton<Config>;
    public:
        Config();
        ~Config();
        
        bool ParseFile();
        std::string GetWorkingDirectory();
        std::string GetResourceFolder();
        std::vector<std::string> GetNodeFlags();
        std::vector<std::string> GetResources();        

    private:
        struct Plugin
        {
            std::string working_dir;
            std::string resource_folder;
            std::vector<std::string> node_flags;
            std::vector<std::string> resources;
        } plugin;     
    };
};