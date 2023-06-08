#!/bin/bash

# Exit if any command fails
set -e

workspace=$(pwd)

cd ${workspace}

# If the build directory exists, delete it
if [ -d "build" ]; then
  echo "Deleting old build directory..."
  rm -r build
fi

# Create the build directory
echo "Creating new build directory..."
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
if ! cmake ..; then
  echo "CMake failed!"
  exit 1
fi

# Run Make
echo "Running Make..."
if ! make -j$(nproc); then
  echo "Make failed!"
  exit 1
fi

echo -e "Build completed successfully! :) \n"
echo

# Ask the user if they want to run the game
read -p "Do you want to run the game? (y/n) " answer
case ${answer:0:1} in
    y|Y )
        # Ask the user for their piece type
        read -p "Choose a piece type (X or O): " piece_type
        echo

        # Run the game with the chosen piece type
        ./Othello $piece_type
    ;;
    * )
        echo "Not running the game. You can run it later with './build/Othello <X|O>'."
    ;;
esac
