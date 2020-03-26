#include <vector>
#include "singleton.hpp"

namespace sampnode
{
    struct Props_t
    {
        std::string entry_file;
        std::string working_dir;
        std::string resource_folder;
        std::vector<std::string> node_flags;
        std::vector<std::string> resources;
        unsigned char log_level = 4;
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