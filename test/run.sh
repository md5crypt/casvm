#!/bin/sh -e
make -C ../asc
make -C ../standalone
node ../asc/build/asc.js -pcl
find ../standalone/ -iname "*.gcda" -delete
../standalone/vm __output/image.bin
mkdir -p gcovr
gcovr -r ../standalone/ --html --html-details -o gcovr/index.html