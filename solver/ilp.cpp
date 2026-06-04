#include "ilp.h"
#include "Highs.h"
#include "utils.h"
#include <set>
#include <unordered_map>
#include <vector>

int lp(Input &input) {
  auto edge_values = ilp_general(input, false);
  auto sum = 0.0;
  for (auto &&[key, val] : edge_values) {
    sum += val;
    std::cout << key << " has value: " << val << std::endl;
  }
  return std::ceil(sum);
}

std::set<int> ilp(Input &input) {
  auto results = ilp_general(input, true);
  std::set<int> edges_to_erase;
  for (auto &&[key, value] : results) {
    if (is_approx_one(value)) {
      edges_to_erase.insert(key);
    }
  }
  return edges_to_erase;
}

std::unordered_map<int, float> ilp_general(Input &input, bool integer) {
  auto trios = std::vector<std::tuple<int, int, int>>();
  auto quartets = std::vector<std::tuple<int, int, int, int>>();
  input.compute_trios_quartets(trios, quartets);

  //! Pseudo random number.
  auto number_of_edges = input.get_node_count();
  auto number_of_rows = trios.size() + quartets.size();
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
  model.lp_.row_lower_ = std::vector<double>(number_of_rows, 1.0);
  model.lp_.row_upper_ =
      std::vector<double>(number_of_rows, input.get_leaf_count() - 1);

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

  // At most number of leafs for all edges.
  for (int a = 0; a < number_of_edges; ++a) {
    index.push_back(a);
  }
  start.push_back(index.size());

  std::vector<double> values(index.size(), 1.0);

  input.delete_lca_tables();

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

std::unordered_map<int, float> ilp_general_alt(Input &input, bool integer) {
  auto trios = input.compute_trios();
  auto quartets = input.compute_quartets();

  auto parents = input.compute_parents();
  auto parents_size = 0;
  for (auto &&[key, val] : parents) {
    parents_size += val.size();
  }

  auto number_of_vertices = input.get_node_count();
  auto number_of_edges = input.get_node_count();
  auto number_of_variables =
      number_of_edges + number_of_vertices * number_of_vertices;
  auto number_of_rows = trios.size() + quartets.size() + parents_size * 3 +
                        input.get_node_count();
  auto tree = input.get_trees().at(0).get();

  if (trios.empty() && quartets.empty()) {
    // It is satisfied.
    std::cout << "# Objective function value: " << YELLOW << 0 << RESET
              << std::endl;
    return std::unordered_map<int, float>{};
  }

  HighsModel model;
  model.lp_.num_col_ = number_of_variables;
  model.lp_.num_row_ = number_of_rows;
  model.lp_.sense_ = ObjSense::kMinimize;

  auto cost = std::vector<double>(number_of_edges, 1.0);
  cost.insert(cost.end(), number_of_vertices * number_of_vertices, 0.0);
  model.lp_.col_cost_ = cost;
  model.lp_.col_lower_ = std::vector<double>(number_of_variables, 0.0);
  model.lp_.col_upper_ = std::vector<double>(number_of_variables, 1.0);

  auto lower = std::vector<double>(trios.size() + quartets.size(), -1.0e30);
  auto upper = std::vector<double>(trios.size() + quartets.size(), 3.0);
  lower.insert(lower.end(), number_of_vertices, 1.0);
  upper.insert(upper.end(), number_of_vertices, 1.0);
  std::vector<double> pattern_upper = {0.0, 1.0e30, 1};
  std::vector<double> pattern_lower = {-1.0e30, 0.0, -1.0e30};
  for (auto i = 0; i < parents_size; ++i) {
    lower.insert(lower.end(), pattern_lower.begin(), pattern_lower.end());
    upper.insert(upper.end(), pattern_upper.begin(), pattern_upper.end());
  }
  model.lp_.row_lower_ = lower;
  model.lp_.row_upper_ = upper;

  model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;

  std::vector<int> start = {0};
  std::vector<int> index;

  for (auto &&[a, b, c] : trios) {
    std::vector<int> edges;
    auto a_b = tree->lca_query(a, b)->get_value();
    auto ab_c = tree->lca_query(a, b, c)->get_value();
    edges = {get_matrix_index(number_of_edges, number_of_vertices, a, a_b),
             get_matrix_index(number_of_edges, number_of_vertices, b, a_b),
             get_matrix_index(number_of_edges, number_of_vertices, c, ab_c),
             get_matrix_index(number_of_edges, number_of_vertices, a_b, ab_c)};
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  for (auto &&[a, b, c, d] : quartets) {
    std::vector<int> edges;
    auto a_b = tree->lca_query(a, b)->get_value();
    auto c_d = tree->lca_query(c, d)->get_value();
    edges = {get_matrix_index(number_of_edges, number_of_vertices, a, a_b),
             get_matrix_index(number_of_edges, number_of_vertices, b, a_b),
             get_matrix_index(number_of_edges, number_of_vertices, c, c_d),
             get_matrix_index(number_of_edges, number_of_vertices, d, c_d)};
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  for (int i = 1; i <= input.get_node_count(); ++i) {
    index.push_back(
        get_matrix_index(number_of_edges, number_of_vertices, i, i));
    start.push_back(index.size());
  }

  std::vector<double> values(index.size(), 1.0);

  for (auto &&[key, val] : parents) {
    for (auto &&par : val) {
      index.push_back(
          get_matrix_index(number_of_edges, number_of_vertices, key, par));
      index.push_back(get_matrix_index(number_of_edges, number_of_vertices,
                                       *(--val.end()), par));
      values.push_back(1.0);
      values.push_back(-1.0);
      start.push_back(index.size());

      index.push_back(
          get_matrix_index(number_of_edges, number_of_vertices, key, par));
      index.push_back(get_matrix_index(number_of_edges, number_of_vertices,
                                       *(--val.end()), par));
      index.push_back(key - 1);
      values.push_back(1.0);
      values.push_back(-1.0);
      values.push_back(1.0);
      start.push_back(index.size());

      index.push_back(
          get_matrix_index(number_of_edges, number_of_vertices, key, par));
      index.push_back(key - 1);
      values.push_back(1.0);
      values.push_back(1.0);
      start.push_back(index.size());
    }
  }

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
            << " constraints and " << YELLOW << number_of_variables << RESET
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
  for (int col = 0; col < number_of_edges; col++) {
    edge_values.insert_or_assign(col + 1, solution.col_value[col]);
  }
  return edge_values;
}
