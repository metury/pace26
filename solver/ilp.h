/// @file ilp.h
/// @brief Creating and running ILP.
/// Definition and computation of the ILP. Also solving the ILP itself.
/// Used solver: highs.dev
#ifndef ilp_h_
#define ilp_h_

#include "tree.h"

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <set>
#include <vector>

class ILP {
private:
  SCIP *scip_ = nullptr;
  std::vector<SCIP_VAR *> vars_;
  /// What input we are solving.
  Input &input_;
  /// Setting limit for constraints.
  int limit_;
  /// Components of connected components.
  std::vector<int> components_;
  int upper_limit_;
  std::vector<Trio> trios_;
  std::vector<Quartet> quartets_;

public:
  ILP(Input &input);
  ~ILP(); // Important: SCIP requires manual memory cleanup
  /// initialize the ilp with first constraints.
  void initialize();
  /// run the current ilp.
  /// @return set of edges that should be deleted.
  std::set<int> run();
  /// update components based on the last result.
  /// @param output what is the forest created by the last solution.
  void set_components(const std::vector<std::unique_ptr<Tree>> &output);
  void set_priorities() const;
  /// update ilp based on unsatisfied constraints.
  /// @return true if ilp has new rows.
  bool update();
};

#endif
