/// @file ilp.h
/// @brief Creating and running ILP.
/// Definition and computation of the ILP. Also solving the ILP itself.
/// Used solver: https://www.scipopt.org/
#ifndef ilp_h_
#define ilp_h_

#include "tree.h"
#include "utils.h"
#include <cstdint>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

/// Class to communicate with SCIP ilp solver.
class ILP {
public:
  /// Default constructor based on the input.
  /// @param input Which input to use.
  ILP(Input &input);
  /// Destructor for the SCIP environment.
  ~ILP();
  /// Drop the current ILP.
  void drop_ilp();
  /// Start the ILP with previously used constraints and found solution.
  /// @param edges_to_erase Feasible solution.
  /// @param repair Whether to repair possibly bad edges to the fork
  /// constraints.
  void warm_start(std::set<uint16_t> &edges_to_erase, bool repair);
  /// Add trio constraint to the ILP.
  /// @param t Which trio to use.
  void add_trio_constr(Trio &t);
  /// Add quartet constraint to the ILP.
  /// @param q Which quartet to use.
  void add_quartet_constr(Quartet &q);
  /// Initialize the ilp with first constraints.
  /// @param lb Lower bound.
  /// @param ub Upper bound.
  /// @param h Whether to go in "heuristic" mode.
  void initialize(uint16_t lb, uint16_t ub, bool h);
  /// Run the current ilp.
  /// @return Set of edges that should be deleted.
  std::set<uint16_t> run();
  /// Update components based on the last result.
  /// @param output What is the forest created by the last solution.
  void set_components(const std::vector<std::unique_ptr<Tree>> &output);
  /// Update ilp based on unsatisfied constraints.
  /// @return True if ilp has new constraints.
  bool update();

private:
  /// ILP formulation.
  SCIP *scip_ = nullptr;
  /// Which variables are in use.
  std::vector<SCIP_VAR *> vars_;
  /// What input we are solving.
  Input &input_;
  /// Components of connected components.
  std::vector<uint16_t> components_;
  /// Upper bound for every constraint.
  int upper_bound_;
  /// List of all incomaptible trios.
  std::vector<Trio> trios_;
  /// List of all incomaptible quartets.
  std::vector<Quartet> quartets_;
  bool heuristics_;
};

#endif
