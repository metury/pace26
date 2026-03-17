#include "ilp.h"
#include "Highs.h"
#include <set>
#include <vector>

void ilp(Input &input) {

  input.assign_numbers();
  input.compute_all_lca();
  auto trios = input.compute_trios();
  auto quartets = input.compute_quartets();

  std::cout << "# Computed trios and quartets" << std::endl;

  //! Pseudo random number.
  auto number_of_edges = input.get_leaf_count() * 2;
  auto number_of_rows = trios.size() + quartets.size();
  auto [tree, lca] = *input.begin();

  if (number_of_rows == 0) {
    // It is satisfied.
    return;
  }

  HighsModel model;
  model.lp_.num_col_ = number_of_edges;
  model.lp_.num_row_ = number_of_rows;
  model.lp_.sense_ = ObjSense::kMinimize;
  model.lp_.col_cost_ = std::vector<double>(number_of_edges, 1.0);
  model.lp_.col_lower_ = std::vector<double>(number_of_edges, 0.0);
  model.lp_.col_upper_ = std::vector<double>(number_of_edges, 1.0);
  model.lp_.row_lower_ = std::vector<double>(number_of_rows, 1.0);
  model.lp_.row_upper_ = std::vector<double>(number_of_rows, 1.0e30);

  model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;

  std::vector<int> start = {0};
  std::vector<int> index;

  for (auto &&[a, b, c] : trios) {
    std::set<int> edges;
    auto [lca_a_b, node_a_b] = lca.query(a, b);
    auto [lca_ab_c, node_ab_c] = lca.query(a, b, c);
    auto node_a = tree->get_descendants().at(a);
    auto node_b = tree->get_descendants().at(b);
    auto node_c = tree->get_descendants().at(c);
    node_a->get_edges(*node_a_b, edges);
    node_b->get_edges(*node_a_b, edges);
    node_c->get_edges(*node_ab_c, edges);
    node_a_b->get_edges(*node_ab_c, edges);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  for (auto &&[a, b, c, d] : quartets) {
    std::set<int> edges;
    auto [lca_a_b, node_a_b] = lca.query(a, b);
    auto [lca_c_d, node_c_d] = lca.query(c, d);
    auto node_a = tree->get_descendants().at(a);
    auto node_b = tree->get_descendants().at(b);
    auto node_c = tree->get_descendants().at(c);
    auto node_d = tree->get_descendants().at(d);
    node_a->get_edges(*node_a_b, edges);
    node_b->get_edges(*node_a_b, edges);
    node_c->get_edges(*node_c_d, edges);
    node_d->get_edges(*node_c_d, edges);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  std::vector<double> values(index.size(), 1.0);

  // a_start_ has num_col_+1 entries, and the last entry is the number
  // of nonzeros in A, allowing the number of nonzeros in the last
  // column to be defined
  model.lp_.a_matrix_.start_ = start;
  model.lp_.a_matrix_.index_ = index;
  model.lp_.a_matrix_.value_ = values;

  // Create a Highs instance
  Highs highs;
  HighsStatus return_status;

  highs.setOptionValue("output_flag", false);

  // Pass the model to HiGHS
  return_status = highs.passModel(model);
  assert(return_status == HighsStatus::kOk);

  // Get a const reference to the LP data in HiGHS
  const HighsLp &lp = highs.getLp();

  model.lp_.integrality_.resize(lp.num_col_);
  for (int col = 0; col < lp.num_col_; col++)
    model.lp_.integrality_[col] = HighsVarType::kInteger;

  highs.passModel(model);

  // highs.writeModel("model.lp");
  // return;

  // Solve the model
  return_status = highs.run();
  assert(return_status == HighsStatus::kOk);

  // Get the model status
  const HighsModelStatus &model_status = highs.getModelStatus();
  assert(model_status == HighsModelStatus::kOptimal);

  const HighsInfo &info = highs.getInfo();
  // std::cout << "Simplex iteration count: " << info.simplex_iteration_count
  //           << std::endl;
  std::cout << "Objective function value: " << info.objective_function_value
            << std::endl;
  // std::cout << "Primal  solution status: "
  //           << highs.solutionStatusToString(info.primal_solution_status)
  //           << std::endl;
  // std::cout << "Dual    solution status: "
  //           << highs.solutionStatusToString(info.dual_solution_status)
  //           << std::endl;
  // std::cout << "Basis: " << highs.basisValidityToString(info.basis_validity)
  //           << std::endl;

  const HighsSolution &solution = highs.getSolution();
  const HighsBasis &basis = highs.getBasis();
  //
  // Report the primal and solution values and basis
  for (int col = 0; col < lp.num_col_; col++) {
    std::cout << "Column " << col;
    std::cout << "; value = " << solution.col_value[col];
    std::cout << std::endl;
  }
}
