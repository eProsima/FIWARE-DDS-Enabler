#!/bin/bash

# DDS Module install script

# Default values for flags
DEBUG_MODE=0
BUILD_TESTS=0

# Parse input arguments
for arg in "$@"
do
    case $arg in
        --debug)
        DEBUG_MODE=1
        echo "Debug mode enabled."
        shift
        ;;
        --tests)
        BUILD_TESTS=1
        echo "Building with tests enabled."
        shift
        ;;
        *)
        echo "Unknown option: $arg"
        exit 1
        ;;
    esac
done

# Function to run installation steps with optional debug mode and test building
install_module() {
    local module_name=$1
    echo "Installing ${module_name}..."

    # Create build directory
    mkdir -p build/${module_name}
    cd build/${module_name}

    # Base cmake command
    cmake_cmd="cmake ../../${module_name}"

    # Add debug mode flag if enabled
    if [[ $DEBUG_MODE -eq 1 ]]; then
        cmake_cmd="$cmake_cmd -DCMAKE_BUILD_TYPE=Debug"
    fi

    # Add testing flag if enabled
    if [[ $BUILD_TESTS -eq 1 ]]; then
        cmake_cmd="$cmake_cmd -DBUILD_TESTING=ON"
    fi

    # Run the cmake command
    eval $cmake_cmd

    # Build and install the module
    sudo cmake --build . --target install

    # Go back to the root directory
    cd ../..
}

# Install modules
install_module "ddsenabler_participants"
install_module "ddsenabler_yaml"
install_module "ddsenabler"

echo "Installation completed."
