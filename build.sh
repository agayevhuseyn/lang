#!/bin/bash

CC="gcc"
OUT="main"
SRC_DIR="src"
INC_DIR="$SRC_DIR/inc"
OBJ_DIR="obj"
FLAGS=""

mkdir -p $OBJ_DIR

for file in $SRC_DIR/*.c; do
  $CC -c $file -o "$OBJ_DIR/$(basename $file .c).o" -I $INC_DIR 
done

$CC $OBJ_DIR/* -o $OUT $FLAGS

rm -rf $OBJ_DIR
