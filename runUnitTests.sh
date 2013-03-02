#!/bin/bash

mkdir -p build
cd build
../configure --prefix="" --enable-debug
cd tests
make check
cd ..
cd ..
