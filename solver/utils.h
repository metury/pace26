#ifndef utils_h_
#define utils_h_

#include <string>
#include <vector>

using LCA_KEY = std::string;

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

#endif
