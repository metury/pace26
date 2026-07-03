#!/bin/bash

set -ueo pipefail

sudo apt-get update
sudo apt-get install -y build-essential cmake scip libscip-dev

mkdir build/
cmake -B build/
cmake --build build/
