# Maximum Agreement Forest problem solver

So far only few things were done... More will follow.

## Compilation and other staff

To compile use cmake and then make. Afterwards you may run the program. Inputs are assumed to be in files presented as arguments.

```bash
mkdir build
cmake -B build
cmake --build build
./build/pace input1 input2 ...
```

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
