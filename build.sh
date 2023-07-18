#!/bin/bash

# Function to check if a command executed successfully
check_success() {
    if [ $? -ne 0 ]; then
        echo "Error: $1 failed."
        exit 1
    fi
}

echo "Cloning the latest glm, glfw, and imgui libraries"

# Clone glm
git clone https://github.com/g-truc/glm
check_success "Cloning glm"

# Clone glfw
git clone https://github.com/glfw/glfw.git
check_success "Cloning glfw"

# Clone imgui (docking branch)
git clone https://github.com/ocornut/imgui --branch docking
check_success "Cloning imgui"

# Move directories to vendor folder
cp -r glm/ vendor/glm
check_success "Moving glm"

cp -r glfw/ vendor/GLFW
check_success "Copying glfw"

cp -r imgui/ vendor/imgui
check_success "Moving imgui"

# Delete original directories
rm -rf glm/
check_success "Deleting glm"

rm -rf glfw/
check_success "Deleting glfw"

rm -rf imgui/
check_success "Deleting imgui"

echo "Building project for architecture arm64"

# Create build directory and generate build files
cmake -S . -B build
check_success "CMake configuration"

# Build project
cd build || exit
make
check_success "Build"

# Run the WalnutApp executable
./WalnutApp/WalnutApp
check_success "Running WalnutApp"
