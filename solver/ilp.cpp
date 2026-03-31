#include "ilp.h"
#include "Highs.h"
#include "utils.h"
#include <set>
#include <vector>

std::set<int> ilp(Input &input) {

  auto trios = input.compute_trios();
  auto quartets = input.compute_quartets();

  //! Pseudo random number.
  auto number_of_edges = input.get_leaf_count() * 2;
  auto number_of_rows = trios.size() + quartets.size();
  auto tree = input.get_trees().at(0).get();

  if (number_of_rows == 0) {
    // It is satisfied.
    std::cout << "# Objective function value: " << YELLOW << 0 << RESET
              << std::endl;
    return std::set<int>{};
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
    auto node_a_b = tree->lca_query(a, b);
    auto node_ab_c = tree->lca_query(a, b, c);
    auto node_a = tree->get_leaf(a);
    auto node_b = tree->get_leaf(b);
    auto node_c = tree->get_leaf(c);
    tree->get_edges(node_a, node_a_b, edges);
    tree->get_edges(node_b, node_a_b, edges);
    tree->get_edges(node_c, node_ab_c, edges);
    tree->get_edges(node_a_b, node_ab_c, edges);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  for (auto &&[a, b, c, d] : quartets) {
    std::set<int> edges;
    auto node_a_b = tree->lca_query(a, b);
    auto node_c_d = tree->lca_query(c, d);
    auto node_a = tree->get_leaf(a);
    auto node_b = tree->get_leaf(b);
    auto node_c = tree->get_leaf(c);
    auto node_d = tree->get_leaf(d);
    tree->get_edges(node_a, node_a_b, edges);
    tree->get_edges(node_b, node_a_b, edges);
    tree->get_edges(node_c, node_c_d, edges);
    tree->get_edges(node_d, node_c_d, edges);
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
  std::cout << "# Objective function value: " << YELLOW
            << info.objective_function_value << RESET << std::endl;

  const HighsSolution &solution = highs.getSolution();
  const HighsBasis &basis = highs.getBasis();
  std::set<int> edges_to_erase;
  for (int col = 0; col < lp.num_col_; col++) {
    if (is_approx_one(solution.col_value[col])) {
      edges_to_erase.insert(col);
    }
  }
  return edges_to_erase;
}
