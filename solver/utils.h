/// @file utils.h
/// @brief Common utility functions.
/// Common functions like string spliting and comparing float numbers.
#ifndef utils_h_
#define utils_h_

#include <string>
#include <vector>

/// This is to generalize LCA table and possibly change it in the future.
using LCA_KEY = std::string;

/// Common RED colos for colorful printing.
const std::string RED = "\033[1;31m";
/// Common BLUE colos for colorful printing.
const std::string BLUE = "\033[1;34m";
/// Common GREEN colos for colorful printing.
const std::string GREEN = "\033[1;32m";
/// Common YELLOW colos for colorful printing.
const std::string YELLOW = "\033[1;33m";
/// Common CYAN colos for colorful printing.
const std::string CYAN = "\033[1;36m";
/// Common RESET colos for colorful printing.
const std::string RESET = "\033[0m";

/// Get the lca key for two leafs sort them and put in string "first"#"second".
/// @param first First leaf.
/// @param second Second leaf.
/// @return LCA key for the hash table.
LCA_KEY get_lca_key(int first, int second);

/// Get the lca key for three leafs sort them and put in string
/// "first"#"second"#"three.
/// @param first First leaf.
/// @param second Second leaf.
/// @param third Third leaf.
/// @return LCA key for the hash table.
LCA_KEY get_lca_key(int first, int second, int third);

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
#endif
