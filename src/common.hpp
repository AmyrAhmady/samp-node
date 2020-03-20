typedef void(*logprintf_t)(const char* format, ...);
extern logprintf_t logprintf;
extern void** ppPluginData;
extern void* pAMXFunctions;

#include "logger.hpp"
#include "utils.hpp"