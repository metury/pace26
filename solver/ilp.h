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
  /// Create empty ILP.
  /// @param input Which input to use.
  ILP(Input &input);
  /// Initialize the ILP with first constraints.
  void initialize();
  /// Run the current ILP.
  /// @return Set of edges that should be deleted.
  std::set<int> run();
  /// Update components based on the last result.
  /// @param output What is the forest created by the last solution.
  void set_components(const std::vector<std::unique_ptr<Tree>> &output);
  /// Update ILP based on unsatisfied constraints.
  /// @return True if ILP has new rows.
  bool update();

private:
  /// Which model it is using.
  Highs highs_;
  /// What input we are solving.
  Input &input_;
  int limit_;
  /// All incompatible trios listed so far.
  std::vector<std::tuple<int, int, int>> trios_;
  /// All incompatible quartets listed so far.
  std::vector<std::tuple<int, int, int, int>> quartets_;
  /// Components of connected components.
  std::vector<int> components_;
  /// How many updates how been done so far.
  int update_counter_;
};

#endif
