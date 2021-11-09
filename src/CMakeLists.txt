include(AMXConfig)
include(AddSAMPPlugin)

# -
# Node
# -

if(WIN32)
	set(NODE_FILE "node.dll")
else()
	set(NODE_FILE "libnode.so.64")
endif()

# -
# Definitions
# - 

add_definitions(-DBUILDING_NODE_EXTENSION=1 -DEXTERNAL_NAPI=1)

# -
# Paths
# -

set(SAMP_SDK_ROOT "${PROJECT_SOURCE_DIR}/deps/samp-plugin-sdk")

if (WIN32)
	set(SYS_PATH win32)
else()
    set(SYS_PATH linux)
endif()

set(NODE_PATH "${PROJECT_SOURCE_DIR}/deps/node/lib/Release/${SYS_PATH}")

# -
# Dependencies
# -

find_package(SAMPSDK REQUIRED)

include_directories(
	${SAMPSDK_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/deps/sampgdk
	${PROJECT_SOURCE_DIR}/deps/node/include
	${PROJECT_SOURCE_DIR}/deps/v8
	${PROJECT_SOURCE_DIR}/deps/uv
	${PROJECT_SOURCE_DIR}/deps/json/single_include/nlohmann
	${PROJECT_SOURCE_DIR}/deps/tinydir
)

# -
# Linking
# - 

link_directories(${NODE_PATH})

file(GLOB SAMPNODE_FILES
	*.hpp
	*.cpp
)

add_samp_plugin(${PROJECT_NAME}
	${SAMPSDK_DIR}/amxplugin.cpp
	${SAMPSDK_DIR}/amxplugin2.cpp
	${PROJECT_SOURCE_DIR}/deps/sampgdk/sampgdk.c
	${SAMPNODE_FILES}
	plugin.def
)

if (WIN32)
    set(LIBRARIES v8_libbase v8_libplatform node shlwapi dbghelp winmm)	
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES
		INSTALL_RPATH "$ORIGIN/..:$ORIGIN/")
	target_link_libraries(${PROJECT_NAME} ${NODE_PATH}/libnode.so.64)	

	set(LIBRARIES ${node} pthread dl m)		

	if(UNIX)
		target_sources(${PROJECT_NAME} PUBLIC ${SAMPSDK_DIR}/amx/getch.c)
	endif()
endif()

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	${NODE_PATH}/${NODE_FILE} ${PROJECT_SOURCE_DIR}/test/${NODE_FILE}
)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# -
# C(++) Version
# -

if(MSVC)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
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
install(FILES ${NODE_PATH}/${NODE_FILE} DESTINATION ".")
install(TARGETS ${PROJECT_NAME} DESTINATION ".")

if(WIN32)
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_GENERATOR TGZ)
endif()

include(CPack)