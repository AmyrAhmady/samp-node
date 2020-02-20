cmake_minimum_required(VERSION 3.0)

project(samp-node)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/lib/cmake-modules")

if(UNIX)
	set(CMAKE_CXX_FLAGS "-m32 -std=c++11")
	set(CMAKE_C_FLAGS "-m32")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()

if(WIN32 AND MVC)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Debug")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Release")
endif()

# removes the sprintf warnings from plugin-natives
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/test/plugins)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(src)
