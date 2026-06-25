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

/// Class to comunicate with SCIP ilp solver.
class ILP {
public:
  /// Default constructor based on the input.
  /// @param input Which input to use.
  ILP(Input &input);
  /// Destructor for the SCIP environment.
  ~ILP();
  /// Initialize the ilp with first constraints.
  void initialize();
  /// Run the current ilp.
  /// @return Set of edges that should be deleted.
  std::set<int> run();
  /// Update components based on the last result.
  /// @param output What is the forest created by the last solution.
  void set_components(const std::vector<std::unique_ptr<Tree>> &output);
  /// Change the priorities of all variables based on their appearance.
  void set_priorities() const;
  /// update ilp based on unsatisfied constraints.
  /// @return true if ilp has new rows.
  bool update();

private:
  /// ILP formulation.
  SCIP *scip_ = nullptr;
  /// Which variables are in use.
  std::vector<SCIP_VAR *> vars_;
  /// What input we are solving.
  Input &input_;
  /// Setting limit for constraints.
  int limit_;
  /// Components of connected components.
  std::vector<int> components_;
  /// Upper limit for every constraint.
  int upper_limit_;
  /// List of all incomaptible trios.
  std::vector<Trio> trios_;
  /// List of all incomaptible quartets.
  std::vector<Quartet> quartets_;
};

#endif
