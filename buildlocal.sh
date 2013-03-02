#!/bin/bash

autoreconf --install
mkdir -p build
cd build
../configure --prefix="" && make $1 && cp src/dunelegacy ../
cd ..
