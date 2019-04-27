#!/bin/sh -e
make -C ../asc
make -C ../emscripten
node ../asc/build/asc.js -pcl -d ../test
node ../emscripten/build/emscripten/test.js