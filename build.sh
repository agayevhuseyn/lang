#!/bin/bash

# Default settings
CC="gcc"  # Default to GCC for Linux
OUT="main" # Default output name for Linux
SRC_DIR="src"
INC_DIR="$SRC_DIR/inc"
OBJ_DIR="obj"
FLAGS=""

# Function to display help message
usage() {
  echo "Usage: $0 [--windows]"
  echo "  --windows    Compile for Windows (using MinGW)"
  echo "  (default)    Compile for Linux"
}

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --windows)
      CC="x86_64-w64-mingw32-gcc"  # Set cross-compiler for Windows
      OUT="main.exe"  # Windows executables have .exe extension
      shift  # Move to the next argument
      ;;
    *)
      usage  # Invalid argument, show usage
      exit 1
      ;;
  esac
done

# Create the object directory if it doesn't exist
mkdir -p $OBJ_DIR

# Compile each .c file to an object file
for file in $SRC_DIR/*.c; do
  $CC -c $file -o "$OBJ_DIR/$(basename $file .c).o" -I $INC_DIR
done

# Link all object files to create the executable
$CC $OBJ_DIR/* -o $OUT $FLAGS

# Clean up the object directory
rm -rf $OBJ_DIR
