#!/bin/bash

cd stripios
g++ main.cpp -o stripios
cp stripios ../source/stripios
cd ..
dos2unix ./maked2x.sh
./maked2x.sh 999 git=$(git rev-parse --short HEAD)
