#!/bin/bash

set -ueo pipefail

cmake --build build

for file in $@; do
  ./build/pace "$file"
  picat sat.pi
done
