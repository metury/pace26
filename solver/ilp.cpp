#include "ilp.h"
#include "scip/scip_sol.h"
#include "tree.h"
#include "utils.h"
#include <set>
#include <vector>

/// SCIILP

SCIILP::SCIILP(Input &input)
    : input_(input), limit_(2 * std::log2(input.get_reduced_leaf_count())),
      upper_limit_(input_.get_reduced_leaf_count() - 2) {
  SCIPcreate(&scip_);
  SCIPincludeDefaultPlugins(scip_);
  SCIPcreateProbBasic(scip_, "PACE2026 - MAF");
  SCIPsetIntParam(scip_, "display/verblevel", 0);
  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);
  trios_ = std::vector<std::array<int, 4>>();
  quartets_ = std::vector<std::array<int, 5>>();
  input_.compute_trios_quartets(trios_, quartets_, limit_, components_, true);
  std::sort(trios_.begin(), trios_.end(),
            [](const auto &a, const auto &b) { return a[3] < b[3]; });
  std::sort(quartets_.begin(), quartets_.end(),
            [](const auto &a, const auto &b) { return a[4] < b[4]; });
  // trios_ = std::vector<std::array<int, 3>>();
  // quartets_ = std::vector<std::array<int, 4>>();
  // std::transform(trios.begin(), trios.end(), std::back_inserter(trios_),
  //                [](const std::array<int, 4> &arr4) {
  //                  return std::array<int, 3>{arr4[0], arr4[1], arr4[2]};
  //                });
  // std::transform(
  //     quartets.begin(), quartets.end(), std::back_inserter(quartets_),
  //     [](const std::array<int, 5> &arr4) {
  //       return std::array<int, 4>{arr4[0], arr4[1], arr4[2], arr4[3]};
  //     });
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
  // auto trios = std::vector<std::tuple<int, int, int>>();
  // auto quartets = std::vector<std::tuple<int, int, int, int>>();
  // input_.compute_trios_quartets(trios, quartets, limit_, components_,
  //                               all_constraints);
  auto forks = std::vector<std::tuple<int, int, int>>();
  auto extended_forks = std::vector<std::array<int, 7>>();
  input_.compute_breakable_forks(forks, extended_forks);
  auto number_of_edges = input_.get_node_count();
  auto number_of_rows = trios_.size() + quartets_.size() + 1 + forks.size();
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
    SCIPreleaseCons(scip_, &cons);
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
    SCIPreleaseCons(scip_, &cons);
  }

  for (int i = 0; i < trios_.size(); ++i) {
    if (i > input_.get_reduced_leaf_count() && trios_[i][3] > limit_) {
      break;
    }
    auto trio = std::make_tuple(trios_[i][0], trios_[i][1], trios_[i][2]);
    // for (auto &&trio : trios) {
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
    SCIPreleaseCons(scip_, &cons);
  }

  for (int i = 0; i < quartets_.size(); ++i) {
    if (i > input_.get_reduced_leaf_count() && quartets_[i][3] > limit_) {
      break;
    }
    auto quartet = std::make_tuple(quartets_[i][0], quartets_[i][1],
                                   quartets_[i][2], quartets_[i][3]);
    // for (auto &&quartet : quartets) {
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

  // SCIP_STATUS status = SCIPgetStatus(scip_);
  // assert(status == SCIP_STATUS_OPTIMAL);

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
  auto tree = input_.get_trees().at(0).get();

  SCIPfreeTransform(scip_);

  bool end = true;
  int i = 0;
  int counter = 0;
  // for (auto &&trio : trios) {
  for (int i = 0; i < trios_.size(); ++i) {
    if (counter > input_.get_reduced_leaf_count()) {
      break;
    }
    if (components_[trios_[i][0]] != components_[trios_[i][1]] ||
        components_[trios_[i][0]] != components_[trios_[i][2]]) {
      continue;
    }
    end = false;
    counter += 1;
    auto trio = std::make_tuple(trios_[i][0], trios_[i][1], trios_[i][2]);
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

  i = 0;
  counter = 0;

  // for (auto &&quartet : quartets) {
  for (; i < quartets_.size(); ++i) {
    if (counter > input_.get_reduced_leaf_count()) {
      break;
    }
    if (components_[quartets_[i][0]] != components_[quartets_[i][1]] ||
        components_[quartets_[i][2]] != components_[quartets_[i][3]]) {
      continue;
    }
    end = false;
    counter += 1;
    auto quartet = std::make_tuple(quartets_[i][0], quartets_[i][1],
                                   quartets_[i][2], quartets_[i][3]);
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

  return !end;
}
