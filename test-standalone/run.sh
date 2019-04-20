#!/bin/sh -e
make -C ../asc
make -C ../standalone
node ../asc/build/asc.js -pcl -d ../test
find ../standalone/ -iname "*.gcda" -delete
../standalone/vm ../test/__output/image.bin
mkdir -p gcovr
gcovr -r ../standalone/ --html --html-details -o gcovr/index.html