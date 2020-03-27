cmake_minimum_required(VERSION 3.0)

# -
# Plugin
# -

if (NOT PLUGIN_VERSION)
	set(PLUGIN_VERSION "0.2.0")
endif()

project(samp-node VERSION ${PLUGIN_VERSION})

# -
# Paths
# -

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test/plugins)
set(CMAKE_INSTALL_PREFIX ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/deps/cmake-modules")

if(MSVC)
	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)		
endif()	

# -
# Compiler Flags
# -

if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++14")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so;.so.64")
	add_link_options(
		"-static-libgcc"
		"-static-libstdc++"
	)		

	set(CPACK_SYSTEM_NAME "linux")
endif()

if(WIN32)
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
		-DWIN32_LEAN_AND_MEAN
	)	
endif()

# -
# Directories
# -

add_subdirectory(deps/yaml-cpp)
add_subdirectory(src)
