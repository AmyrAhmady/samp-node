include(AMXConfig)
include(AddSAMPPlugin)

set(SAMP_SDK_ROOT "${PROJECT_SOURCE_DIR}/lib/samp-plugin-sdk")
find_package(SAMPSDK REQUIRED)

if (WIN32)
    set(NODE_DLL node.dll)

    if (WIN64)
        set(SYS_PATH win64)
    else()
        set(SYS_PATH win32)
    endif()
elseif(APPLE)
    set(NODE_DLL libnode.59.dylib)
    set(SYS_PATH mac)
else()
    set(NODE_DLL libnode.so.59)
    set(SYS_PATH lin)
endif()

link_directories(${PROJECT_SOURCE_DIR}/lib/${SYS_PATH})
add_definitions(-DBUILDING_NODE_EXTENSION=1 -DEXTERNAL_NAPI=1)

include_directories(
	${SAMPSDK_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/include/node
	${PROJECT_SOURCE_DIR}/include/v8
	${PROJECT_SOURCE_DIR}/include/uv
)

add_samp_plugin(${PROJECT_NAME}
	${SAMPSDK_DIR}/amxplugin.cpp
	${SAMPSDK_DIR}/amxplugin2.cpp
	${SAMPSDK_DIR}/amx/getch.c
	common.hpp
	main.cpp
	impl.cpp
	impl.hpp
	natives.cpp
	natives.hpp
	node.cpp
	plugin.def
)

if (WIN32)
    set(LIBRARIES v8_libbase v8_libplatform node shlwapi dbghelp winmm)
else()
    set(LIBRARIES v8_libbase v8_libplatform node pthread dl m)

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_SOURCE_DIR}/lib/${SYS_PATH}/${NODE_DLL}" "${CMAKE_SOURCE_DIR}/lib/${SYS_PATH}/libnode.so"
    )
endif()

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if(MSVC)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endif()