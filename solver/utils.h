/// @file utils.h
/// @brief Common utility functions.
/// Common functions like string spliting and comparing float numbers.
#ifndef utils_h_
#define utils_h_

#include <cstdint>
#include <string>
#include <vector>

/// Common RED colos for colorful printing.
const std::string RED = "\033[1;31m";
/// Common GREEN colos for colorful printing.
const std::string GREEN = "\033[1;32m";
/// Common YELLOW colos for colorful printing.
const std::string YELLOW = "\033[1;33m";
/// Common BLUE colos for colorful printing.
const std::string BLUE = "\033[1;34m";
/// Common VIOLET colos for colorful printing.
const std::string VIOLET = "\033[1;35m";
/// Common CYAN colos for colorful printing.
const std::string CYAN = "\033[1;36m";
/// Common RESET colos for colorful printing.
const std::string RESET = "\033[0m";

/// Compare float value to "almost one".
/// @param x Given value to comparing to one.
/// @param tolerance How far from one it can be.
/// @return Whether x is at most tolerance far away from 1.
bool is_approx_one(float x, float tolerance = 1e-6f);

/// Split strings by space.
/// @param str Input string.
/// @return Vector of all substrings.
std::vector<std::string> split(const std::string &str);

/// Split strings by delimiter.
/// @param str Input string.
/// @param delimiter Given delimiter.
/// @return Vector of all substrings.
std::vector<std::string> split(const std::string &str, char delimiter);

/// Decrement all values by one but not below 0.
/// @param args Values.
template <typename... Ints> void decrement_to_zero(Ints &...args) {
  ((args = std::max(0, args - 1)), ...);
}

/// Check if at least one value is positive.
/// @param args Values.
/// @return True if at least one value is positive.
template <typename... Ints> bool has_positive(Ints &...args) {
  return (... || (args > 0));
}

/// Structure holding incompatible Trio.
struct Trio {
  uint16_t a, b, c;
  uint16_t size = 0;
  bool used = false;
};

/// Structure holding incompatible Quartet.
struct Quartet {
  uint16_t a, b, x, y;
  uint16_t size = 0;
  bool used = false;
};

/// Structure holding Fork.
struct Fork {
  uint16_t parent, left, right;
};
#endif
