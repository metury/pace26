# Installation Guide

This is a `C++` project that has CMake build system. Therefore in order to compile this the dependencies must be installed.

## Dependencies

There is only one dependency, if we are not counting `C++` default libraries, and that is [SCIP optimization suite](www.scipopt.org). So if this is installed and the libraries are known then the compilation is a piece of cake.

## Compilation

To compile we only need to use standard series of CMake related calls.

```bash
mkdir build/
cmake -B build/
cmake --build build/
```
