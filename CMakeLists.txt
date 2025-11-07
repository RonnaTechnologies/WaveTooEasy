cmake_minimum_required(VERSION 3.20)
project(sound_module LANGUAGES CXX VERSION 0.1.0)

include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set a default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Find Catch2
find_package(Catch2 REQUIRED)

# Add the extras directory to CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH ${Catch2_DIR}/extras)

# Your test executable
add_executable(sound_module_tests tests.cxx src/FileSystem.cpp)
target_compile_features(sound_module_tests PRIVATE cxx_std_20)
set_target_properties(sound_module_tests PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(sound_module_tests PRIVATE include)

target_link_libraries(sound_module_tests PRIVATE Catch2::Catch2WithMain)

target_compile_definitions(sound_module_tests PRIVATE $<$<CONFIG:Debug>:TESTS_ENABLED=1>)

# Include Catch and discover tests
include(Catch)
catch_discover_tests(sound_module_tests)
