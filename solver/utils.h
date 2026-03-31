#ifndef utils_h_
#define utils_h_

#include <string>
#include <vector>

using LCA_KEY = std::string;

const std::string RED = "\033[31m";
const std::string BLUE = "\033[34m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string RESET = "\033[0m";

/// Split strings by space.
/// @param str Input string.
/// @return Vector of all substrings.
std::vector<std::string> split(const std::string &str);

/// Split strings by delimiter.
/// @param str Input string.
/// @param delimiter Given delimiter.
/// @return Vector of all substrings.
std::vector<std::string> split(const std::string &str, char delimiter);

LCA_KEY get_lca_key(int first, int second);

LCA_KEY get_lca_key(int first, int second, int third);

bool is_approx_one(float x, float tolerance = 1e-6f);

#endif
