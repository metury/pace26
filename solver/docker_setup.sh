#!/bin/bash

set -ueo pipefail

sudo apt-get update

if sudo apt-get install -y scip libscip-dev; then
  echo "========================================================"
  echo "✅ SCIP installed successfully from APT repositories."
  echo "========================================================"
else
  echo "========================================================"
  echo "❌ SCIP was not found in your standard APT repositories."
  echo "========================================================"
  exit 1
fi

mkdir build/
cmake -B build/
cmake --build build/
