#!/bin/bash
mkdir -p ../build
pushd ../build 
c++ ../src/sdl_handmade.cpp -o handmadehero -g `sdl2-config --cflags --libs`
popd