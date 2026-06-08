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

/// Class containing ILP model that is solving MAF.
class ILP {
public:
  /// Initialize ILP.
  /// @param input
  ILP(Input &input);
  /// Run the current ILP.
  /// @param input
  /// @return
  std::set<int> run(Input &input);
  /// Update ILP based on unsatisfied constraints.
  /// @param
  /// @param
  /// @return
  bool update(Input &input, std::vector<std::unique_ptr<Tree>> &output);

private:
  ///
  const int limit_ = 500;
  ///
  Highs highs_;
  ///
  std::vector<std::tuple<int, int, int>> trios_;
  ///
  std::vector<std::tuple<int, int, int, int>> quartets_;
  ///
  std::vector<int> components_;
};
#endif
