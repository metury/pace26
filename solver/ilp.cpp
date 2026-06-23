#include "ilp.h"
#include "lp_data/HighsStatus.h"
#include "tree.h"
#include "utils.h"
#include <ranges>
#include <set>
#include <vector>

////// ILP
///
/// ILP::ILP(Input &input) : input_(input), limit_(4) {
///  highs_.setOptionValue("output_flag", false);
///  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);
///}
///
/// void ILP::initialize() {
///  if (input_.get_reduced_leaf_count() <= 25) {
///    limit_ = 25 * 25 * 25 * 25;
///  }
///  auto trios = std::vector<std::tuple<int, int, int>>();
///  auto quartets = std::vector<std::tuple<int, int, int, int>>();
///  input_.compute_trios_quartets(trios, quartets, limit_, components_);
///
///  auto number_of_edges = input_.get_node_count();
///  auto number_of_rows = trios.size() + quartets.size() + 1;
///  auto tree = input_.get_trees().at(0).get();
///
///  HighsModel model;
///
///  model.lp_.num_col_ = number_of_edges;
///  model.lp_.col_cost_ = std::vector<double>(number_of_edges, 1.0);
///  model.lp_.col_lower_ = std::vector<double>(number_of_edges, 0.0);
///  model.lp_.col_upper_ = std::vector<double>(number_of_edges, 1.0);
///
///  model.lp_.num_row_ = number_of_rows;
///  auto low = std::vector<double>(number_of_rows - 1, 1.0);
///  low.push_back(0);
///  model.lp_.row_lower_ = low;
///  auto high = std::vector<double>(number_of_rows - 1, 1.0e30);
///  high.push_back(input_.get_reduced_leaf_count() - 2);
///  model.lp_.row_upper_ = high;
///
///  model.lp_.sense_ = ObjSense::kMinimize;
///  model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;
///
///  std::vector<int> start = {0};
///  std::vector<int> index;
///
///  for (auto &&trio : trios) {
///    auto edges = tree->get_trio_edges(trio);
///    index.insert(index.end(), edges.begin(), edges.end());
///    start.push_back(index.size());
///  }
///
///  for (auto &&quartet : quartets) {
///    auto edges = tree->get_quartet_edges(quartet);
///    index.insert(index.end(), edges.begin(), edges.end());
///    start.push_back(index.size());
///  }
///
///  for (int a = 0; a < number_of_edges; ++a) {
///    index.push_back(a);
///  }
///  start.push_back(index.size());
///
///  std::vector<double> values(index.size(), 1.0);
///  model.lp_.a_matrix_.start_ = start;
///  model.lp_.a_matrix_.index_ = index;
///  model.lp_.a_matrix_.value_ = values;
///
///  HighsStatus return_status = highs_.passModel(model);
///  assert(return_status == HighsStatus::kOk);
///
///  const HighsLp &lp = highs_.getLp();
///
///  model.lp_.integrality_.resize(lp.num_col_);
///  for (int col = 0; col < lp.num_col_; col++)
///    model.lp_.integrality_[col] = HighsVarType::kInteger;
///
///  highs_.passModel(model);
///  highs_.writeModel("my_model.lp");
///}
///
/// std::set<int> ILP::run() {
///  std::cout << "# Solving ILP with " << YELLOW << highs_.getNumRow() << RESET
///            << " constraints and " << YELLOW << highs_.getNumCol() << RESET
///            << " variables." << std::endl;
///
///  // Solve the model
///  HighsStatus return_status = highs_.run();
///  assert(return_status == HighsStatus::kOk);
///
///  // Get the model status
///  const HighsModelStatus &model_status = highs_.getModelStatus();
///  assert(model_status == HighsModelStatus::kOptimal);
///
///  const HighsInfo &info = highs_.getInfo();
///  std::cout << "# Objective function value: " << YELLOW
///            << info.objective_function_value << RESET << std::endl;
///
///  const HighsSolution &solution = highs_.getSolution();
///  const HighsBasis &basis = highs_.getBasis();
///
///  const HighsLp &lp = highs_.getLp();
///
///  std::set<int> edges_to_erase;
///  for (int col = 0; col < lp.num_col_; col++) {
///    if (is_approx_one(solution.col_value[col])) {
///      edges_to_erase.insert(col + 1);
///    }
///  }
///  std::vector<int> indices;
///  for (int a = 1; a < input_.get_node_count() + 1; ++a) {
///    indices.push_back(a);
///  }
///  auto upper_bound = input_.get_reduced_leaf_count() - 2;
///  std::vector<double> values = std::vector<double>(indices.size(), 1);
///  HighsStatus status =
///      highs_.addRow(edges_to_erase.size(), upper_bound, indices.size(),
///                    indices.data(), values.data());
///  return edges_to_erase;
///}
///
/// void ILP::set_components(const std::vector<std::unique_ptr<Tree>> &output) {
///  auto tree_count = 0;
///  for (auto &&tree : output) {
///    if (!tree->is_empty()) {
///      ++tree_count;
///      auto leafs = tree->get_leafs();
///      for (auto &&taxa : leafs) {
///        components_[taxa] = tree_count;
///      }
///    }
///  }
///}
///
/// bool ILP::update() {
///  limit_ = input_.get_reduced_leaf_count() < 2 * limit_
///               ? input_.get_reduced_leaf_count()
///               : 2 * limit_;
///
///  auto trios = std::vector<std::tuple<int, int, int>>();
///  auto quartets = std::vector<std::tuple<int, int, int, int>>();
///  input_.compute_trios_quartets(trios, quartets, limit_, components_);
///  if (trios.empty() && quartets.empty()) {
///    return false;
///  }
///
///  auto tree = input_.get_trees().at(0).get();
///
///  for (auto &&trio : trios) {
///    auto edges = tree->get_trio_edges(trio);
///    std::vector<int> indices;
///    indices.insert(indices.end(), edges.begin(), edges.end());
///    auto values = std::vector<double>(indices.size(), 1);
///    HighsStatus status =
///        highs_.addRow(1, 1.0e30, indices.size(), indices.data(),
///        values.data());
///    assert(status == HighsStatus::kOk);
///  }
///
///  for (auto &&quartet : quartets) {
///    auto edges = tree->get_quartet_edges(quartet);
///    std::vector<int> indices;
///    indices.insert(indices.end(), edges.begin(), edges.end());
///    auto values = std::vector<double>(indices.size(), 1);
///    HighsStatus status =
///        highs_.addRow(1, 1.0e30, indices.size(), indices.data(),
///        values.data());
///    assert(status == HighsStatus::kOk);
///  }
///
///  return true;
///}

