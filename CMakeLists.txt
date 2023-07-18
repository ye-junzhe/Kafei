cmake_minimum_required(VERSION 3.13)
project(Walnut)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

# Define an option for the build type
set(BUILD_TYPE "Release" CACHE STRING "Choose the build type (Release/Debug)")

# Set the build type based on the user input
if(BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
elseif(BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
else()
    message(FATAL_ERROR "Invalid build type. Valid options are 'Release' and 'Debug'")
endif()

add_subdirectory(vendor)
add_subdirectory(Walnut)
add_subdirectory(WalnutApp)
