include(AMXConfig)
include(AddSAMPPlugin)

# -
# Definitions
# - 

add_definitions(-DBUILDING_NODE_EXTENSION=1 -DEXTERNAL_NAPI=1)

# -
# Paths
# -

set(SAMP_SDK_ROOT "${PROJECT_SOURCE_DIR}/lib/samp-plugin-sdk")

if (WIN32)
    if (WIN64)
        set(SYS_PATH win64)
    else()
        set(SYS_PATH win32)
    endif()
else()
    set(SYS_PATH linux)
endif()

set(NODE_PATH "${PROJECT_SOURCE_DIR}/lib/node/${SYS_PATH}")

# -
# Dependencies
# -

find_package(SAMPSDK REQUIRED)

include_directories(
	${SAMPSDK_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/include/node
	${PROJECT_SOURCE_DIR}/include/v8
	${PROJECT_SOURCE_DIR}/include/uv
)

# -
# Linking
# - 

link_directories(${NODE_PATH})

add_samp_plugin(${PROJECT_NAME}
	${SAMPSDK_DIR}/amxplugin.cpp
	${SAMPSDK_DIR}/amxplugin2.cpp
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
	
	if(UNIX)
		target_sources(${PROJECT_NAME} PUBLIC ${SAMPSDK_DIR}/amx/getch.c)
	endif()
endif()

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# -
# C(++) Version
# -

if(MSVC)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endif()

# -
# CPack
# - 

set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/build/cpack)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "./")
set(CPACK_PACKAGE_VERSION ${PLUGIN_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

install(FILES ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.inc DESTINATION ".")
install(TARGETS ${PROJECT_NAME} DESTINATION "./")

if(WIN32)
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_GENERATOR TGZ)
endif()

include(CPack)