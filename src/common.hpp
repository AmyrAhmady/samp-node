extern void** ppPluginData;
extern void* pAMXFunctions;
typedef void (*logprintf_t)(const char* szFormat, ...);
extern logprintf_t logprintf;

#define TRY(exp) do { \
    napi_status status; \
    status = exp; \
    if (status != napi_ok) { \
        napi_throw_error(env, NULL, "N-API call failed"); \
    } \
} while(0)

#define REGISTER_METHOD(env, object, name, func) do { \
    napi_value fn; \
    TRY(napi_create_function(env, NULL, 0, func, NULL, &fn)); \
    TRY(napi_set_named_property(env, object, name, fn)); \
} while(0)

#define REGISTER_MEMBER(type, env, object, name, value) do { \
    napi_value val; \
    TRY(napi_create_##type (env, value, &val)); \
    TRY(napi_set_named_property(env, object, name, val)); \
} while (0)

// Newline at end of file prevents gcc giving a warning:
// warning: backslash-newline at end of file