#!/bin/sh
cd ..
mkdir -p build
cd build
rm -rf *
cmake ..
make && make install