/// SCIILP

SCIILP::SCIILP(Input &input)
    : input_(input), limit_(std::log2(input.get_reduced_leaf_count())),
      upper_limit_(input_.get_reduced_leaf_count() - 2) {
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

void SCIILP::initialize(bool all_constraints) {
  auto trios = std::vector<std::tuple<int, int, int>>();
  auto quartets = std::vector<std::tuple<int, int, int, int>>();
  input_.compute_trios_quartets(trios, quartets, limit_, components_,
                                all_constraints);
  auto forks = std::vector<std::tuple<int, int, int>>();
  auto extended_forks = std::vector<std::array<int, 7>>();
  input_.compute_breakable_forks(forks, extended_forks);
  auto number_of_edges = input_.get_node_count();
  auto number_of_rows = trios.size() + quartets.size() + 1 + forks.size();
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

  for (auto &&[a, b, c] : forks) {
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "fork_cons", 0, nullptr, nullptr,
                              0.0, 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[a - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[b - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[c - 1], 1.0);
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons); // Release memory once added to SCIP
  }

  for (auto &&fork : extended_forks) {
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "extended_fork_cons", 0, nullptr,
                              nullptr, 0.0, 4.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[0] - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[1] - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[2] - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[3] - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[4] - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[5] - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[fork[6] - 1], 1.0);
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons); // Release memory once added to SCIP
  }

  for (auto &&trio : trios) {
    auto edges = tree->get_trio_edges(trio);
    auto local_bound =
        edges.size() - 1 < upper_limit_ ? edges.size() - 1 : upper_limit_;
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "trio_cons", 0, nullptr, nullptr,
                              1.0, local_bound);
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons); // Release memory once added to SCIP
  }

  for (auto &&quartet : quartets) {
    auto edges = tree->get_quartet_edges(quartet);
    auto local_bound =
        edges.size() < upper_limit_ ? edges.size() : upper_limit_;
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "quartet_cons", 0, nullptr, nullptr,
                              1.0, local_bound);
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons);
  }

  SCIP_CONS *cons_all;
  SCIPcreateConsBasicLinear(scip_, &cons_all, "all_edges", 0, nullptr, nullptr,
                            0.0, upper_limit_);
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

void SCIILP::set_priorities() const {
  for (int i = 0; i < vars_.size(); ++i) {
    SCIP_VAR *var = vars_[i];

    int nlocks_down = SCIPvarGetNLocksDown(var);
    int nlocks_up = SCIPvarGetNLocksUp(var);

    int total_occurrences = nlocks_down + nlocks_up;

    SCIPchgVarBranchPriority(scip_, var, total_occurrences);
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
                            nullptr, obj_val, upper_limit_);

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
  input_.compute_trios_quartets(
      trios, quartets, 4 * input_.get_reduced_leaf_count(), components_, true);
  if (trios.empty() && quartets.empty()) {
    return false;
  }

  auto tree = input_.get_trees().at(0).get();

  SCIPfreeTransform(scip_);

  for (auto &&trio : trios) {
    auto edges = tree->get_trio_edges(trio);
    auto local_bound =
        edges.size() - 1 < upper_limit_ ? edges.size() - 1 : upper_limit_;
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "update_trio", 0, nullptr, nullptr,
                              1.0, local_bound);
    for (int idx : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[idx], 1.0);
    }
    SCIP_RETCODE status = SCIPaddCons(scip_, cons);
    assert(status == SCIP_OKAY);
    SCIPreleaseCons(scip_, &cons);
  }

  for (auto &&quartet : quartets) {
    auto edges = tree->get_quartet_edges(quartet);
    auto local_bound =
        edges.size() - 1 < upper_limit_ ? edges.size() - 1 : upper_limit_;
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "update_quartet", 0, nullptr,
                              nullptr, 1.0, local_bound);
    for (int idx : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[idx], 1.0);
    }
    SCIP_RETCODE status = SCIPaddCons(scip_, cons);
    assert(status == SCIP_OKAY);
    SCIPreleaseCons(scip_, &cons);
  }

  return true;
}
