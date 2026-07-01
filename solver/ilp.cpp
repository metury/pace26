#include "ilp.h"
#include <random>

/// ILP

ILP::ILP(Input &input)
    : input_(input), limit_(2 * std::log2(input.get_reduced_leaf_count())),
      upper_limit_(std::max(input_.get_reduced_leaf_count() - 2, 0)) {
  SCIPcreate(&scip_);
  SCIPincludeDefaultPlugins(scip_);
  SCIPcreateProbBasic(scip_, "PACE2026 - MAF");
  SCIPsetIntParam(scip_, "display/verblevel", 0);

  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);

  trios_ = std::vector<Trio>();
  quartets_ = std::vector<Quartet>();
  input_.compute_trios_quartets(trios_, quartets_);
  std::sort(trios_.begin(), trios_.end(),
            [](const auto &a, const auto &b) { return a.size < b.size; });
  std::sort(quartets_.begin(), quartets_.end(),
            [](const auto &a, const auto &b) { return a.size < b.size; });
}

ILP::~ILP() {
  if (scip_ != nullptr) {
    for (auto var : vars_) {
      SCIPreleaseVar(scip_, &var);
    }
    SCIPfree(&scip_);
  }
}

void ILP::initialize() {
  auto forks = std::vector<Fork>();
  auto extended_forks = std::vector<ExtendedFork>();
  input_.compute_breakable_forks(forks, extended_forks);

  auto number_of_edges = input_.get_node_count();
  auto tree = input_.get_trees().at(0).get();

  SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE);

  vars_.resize(number_of_edges);
  for (int col = 0; col < number_of_edges; ++col) {
    std::string name = "e_" + std::to_string(col + 1);
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

  for (auto &&[a, b, c, d, e, f, g] : extended_forks) {
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "extended_fork_cons", 0, nullptr,
                              nullptr, 0.0, 4.0);
    SCIPaddCoefLinear(scip_, cons, vars_[a - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[b - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[c - 1], 2.0);
    SCIPaddCoefLinear(scip_, cons, vars_[d - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[e - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[f - 1], 1.0);
    SCIPaddCoefLinear(scip_, cons, vars_[g - 1], 1.0);
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons);
  }

  auto number_of_constraints = std::exp2(std::log2(trios_.size()) / 2);

  for (int i = 0; i < trios_.size(); ++i) {
    if (i > number_of_constraints && trios_[i].size > limit_) {
      break; // Take at least N of them. Take all constraints of length at
      //  most limit_.
    }
    trios_[i].used = true;
    auto edges = tree->get_trio_edges(trios_[i]);
    int edges_size = edges.size() - 1;
    auto local_bound = std::min(edges_size, upper_limit_);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "trio_cons", 0, nullptr, nullptr,
                              1.0, local_bound);
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIPaddCons(scip_, cons);
    SCIPreleaseCons(scip_, &cons);
  }

  number_of_constraints = std::exp2(std::log2(quartets_.size()) / 2);

  for (int i = 0; i < quartets_.size(); ++i) {
    if (i > number_of_constraints && quartets_[i].size > limit_) {
      break; // Take at least N of them. Take all constraints of length at
      // most limit_.
    }
    quartets_[i].used = true;
    auto edges = tree->get_quartet_edges(quartets_[i]);
    int edges_size = edges.size();
    auto local_bound = std::min(edges_size, upper_limit_);
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

void ILP::set_priorities() const {
  auto counters = std::vector<unsigned>(input_.get_node_count(), 0);
  auto tree = input_.get_trees().at(0).get();
  for (auto &&trio : trios_) {
    if (trio.used) {
      auto edges = tree->get_trio_edges(trio);
      for (auto &&edge : edges) {
        ++counters[edge];
      }
    }
  }
  for (auto &&quartet : quartets_) {
    if (quartet.used) {
      auto edges = tree->get_quartet_edges(quartet);
      for (auto &&edge : edges) {
        ++counters[edge];
      }
    }
  }
  for (int i = 1; i < input_.get_leaf_count() + 1; ++i) {
    SCIP_VAR *var = vars_[i];

    int total_occurrences = counters[i];

    SCIPchgVarBranchPriority(scip_, var, std::min(1, total_occurrences));
  }
}

std::set<int> ILP::run() {
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

bool ILP::update() {
  auto tree = input_.get_trees().at(0).get();

  SCIPfreeTransform(scip_);
  bool end = true;
  int counter = 0;

  std::random_device rd;
  std::mt19937 gen(rd());
  double n = input_.get_reduced_leaf_count();
  double p = n / trios_.size();
  std::bernoulli_distribution d(p);

  for (auto &&trio : trios_) {
    bool pass = d(gen);
    bool satisfied = components_[trio.a] != components_[trio.b] ||
                     components_[trio.a] != components_[trio.c];
    if (trio.used || (satisfied && !pass)) {
      continue;
    }

    end = end && satisfied;
    trio.used = true;
    ++counter;
    auto edges = tree->get_trio_edges(trio);
    int edges_size = edges.size() - 1;
    auto local_bound = std::min(edges_size, upper_limit_);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "update_trio", 0, nullptr, nullptr,
                              1.0, local_bound);
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIP_RETCODE status = SCIPaddCons(scip_, cons);
    assert(status == SCIP_OKAY);
    SCIPreleaseCons(scip_, &cons);
  }

  counter = 0;
  p = n / quartets_.size();
  d = std::bernoulli_distribution(p);

  for (auto &&quartet : quartets_) {
    bool pass = d(gen);
    bool satisfied = components_[quartet.a] != components_[quartet.b] ||
                     components_[quartet.x] != components_[quartet.y];
    if (quartet.used || (satisfied && !pass)) {
      continue;
    }
    end = end && satisfied;
    quartet.used = true;
    ++counter;
    auto edges = tree->get_quartet_edges(quartet);
    int edges_size = edges.size();
    auto local_bound = std::min(edges_size, upper_limit_);
    SCIP_CONS *cons;
    SCIPcreateConsBasicLinear(scip_, &cons, "update_quartet", 0, nullptr,
                              nullptr, 1.0, local_bound);
    for (int edge : edges) {
      SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
    }
    SCIP_RETCODE status = SCIPaddCons(scip_, cons);
    assert(status == SCIP_OKAY);
    SCIPreleaseCons(scip_, &cons);
  }

  return !end;
}
