#!/bin/bash

set -ueo pipefail

sudo apt update

wget https://scipopt.org/download/release/scipoptsuite_10.0.2-1+trixie_amd64.deb

sudo apt install -y ./scipoptsuite_*

sudo apt install -y build-essential cmake

rm -rf scipoptsuite_10*

mkdir -p build/
cmake -B build/
cmake --build build/
