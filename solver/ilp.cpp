#include "ilp.h"
#include "lp_data/HighsStatus.h"
#include "tree.h"
#include "utils.h"
#include <set>
#include <vector>

/// ILP

ILP::ILP(Input &input)
    : input_(input), update_counter_(4),
      limit_(input.get_reduced_leaf_count()) {
  highs_.setOptionValue("output_flag", false);
  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);
}

void ILP::initialize() {
  if (input_.get_reduced_leaf_count() <= 25) {
    update_counter_ = 25 * 25 * 25 * 25;
  }
  input_.compute_trios_quartets(trios_, quartets_, update_counter_,
                                components_);

  auto number_of_edges = input_.get_node_count();
  auto number_of_rows = trios_.size() + quartets_.size() + 1;
  auto tree = input_.get_trees().at(0).get();

  HighsModel model;

  model.lp_.num_col_ = number_of_edges;
  model.lp_.col_cost_ = std::vector<double>(number_of_edges, 1.0);
  model.lp_.col_lower_ = std::vector<double>(number_of_edges, 0.0);
  model.lp_.col_upper_ = std::vector<double>(number_of_edges, 1.0);

  model.lp_.num_row_ = number_of_rows;
  auto low = std::vector<double>(number_of_rows - 1, 1.0);
  low.push_back(0);
  model.lp_.row_lower_ = low;
  auto high = std::vector<double>(number_of_rows - 1, 1.0e30);
  high.push_back(input_.get_reduced_leaf_count() - 2);
  model.lp_.row_upper_ = high;

  model.lp_.sense_ = ObjSense::kMinimize;
  model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;

  std::vector<int> start = {0};
  std::vector<int> index;

  for (auto &&trio : trios_) {
    auto edges = tree->get_trio_edges(trio);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  for (auto &&quartet : quartets_) {
    auto edges = tree->get_quartet_edges(quartet);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  quartets_ = {};
  trios_ = {};

  for (int a = 0; a < number_of_edges; ++a) {
    index.push_back(a);
  }
  start.push_back(index.size());

  std::vector<double> values(index.size(), 1.0);
  model.lp_.a_matrix_.start_ = start;
  model.lp_.a_matrix_.index_ = index;
  model.lp_.a_matrix_.value_ = values;

  HighsStatus return_status = highs_.passModel(model);
  assert(return_status == HighsStatus::kOk);

  const HighsLp &lp = highs_.getLp();

  model.lp_.integrality_.resize(lp.num_col_);
  for (int col = 0; col < lp.num_col_; col++)
    model.lp_.integrality_[col] = HighsVarType::kInteger;

  highs_.passModel(model);
  highs_.writeModel("my_model.lp");
}

std::set<int> ILP::run() {
  std::cout << "# Solving ILP with " << YELLOW << highs_.getNumRow() << RESET
            << " constraints and " << YELLOW << highs_.getNumCol() << RESET
            << " variables." << std::endl;

  // Solve the model
  HighsStatus return_status = highs_.run();
  assert(return_status == HighsStatus::kOk);

  // Get the model status
  const HighsModelStatus &model_status = highs_.getModelStatus();
  assert(model_status == HighsModelStatus::kOptimal);

  const HighsInfo &info = highs_.getInfo();
  std::cout << "# Objective function value: " << YELLOW
            << info.objective_function_value << RESET << std::endl;

  const HighsSolution &solution = highs_.getSolution();
  const HighsBasis &basis = highs_.getBasis();

  const HighsLp &lp = highs_.getLp();

  std::set<int> edges_to_erase;
  for (int col = 0; col < lp.num_col_; col++) {
    if (is_approx_one(solution.col_value[col])) {
      edges_to_erase.insert(col + 1);
    }
  }
  std::vector<int> indices;
  for (int a = 1; a < input_.get_node_count() + 1; ++a) {
    indices.push_back(a);
  }
  auto upper_bound = input_.get_reduced_leaf_count() - 2;
  std::vector<double> values = std::vector<double>(indices.size(), 1);
  HighsStatus status =
      highs_.addRow(edges_to_erase.size(), upper_bound, indices.size(),
                    indices.data(), values.data());
  return edges_to_erase;
}

void ILP::set_components(const std::vector<std::unique_ptr<Tree>> &output) {
  auto tree_count = 0;
  for (auto &&tree : output) {
    if (!tree->is_empty()) {
      ++tree_count;
      auto leafs = tree->get_leafs();
      for (auto &&taxa : leafs) {
        components_[taxa] = tree_count;
      }
    }
  }
}

bool ILP::update() {
  auto trio_counter = trios_.size();
  auto quartet_counter = quartets_.size();

  update_counter_ = input_.get_reduced_leaf_count() < 2 * update_counter_
                        ? input_.get_reduced_leaf_count()
                        : 2 * update_counter_;

  input_.compute_trios_quartets(trios_, quartets_, update_counter_,
                                components_);
  if (trio_counter == trios_.size() && quartet_counter == quartets_.size()) {
    return false;
  }

  auto tree = input_.get_trees().at(0).get();

  for (int i = trio_counter; i < trios_.size(); ++i) {
    auto edges = tree->get_trio_edges(trios_[i]);
    std::vector<int> indices;
    indices.insert(indices.end(), edges.begin(), edges.end());
    auto values = std::vector<double>(indices.size(), 1);
    HighsStatus status =
        highs_.addRow(1, 1.0e30, indices.size(), indices.data(), values.data());
    assert(status == HighsStatus::kOk);
  }

  for (int i = quartet_counter; i < quartets_.size(); ++i) {
    auto edges = tree->get_quartet_edges(quartets_[i]);
    std::vector<int> indices;
    indices.insert(indices.end(), edges.begin(), edges.end());
    auto values = std::vector<double>(indices.size(), 1);
    HighsStatus status =
        highs_.addRow(1, 1.0e30, indices.size(), indices.data(), values.data());
    assert(status == HighsStatus::kOk);
  }

  quartets_ = {};
  trios_ = {};

  return true;
}
