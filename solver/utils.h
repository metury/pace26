/// @file utils.h
/// @brief Common utility functions.
/// Common functions like string spliting and comparing float numbers.
#ifndef utils_h_
#define utils_h_

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

int get_matrix_index(int edge_index, int size, int first_vertex = 0,
                     int second_vertex = 0);
#endif
