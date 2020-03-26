#include <vector>
#include "singleton.hpp"
#include <yaml-cpp/yaml.h>

namespace sampnode
{
    struct Props_t
    {
        std::string entry_file;
        std::string working_dir;
        std::string resource_folder;
        std::vector<std::string> node_flags;
        std::vector<std::string> resources;
    };

    class Config : public Singleton<Config>
    {
        friend class Singleton<Config>;
    public:
        Config();
        ~Config();
        
        bool ParseFile();
        Props_t& Config::Props();      

    private:
        Props_t props;

        bool usingJson = true;
    };
};