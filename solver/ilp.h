/// @file ilp.h
/// @brief Creating and running ILP.
/// Definition and computation of the ILP. Also solving the ILP itself.
/// Used solver: highs.dev
#ifndef ilp_h_
#define ilp_h_

#include "Highs.h"
#include "tree.h"
#include <tuple>
#include <vector>

class ILP {
public:
  ILP(Input &input);
  std::set<int> run(Input &input);
  bool update(Input &input, std::vector<std::unique_ptr<Tree>> &output);

private:
  const int limit_ = 500;
  Highs highs_;
  std::vector<std::tuple<int, int, int>> trios_;
  std::vector<std::tuple<int, int, int, int>> quartets_;
  std::vector<int> components_;
};
#endif
