#!/bin/bash

# DDS Module install script

# Create build directory and install ddsenabler_participants
echo "Installing ddsenabler_participants..."
mkdir -p build/ddsenabler_participants
cd build/ddsenabler_participants
cmake ../../ddsenabler_participants
sudo cmake --build . --target install
cd ../..

# Create build directory and install ddsenabler_yaml
echo "Installing ddsenabler_yaml..."
mkdir -p build/ddsenabler_yaml
cd build/ddsenabler_yaml
cmake ../../ddsenabler_yaml
sudo cmake --build . --target install
cd ../..

# Create build directory and install ddsenabler
echo "Installing ddsenabler..."
mkdir -p build/ddsenabler
cd build/ddsenabler
cmake ../../ddsenabler
sudo cmake --build . --target install
cd ../..

echo "Installation completed."
