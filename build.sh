#!/bin/bash
cd "$(dirname "$0")"
mkdir -p ./build
cd ./build
c++ -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 ../src/sdl_handmade.cpp -o handmadehero -g -Wall -Wno-unused-variable -Wno-unused-function `sdl2-config --cflags --libs` 