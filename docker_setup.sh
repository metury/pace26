#!/bin/bash

set -ueo pipefail

sudo apt update

SCIP="scipoptsuite_10.0.2-1+trixie_amd64.deb"

wget https://scipopt.org/download/release/"$SCIP"

sudo apt install -y ./"$SCIP"

sudo apt install -y build-essential cmake

rm -rf "$SCIP"

mkdir -p build/
cmake -B build/
cmake --build build/
