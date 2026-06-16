#!/bin/bash

set -ueo pipefail

for arg in $@; do
  value=$(($arg - 1))
  number=$(printf "%02d\n" "$value")
  file="input/pace26_exact_pub/exact$number.nw"
  time cat "$file" | ./build/pace
done
