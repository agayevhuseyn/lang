#!/bin/bash

CC="gcc"
OUT="main"
SRC_DIR="src"
INC_DIR="$SRC_DIR/inc"
OBJ_DIR="obj"
FLAGS=""

usage() {
  echo "Usage: $0 [--windows]"
  echo "  --windows    Compile for Windows (using MinGW)"
  echo "  (default)    Compile for Linux"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --windows)
      CC="x86_64-w64-mingw32-gcc"
      OUT="main.exe"
      shift
      ;;
    *)
      usage
      exit 1
      ;;
  esac
done

mkdir -p $OBJ_DIR

for file in $SRC_DIR/*.c; do
  $CC -c $file -o "$OBJ_DIR/$(basename $file .c).o" -I $INC_DIR
done

$CC $OBJ_DIR/* -o $OUT $FLAGS

rm -rf $OBJ_DIR
