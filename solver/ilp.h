/// @file ilp.h
/// @brief Creating and running ILP.
/// Definition and computation of the ILP. Also solving the ILP itself.
/// Used solver: highs.dev
#ifndef ilp_h_
#define ilp_h_

#include "Highs.h"
#include "tree.h"

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <set>
#include <vector>

class SCIILP {
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

public:
  SCIILP(Input &input);
  ~SCIILP(); // Important: SCIP requires manual memory cleanup
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

/// Class containing ILP model that is solving MAF.
class ILP {
public:
  /// Create empty ILP.
  /// @param input Which input to use.
  ILP(Input &input);
  /// initialize the ilp with first constraints.
  void initialize();
  /// run the current ilp.
  /// @return set of edges that should be deleted.
  std::set<int> run();
  /// update components based on the last result.
  /// @param output what is the forest created by the last solution.
  void set_components(const std::vector<std::unique_ptr<Tree>> &output);
  /// update ilp based on unsatisfied constraints.
  /// @return true if ilp has new rows.
  bool update();

private:
  /// Which model it is using.
  Highs highs_;
  /// What input we are solving.
  Input &input_;
  /// Setting limit for constraints.
  int limit_;
  /// Components of connected components.
  std::vector<int> components_;
};

#endif
