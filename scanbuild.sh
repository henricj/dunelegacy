#!/bin/bash

./clean

mkdir -p build
cd build
scan-build ../configure --prefix="" --enable-debug && scan-build make
cd ..
