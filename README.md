# [Pace Challenge 2026](https://pacechallenge.org/2026/) - Maximum Agreement Forest

This is a solver for the Pace Challenge 2026, where the task was to find maximum agreement forest. This includes only exact track solver.

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

### Some Additional Notes

To keep consistency of the code you could use the following.

```bash
clang-format *.cpp *.h
```

Otherwise we will stick to snake notation (for example: `get_my_favorite_value()`) for functions and upper camel case (`MyFavoriteClass`) for classes.
Whenever we have some private functions or values we use `_` at the end, just to clarify.

Lastly we also want to keep some structure and documentation. Therefore stick to using doxygen documentation.

```cpp
/// This is the function.
/// @param value Its given value.
/// @return If it is odd.
bool is_odd(int value);
```

Therefore we could use doxygen to generate documentation.

```bash
doxygen

cd docs

# Open web page documentation.
xdg-open html/index.html

# Or create PDF version.
cd latex
make
xdg-open refman.pdf
```

# Submitting and compiling the code

In order to compile this one must have compiled highs library (ideally in musl for optil) and libz to static libraries. Then one can link the compilation to these libraries and set it to static as well.
