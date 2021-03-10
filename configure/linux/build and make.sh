#!/usr/bin/env bash

# CmnLib
cd ../../CmnLib
mkdir build
cd build
cmake ..
make
make install

# CmnMath
cd ../../CmnMath
mkdir build
cd build
cmake ..
make
make install

# CmnIP
cd ../../CmnIP
mkdir build
cd build
cmake ..
make
make install

# CmnIO
cd ../../CmnIO
mkdir build
cd build
cmake ..
make
make install

