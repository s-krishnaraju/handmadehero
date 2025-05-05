#!/bin/bash
cd "$(dirname "$0")"
mkdir -p ./build
cd ./build
c++ ../src/sdl_handmade.cpp -o handmadehero -g `sdl2-config --cflags --libs`
./handmadehero 