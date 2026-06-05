#include "ilp.h"
#include "Highs.h"
#include "utils.h"
#include <set>
#include <unordered_map>
#include <vector>

// int lp(Input &input) {
//   auto edge_values = ilp_general(input, false);
//   auto sum = 0.0;
//   for (auto &&[key, val] : edge_values) {
//     sum += val;
//     std::cout << key << " has value: " << val << std::endl;
//   }
//   return std::ceil(sum);
// }

std::set<int> ilp(Input &input, int limit,
                  const std::vector<std::tuple<int, int, int>> &trios,
                  const std::vector<std::tuple<int, int, int, int>> &quartets) {
  auto results = ilp_general(input, limit, trios, quartets, true);
  std::set<int> edges_to_erase;
  for (auto &&[key, value] : results) {
    if (is_approx_one(value)) {
      edges_to_erase.insert(key);
    }
  }
  return edges_to_erase;
}

std::unordered_map<int, float>
ilp_general(Input &input, int limit,
            const std::vector<std::tuple<int, int, int>> &trios,
            const std::vector<std::tuple<int, int, int, int>> &quartets,
            bool integer) {
  // auto trios = std::vector<std::tuple<int, int, int>>();
  // auto quartets = std::vector<std::tuple<int, int, int, int>>();
  // input.compute_trios_quartets(trios, quartets, limit, components);

  // auto nr_of_trios = LIMIT > trios.size() ? trios.size() : LIMIT;
  // auto nr_of_quartets = LIMIT > quartets.size() ? quartets.size() : LIMIT;
  auto nr_of_trios = trios.size();
  auto nr_of_quartets = quartets.size();

  //! Pseudo random number.
  auto number_of_edges = input.get_node_count();
  auto number_of_rows = nr_of_quartets + nr_of_trios;
  auto tree = input.get_trees().at(0).get();

  if (number_of_rows == 0) {
    // It is satisfied.
    std::cout << "# Objective function value: " << YELLOW << 0 << RESET
              << std::endl;
    return std::unordered_map<int, float>{};
  }

  // Add last row that says we have upper bound on the all edges.
  number_of_rows += 1;

  HighsModel model;
  model.lp_.num_col_ = number_of_edges;
  model.lp_.num_row_ = number_of_rows;
  model.lp_.sense_ = ObjSense::kMinimize;
  model.lp_.col_cost_ = std::vector<double>(number_of_edges, 1.0);
  model.lp_.col_lower_ = std::vector<double>(number_of_edges, 0.0);
  model.lp_.col_upper_ = std::vector<double>(number_of_edges, 1.0);
  auto low = std::vector<double>(number_of_rows - 1, 1.0);
  low.push_back(limit);
  model.lp_.row_lower_ = low;
  auto high = std::vector<double>(number_of_rows - 1, 1.0e30);
  high.push_back(input.get_leaf_count() - 2);
  model.lp_.row_upper_ = high;

  model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;

  std::vector<int> start = {0};
  std::vector<int> index;

  auto counter = 0;
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

  counter = 0;
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

  // At most number of leafs for all edges.
  for (int a = 0; a < number_of_edges; ++a) {
    index.push_back(a);
  }
  start.push_back(index.size());

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

  // IF IT IS INTEGER! ==========================================
  if (integer) {
    model.lp_.integrality_.resize(lp.num_col_);
    for (int col = 0; col < lp.num_col_; col++)
      model.lp_.integrality_[col] = HighsVarType::kInteger;
  }
  // IF IT IS INTEGER! ==========================================

  highs.passModel(model);

  // highs.writeModel("model.lp");
  // return std::unordered_map<int, float>{};

  std::cout << "# Solving ILP with " << YELLOW << number_of_rows << RESET
            << " constraints and " << YELLOW << number_of_edges << RESET
            << " variables." << std::endl;

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

  std::unordered_map<int, float> edge_values;
  for (int col = 0; col < lp.num_col_; col++) {
    edge_values.insert_or_assign(col + 1, solution.col_value[col]);
  }
  return edge_values;
}
