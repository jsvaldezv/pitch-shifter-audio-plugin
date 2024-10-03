# ==================================== Main Configuration ===================================== #
cmake_minimum_required(VERSION 3.21)

# Read version and plugin name and create variables
file(READ "plugin.cfg" CONFIG_CONTENT)

# Using regular expressions to extract values without newline characters
string(REGEX MATCH "PLUGIN_NAME=([^\n]*)" _ ${CONFIG_CONTENT})
set(PROJECT_NAME ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION=([^\n]*)" _ ${CONFIG_CONTENT})
set(CURRENT_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "CODE=([^\n]*)" _ ${CONFIG_CONTENT})
set(CURRENT_PLUGIN_CODE ${CMAKE_MATCH_1})

message(STATUS "Plugin Name: ${PROJECT_NAME}")
message(STATUS "Version: ${CURRENT_VERSION}")
message(STATUS "Plugin Code: ${CURRENT_PLUGIN_CODE}")

# Setting the project name and version correctly
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

set(CPP_VERSION cxx_std_20)
set(COMPANY_NAME "jsvaldezv")
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Support macOS down to High Sierra" FORCE)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;PreRelease" CACHE STRING "Configurations" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
include(SetSourceFiles)
include(SubProjectVersion)

# =========================================== JUCE ============================================== #
add_subdirectory(Libraries/JUCE)

subproject_version(JUCE juce_version)
message(STATUS "JUCE version: ${juce_version}")

# ========================================= IDE env variables ========================================== #
set(CMAKE_XCODE_SCHEME_ENVIRONMENT OS_ACTIVITY_MODE=disable)

# ============================================= Plug-in ================================================ #
message(STATUS "Generator: ${CMAKE_GENERATOR}")
add_subdirectory(Libraries/rubberband)
add_subdirectory(Source)

# ============================================= Warnings =============================================== #
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
endif()