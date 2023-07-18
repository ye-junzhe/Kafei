#!/bin/bash

# Ninja as default generator,
# But if Ninja is not installed, just select cmake
build_system="Ninja"

read -rp "Build from source? (y/n): " build_from_source
read -rp "Use CMake or Ninja as the build system? (cmake/ninja): " selected_build_system

if [[ $selected_build_system == "cmake" ]]; then
    build_system="Unix Makefiles"
else
    build_system="Ninja"
fi

if [[ $build_from_source == [Yy] ]]; then
    echo "Building from ground up using $build_system."
    if [[ $build_system == "Ninja" ]]; then
        rm -rf ninja-build
        cmake -S . -B ninja-build -DCMAKE_BUILD_TYPE=Release -G"$build_system"
        cd ninja-build || exit
        ninja
    else
        rm -rf build
        cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G"$build_system"
        cd build || exit
        make
    fi
else
    echo "Building only WalnutApp"

    if [[ $build_system == "Ninja" ]]; then
        cmake --build ./ninja-build --target WalnutApp
        cd ninja-build || exit
        ninja
    else
        cmake --build ./build --target WalnutApp
        cd build || exit
        make
    fi
fi

./WalnutApp/WalnutApp
