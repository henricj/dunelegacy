#!/bin/bash

mkdir -p build
cd build
export CC=clang++
export CXX=clang++
../configure --prefix="" --enable-debug && make $1 && cp src/dunelegacy ../
cd ..
