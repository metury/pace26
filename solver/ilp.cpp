#include "ilp.h"
#include "lp_data/HighsStatus.h"
#include "tree.h"
#include "utils.h"
#include <set>
#include <vector>

/// ILP

ILP::ILP(Input &input) : input_(input), limit_(4) {
  highs_.setOptionValue("output_flag", false);
  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);
}

void ILP::initialize() {
  if (input_.get_reduced_leaf_count() <= 25) {
    limit_ = 25 * 25 * 25 * 25;
  }
  auto trios = std::vector<std::tuple<int, int, int>>();
  auto quartets = std::vector<std::tuple<int, int, int, int>>();
  input_.compute_trios_quartets(trios, quartets, limit_, components_);

  auto number_of_edges = input_.get_node_count();
  auto number_of_rows = trios.size() + quartets.size() + 1;
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

  for (auto &&trio : trios) {
    auto edges = tree->get_trio_edges(trio);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

  for (auto &&quartet : quartets) {
    auto edges = tree->get_quartet_edges(quartet);
    index.insert(index.end(), edges.begin(), edges.end());
    start.push_back(index.size());
  }

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
  limit_ = input_.get_reduced_leaf_count() < 2 * limit_
               ? input_.get_reduced_leaf_count()
               : 2 * limit_;

  auto trios = std::vector<std::tuple<int, int, int>>();
  auto quartets = std::vector<std::tuple<int, int, int, int>>();
  input_.compute_trios_quartets(trios, quartets, limit_, components_);
  if (trios.empty() && quartets.empty()) {
    return false;
  }

  auto tree = input_.get_trees().at(0).get();

  for (auto &&trio : trios) {
    auto edges = tree->get_trio_edges(trio);
    std::vector<int> indices;
    indices.insert(indices.end(), edges.begin(), edges.end());
    auto values = std::vector<double>(indices.size(), 1);
    HighsStatus status =
        highs_.addRow(1, 1.0e30, indices.size(), indices.data(), values.data());
    assert(status == HighsStatus::kOk);
  }

  for (auto &&quartet : quartets) {
    auto edges = tree->get_quartet_edges(quartet);
    std::vector<int> indices;
    indices.insert(indices.end(), edges.begin(), edges.end());
    auto values = std::vector<double>(indices.size(), 1);
    HighsStatus status =
        highs_.addRow(1, 1.0e30, indices.size(), indices.data(), values.data());
    assert(status == HighsStatus::kOk);
  }

  return true;
}

/// SCIILP

SCIILP::SCIILP(Input &input) : input_(input), limit_(4) {
  SCIPcreate(&scip_);
  SCIPincludeDefaultPlugins(scip_);
  SCIPcreateProbBasic(scip_, "PACE2026 - MAF");
  SCIPsetIntParam(scip_, "display/verblevel", 0);
  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);
}

SCIILP::~SCIILP() {
  if (scip_ != nullptr) {
    for (auto var : vars_) {
      SCIPreleaseVar(scip_, &var);
    }
    SCIPfree(&scip_);
  }
}

void SCIILP::initialize() {
  auto trios = std::vector<std::tuple<int, int, int>>();
  auto quartets = std::vector<std::tuple<int, int, int, int>>();
  input_.compute_trios_quartets(trios, quartets, limit_, components_);
  auto number_of_edges = input_.get_node_count();
  auto number_of_rows = trios.size() + quartets.size() + 1;
  auto tree = input_.get_trees().at(0).get();

  SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE);

  vars_.resize(number_of_edges);
  for (int col = 0; col < number_of_edges; ++col) {
    std::string name = "e_" + std::to_string(col + 1);
    // SCIPcreateVarBasic(scip, var, name, lower_bound, upper_bound, obj_cost,
    // vartype)
    SCIPcreateVarBasic(scip_, &vars_[col], name.c_str(), 0.0, 1.0, 1.0,
                       SCIP_VARTYPE_BINARY);
    SCIPaddVar(scip_, vars_[col]);
  }

  for (auto &&trio : trios) {
    auto edges = tree->get_trio_edges(trio);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "trio_cons", 0, nullptr, nullptr,
                              1.0, SCIPinfinity(scip_));
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons); // Release memory once added to SCIP
  }

  for (auto &&quartet : quartets) {
    auto edges = tree->get_quartet_edges(quartet);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "quartet_cons", 0, nullptr, nullptr,
                              1.0, SCIPinfinity(scip_));
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons);
  }

  SCIP_CONS *cons_all;
  double upper_bound = input_.get_reduced_leaf_count() - 2;
  SCIPcreateConsBasicLinear(scip_, &cons_all, "all_edges", 0, nullptr, nullptr,
                            0.0, upper_bound);
  for (int a = 0; a < vars_.size(); ++a) {
    SCIPaddCoefLinear(scip_, cons_all, vars_[a], 1.0);
  }
  SCIPaddCons(scip_, cons_all);
  SCIPreleaseCons(scip_, &cons_all);
}

