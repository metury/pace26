#include "ilp.h"
#include "scip/scip_numerics.h"
#include "scip/type_var.h"
#include "utils.h"

/// ILP

ILP::ILP(Input &input)
    : input_(input), limit_(2 * std::log2(input.get_reduced_leaf_count())),
      priorities_(input_.get_node_count(), 0) {

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

void ILP::drop_ilp() {
  if (scip_ != nullptr) {
    for (auto var : vars_) {
      SCIPreleaseVar(scip_, &var);
    }
    SCIPfree(&scip_);
  }
}

void ILP::warm_start(std::set<int> &edges_to_erase) {
  auto forks = std::vector<Fork>();
  auto extended_forks = std::vector<ExtendedFork>();
  input_.compute_breakable_forks(forks, extended_forks);
  for (auto &&[a, b, c] : forks) {
    if (edges_to_erase.contains(a) &&
        (edges_to_erase.contains(b) || edges_to_erase.contains(c))) {
      edges_to_erase.erase(a);
      edges_to_erase.insert(b);
      edges_to_erase.insert(c);
    }
  }
  SCIP_SOL *sol;
  SCIP_Bool stored;

  SCIPcreateSol(scip_, &sol, NULL);

  for (size_t i = 0; i < vars_.size(); ++i) {
    SCIP_VAR *var = vars_[i];

    double val = edges_to_erase.contains(i + 1) ? 1.0 : 0.0;

    SCIPsetSolVal(scip_, sol, var, val);
  }

  SCIPaddSolFree(scip_, &sol, &stored);
}

void ILP::add_trio_constr(Trio &t) {
  auto tree = input_.get_trees().at(0).get();
  t.used = true;
  auto edges = tree->get_trio_edges(t);
  int edges_size = edges.size() - 1;
  auto local_bound = std::min(edges_size, upper_limit_);
  if (heuristics_) {
    local_bound = upper_limit_;
  }
  SCIP_CONS *cons;
  SCIPcreateConsBasicLinear(scip_, &cons, "trio_cons", 0, nullptr, nullptr, 1.0,
                            local_bound);
  for (int edge : edges) {
    SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
  }
  SCIPaddCons(scip_, cons);
  SCIPreleaseCons(scip_, &cons);
}

void ILP::add_quartet_constr(Quartet &q) {
  auto tree = input_.get_trees().at(0).get();
  q.used = true;
  auto edges = tree->get_quartet_edges(q);
  int edges_size = edges.size();
  auto local_bound = std::min(edges_size, upper_limit_);
  if (heuristics_) {
    local_bound = upper_limit_;
  }
  SCIP_CONS *cons;
  SCIPcreateConsBasicLinear(scip_, &cons, "quartet_cons", 0, nullptr, nullptr,
                            1.0, local_bound);
  for (int edge : edges) {
    SCIPaddCoefLinear(scip_, cons, vars_[edge], 1.0);
  }
  SCIPaddCons(scip_, cons);
  SCIPreleaseCons(scip_, &cons);
}

void ILP::initialize(int lb, int ub, bool h) {
  heuristics_ = h;
  upper_limit_ = ub;
  SCIPcreate(&scip_);
  SCIPincludeDefaultPlugins(scip_);
  SCIPcreateProbBasic(scip_, "PACE2026 - MAF");
  SCIPsetIntParam(scip_, "display/verblevel", 0);
  components_ = std::vector<int>(input_.get_leaf_count() + 1, 1);

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

  if (!heuristics_) {
    auto forks = std::vector<Fork>();
    auto extended_forks = std::vector<ExtendedFork>();
    input_.compute_breakable_forks(forks, extended_forks);
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

    for (auto &&trio : trios_) {
      if (trio.used) {
        add_trio_constr(trio);
      }
    }
    for (auto &&quartet : quartets_) {
      if (quartet.used) {
        add_quartet_constr(quartet);
      }
    }
  } else {
    auto number_of_constraints = std::max(2.0, std::log2(trios_.size()));
    std::vector<int> counters(input_.get_leaf_count() + 1,
                              number_of_constraints);

    for (int i = 0; i < trios_.size(); ++i) {
      if (trios_[i].size > limit_) {
        break; // Take at least N of them. Take all constraints of length at
        //  most limit_.
      }
      if (has_positive(counters[trios_[i].a], counters[trios_[i].b],
                       counters[trios_[i].c])) {
        add_trio_constr(trios_[i]);

        decrement_to_zero(counters[trios_[i].a], counters[trios_[i].b],
                          counters[trios_[i].c]);
      }
    }

    number_of_constraints = std::max(2.0, std::log2(quartets_.size()));
    std::fill(counters.begin(), counters.end(), number_of_constraints);

    for (int i = 0; i < quartets_.size(); ++i) {
      if (quartets_[i].size > limit_) {
        break; // Take at least N of them. Take all constraints of length at
        // most limit_.
      }
      if (has_positive(counters[quartets_[i].a], counters[quartets_[i].b],
                       counters[quartets_[i].x], counters[quartets_[i].y])) {
        add_quartet_constr(quartets_[i]);

        decrement_to_zero(counters[quartets_[i].a], counters[quartets_[i].b],
                          counters[quartets_[i].x], counters[quartets_[i].y]);
      }
    }
  }

  // for (auto &&[a, b, c, d, e, f, g] : extended_forks) {
  //   SCIP_CONS *cons;
  //   SCIPcreateConsBasicLinear(scip_, &cons, "extended_fork_cons", 0,
  //   nullptr,
  //                             nullptr, 0.0, 4.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[a - 1], 2.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[b - 1], 2.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[c - 1], 2.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[d - 1], 1.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[e - 1], 1.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[f - 1], 1.0);
  //   SCIPaddCoefLinear(scip_, cons, vars_[g - 1], 1.0);
  //   SCIPaddCons(scip_, cons);
  //   SCIPreleaseCons(scip_, &cons);
  // }

  SCIP_CONS *cons_all;
  SCIPcreateConsBasicLinear(scip_, &cons_all, "all_edges", 0, nullptr, nullptr,
                            lb, upper_limit_);
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
  // auto counters = std::vector<unsigned>(input_.get_node_count(), 0);
  // auto tree = input_.get_trees().at(0).get();
  // for (auto &&trio : trios_) {
  //   if (trio.used) {
  //     auto edges = tree->get_trio_edges(trio);
  //     for (auto &&edge : edges) {
  //       ++counters[edge];
  //     }
  //   }
  // }
  // for (auto &&quartet : quartets_) {
  //   if (quartet.used) {
  //     auto edges = tree->get_quartet_edges(quartet);
  //     for (auto &&edge : edges) {
  //       ++counters[edge];
  //     }
  //   }
  // }
  for (int i = 0; i < vars_.size(); ++i) {
    SCIP_VAR *var = vars_[i];

    int total_occurrences = priorities_[i];

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
      priorities_[col] += 1;
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

  if (heuristics_) {
    SCIP_CONS *new_cons_h;
    SCIPcreateConsBasicLinear(scip_, &new_cons_h, "post_run_h_cons", 0, nullptr,
                              nullptr, edges_to_erase.size(),
                              SCIPinfinity(scip_));

    for (auto &&e : edges_to_erase) {
      SCIPaddCoefLinear(scip_, new_cons_h, vars_[e - 1], 1.0);
    }

    SCIPaddCons(scip_, new_cons_h);
    SCIPreleaseCons(scip_, &new_cons_h);
  }

  return edges_to_erase;
}

bool ILP::update() {
  auto tree = input_.get_trees().at(0).get();

  SCIPfreeTransform(scip_);

  bool end = true;

  auto number_of_constraints = std::max(2.0, std::log2(trios_.size()));
  std::vector<int> counters(input_.get_leaf_count() + 1, number_of_constraints);

  for (auto &&trio : trios_) {
    bool satisfied = components_[trio.a] != components_[trio.b] ||
                     components_[trio.a] != components_[trio.c];
    if (!satisfied &&
        has_positive(counters[trio.a], counters[trio.b], counters[trio.c])) {
      add_trio_constr(trio);
      end = false;
      decrement_to_zero(counters[trio.a], counters[trio.b], counters[trio.c]);
    }
  }

  number_of_constraints = std::max(2.0, std::log2(quartets_.size()));
  std::fill(counters.begin(), counters.end(), number_of_constraints);

  for (auto &&quartet : quartets_) {
    bool satisfied = components_[quartet.a] != components_[quartet.b] ||
                     components_[quartet.x] != components_[quartet.y];
    if (!satisfied && has_positive(counters[quartet.a], counters[quartet.b],
                                   counters[quartet.x], counters[quartet.y])) {
      add_quartet_constr(quartet);
      end = false;
      decrement_to_zero(counters[quartet.a], counters[quartet.b],
                        counters[quartet.x], counters[quartet.y]);
    }
  }

  if (!heuristics_) {
    std::erase_if(trios_, [](const auto &a) { return a.used; });
    std::erase_if(quartets_, [](const auto &a) { return a.used; });
  }

  return !end;
}
