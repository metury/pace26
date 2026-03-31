#include "utils.h"
#include <sstream>

LCA_KEY get_lca_key(int first, int second) {
  std::ostringstream os;
  if (first > second) {
    std::swap(first, second);
  }
  os << first << "#" << second;
  return os.str();
}

LCA_KEY get_lca_key(int first, int second, int third) {
  std::ostringstream os;
  if (second < first && second < third) {
    std::swap(first, second);
  } else if (third < first && third < second) {
    std::swap(first, third);
  }
  if (second > third) {
    std::swap(second, third);
  }
  os << first << "#" << second << "#" << third;
  return os.str();
}

bool is_approx_one(float x, float tolerance) {
  return std::abs(x - 1.0f) < tolerance;
}

std::vector<std::string> split(const std::string &str) {
  std::vector<std::string> tokens;
  std::istringstream iss(str);
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

std::vector<std::string> split(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = str.find(delimiter);
  while (end != std::string::npos) {
    tokens.push_back(str.substr(start, end - start));
    start = end + 1;
    end = str.find(delimiter, start);
  }
  tokens.push_back(str.substr(start));
  return tokens;
}
