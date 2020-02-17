#!/bin/bash

# This bash script is running inside the Docker container
# to actually compile the cIOS. It first creates the
# stripios binary, then runs the maked2x script, while
# including the git commit ID in the version string.

# See wiki, this is needed to be able to compile outside of Github environment.
cd /docker-mountpoint || /bin/true

cd stripios
g++ main.cpp -o stripios
cp stripios ../source/stripios
cd ..
dos2unix ./maked2x.sh
./maked2x.sh 999 git+$(git rev-parse --short HEAD)
