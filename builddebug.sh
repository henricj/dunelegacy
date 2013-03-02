#!/bin/bash

mkdir -p build
cd build
../configure --prefix="" --enable-debug && make $1 && cp src/dunelegacy ../
cd ..