void SCIILP::set_components(const std::vector<std::unique_ptr<Tree>> &output) {
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

std::set<int> SCIILP::run() {
  std::cout << "# Solving ILP with " << YELLOW << SCIPgetNConss(scip_) << RESET
            << " constraints and " << YELLOW << SCIPgetNVars(scip_) << RESET
            << " variables." << std::endl;
  SCIP_RETCODE ret = SCIPsolve(scip_);
  assert(ret == SCIP_OKAY);

  SCIP_STATUS status = SCIPgetStatus(scip_);
  assert(status == SCIP_STATUS_OPTIMAL);

  SCIP_SOL *solution = SCIPgetBestSol(scip_);

  double obj_val = SCIPgetSolOrigObj(scip_, solution);
  std::cout << "# Objective function value: " << YELLOW << obj_val << RESET
            << std::endl;

  std::set<int> edges_to_erase;
  for (int col = 0; col < vars_.size(); col++) {
    auto val = SCIPgetSolVal(scip_, solution, vars_[col]);
    if (is_approx_one(val)) {
      edges_to_erase.insert(col + 1);
    }
  }

  SCIPfreeTransform(scip_);

  SCIP_CONS *new_cons;
  SCIPcreateConsBasicLinear(scip_, &new_cons, "post_run_cons", 0, nullptr,
                            nullptr, obj_val,
                            input_.get_reduced_leaf_count() - 2);

  for (int a = 0; a < vars_.size(); ++a) {
    SCIPaddCoefLinear(scip_, new_cons, vars_[a], 1.0);
  }

  SCIPaddCons(scip_, new_cons);
  SCIPreleaseCons(scip_, &new_cons);

  return edges_to_erase;
}

bool SCIILP::update() {
  auto trios = std::vector<std::tuple<int, int, int>>();
  auto quartets = std::vector<std::tuple<int, int, int, int>>();

  limit_ = limit_ > input_.get_reduced_leaf_count()
               ? input_.get_reduced_leaf_count()
               : 2 * limit_;
  input_.compute_trios_quartets(trios, quartets, limit_, components_);
  if (trios.empty() && quartets.empty()) {
    return false;
  }

  auto tree = input_.get_trees().at(0).get();

  SCIPfreeTransform(scip_);

  for (auto &&trio : trios) {
    auto edges = tree->get_trio_edges(trio);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "update_trio", 0, nullptr, nullptr,
                              1.0, SCIPinfinity(scip_));
    for (int idx : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[idx], 1.0);
    }
    SCIP_RETCODE status = SCIPaddCons(scip_, cons);
    assert(status == SCIP_OKAY);
    SCIPreleaseCons(scip_, &cons);
  }

  for (auto &&quartet : quartets) {
    auto edges = tree->get_quartet_edges(quartet);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "update_quartet", 0, nullptr,
                              nullptr, 1.0, SCIPinfinity(scip_));
    for (int idx : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[idx], 1.0);
    }
    SCIP_RETCODE status = SCIPaddCons(scip_, cons);
    assert(status == SCIP_OKAY);
    SCIPreleaseCons(scip_, &cons);
  }

  return true;
}
