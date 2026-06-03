#include "tree.h"
#include "utils.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// ==========
// == Tree ==
// ==========

Tree::Tree(std::unique_ptr<Node> root) : root_(std::move(root)) {
  root_->set_parent(nullptr);
}

void Tree::assign_numbers(int i, int n) {
  root_->assign_numbers(i * (n - 1) + 2);
}

void Tree::consolidate() {
  root_->consolidate();
  while (root_ != std::nullptr_t() && !root_->is_leaf() &&
         (root_->get_left() == nullptr || root_->get_right() == nullptr)) {
    if (root_->get_left() == nullptr && root_->get_right() == nullptr) {
      root_ = std::nullptr_t();
    }
    if (root_->get_left() == nullptr) {
      root_ = std::make_unique<Node>(std::move(*root_->remove_right()));
    } else {
      root_ = std::make_unique<Node>(std::move(*root_->remove_left()));
    }
  }
}

void Tree::contract_cherry(int first, int second) {
  auto node_a = descendants_.at(first);
  auto node_b = descendants_.at(second);
  if (node_a->get_parent() != nullptr &&
      *node_a->get_parent() == *node_b->get_parent()) {
    auto parent = node_a->get_parent();
    parent->set_value(first);
    parent->set_type(LEAF);
    parent->remove_left();
    parent->remove_right();
    descendants_.insert_or_assign(first, parent);
    descendants_.erase(second);
  }
}

void Tree::contract_chain(int a, int b, int c, int d) {
  auto node_b = get_leaf(b);
  auto parent_b = node_b->get_parent();
  std::unique_ptr<Node> child;
  if (parent_b->get_left() == node_b) {
    parent_b->remove_left();
    child = parent_b->remove_right();
  } else {
    parent_b->remove_right();
    child = parent_b->remove_left();
  }
  auto parent = parent_b->get_parent();
  if (parent_b == parent->get_left()) {
    parent->remove_left();
    parent->set_left(std::move(child));
  } else {
    parent->remove_right();
    parent->set_right(std::move(child));
  }
}

void Tree::compute_lca_leafs() {
  descendants_ = root_->compute_lca_leafs(pairs_, triples_);
}

void Tree::get_edges(Node *below, Node *above, std::set<int> &edges) const {
  auto current = below;
  while (current->get_value() != above->get_value() &&
         current->get_parent() != nullptr) {
    edges.insert(current->get_value() - 1);
    current = current->get_parent();
  }
}

bool Tree::is_cherry(int first, int second) const {
  auto node_a = descendants_.at(first);
  auto node_b = descendants_.at(second);
  return node_a->get_parent() != nullptr &&
         *node_a->get_parent() == *node_b->get_parent();
}

bool Tree::is_chain(int a, int b, int c, int d) const {
  auto parent = get_leaf(d)->get_parent();
  if (parent == nullptr)
    return false;
  parent = parent->get_parent();
  auto tmp_parent = get_leaf(c)->get_parent();
  if (tmp_parent == nullptr || parent != tmp_parent)
    return false;
  parent = parent->get_parent();
  tmp_parent = get_leaf(b)->get_parent();
  if (tmp_parent == nullptr || parent != tmp_parent)
    return false;
  parent = parent->get_parent();
  tmp_parent = get_leaf(a)->get_parent();
  if (tmp_parent == nullptr || parent != tmp_parent)
    return false;
  return true;
}

bool Tree::is_empty() const { return root_ == std::nullptr_t(); }

Node *Tree::lca_query(int first, int second) const {
  return pairs_.at(get_lca_key(first, second));
}

Node *Tree::lca_query(int first, int second, int third) const {
  return triples_.at(get_lca_key(first, second, third));
}

void Tree::write(std::ostream &os,
                 const std::unordered_map<int, std::string> &subst) const {
  get_root()->write_with_substitution(os, subst);
  os << ";" << std::endl;
}

std::istream &operator>>(std::istream &is, Tree &t) {
  is >> *(t.get_root());
  return is;
}

// =======================
// == TreeDecomposition ==
// =======================

TreeDecomposition::TreeDecomposition(const std::string &str) {
  auto parts = split(str.substr(1, str.size() - 2), ',');
  treewidth_ = std::stoi(parts[0]);
  std::vector<int> bag;
  bool edges = false;
  for (int i = 1; i < parts.size(); ++i) {
    if (edges) {
      auto const pos = parts[i].find_last_of('[');
      const auto first = parts[i].substr(pos + 1);
      edges_.push_back(
          std::make_tuple(std::stoi(first), std::stoi(parts[++i])));
    } else {
      if (parts[i][0] == '[') {
        if (bag.size() > 0) {
          bags_.push_back(bag);
          bag.erase(bag.begin(), bag.end());
        }
      }
      auto const pos = parts[i].find_last_of('[');
      const auto first = parts[i].substr(pos + 1);
      bag.push_back(std::stoi(first));
      auto len = parts[i].length();
      if (parts[i].size() > 2 && parts[i][len - 2] == ']') {
        edges = true;
        bags_.push_back(bag);
      }
    }
  }
}

void TreeDecomposition::write(std::ostream &os) {
  os << "TreeWidth: " << treewidth_ << std::endl;
  os << "Bags:" << std::endl;
  for (auto &&bag : bags_) {
    os << "\t";
    for (auto &&val : bag)
      os << val << ";";
    os << std::endl;
  }
  os << "Edges:" << std::endl;
  for (auto &&edge : edges_) {
    std::cout << "\t[" << std::get<0>(edge) << "," << std::get<1>(edge) << "]"
              << std::endl;
  }
}

// ===========
// == Input ==
// ===========

Input::Input(std::istream &is) {
  std::string line;
  while (getline(is, line)) {
    if (line.size() > 0 && line[0] == '#') {
      if (line.size() > 1 && line[1] == 'p') {
        auto tokens = split(line);
        t_ = std::stoi(tokens[1]);
        n_ = std::stoi(tokens[2]);
      } else if (line.size() > 1 && line[1] == 'x') {
        auto tokens = split(line);
        if (tokens[1] == "treedecomp") {
          set_tree_decomposition(tokens[2]);
        }
      }
    } else if (!line.empty()) {
      trees_.push_back(std::make_unique<Tree>());
      std::istringstream iss(line);
      iss >> *(trees_.at(trees_.size() - 1).get());
    }
    if (t_ > 0 && trees_.size() == t_) {
      break;
    }
  }
  assign_numbers();
  compute_all_lca();
}

void Input::assign_numbers() {
  for (int i = 0; i < t_; ++i) {
    trees_[i]->assign_numbers(i + 1, n_);
  }
}

void Input::compute_all_lca() {
  for (auto &&tree : trees_) {
    tree->compute_lca_leafs();
  }
}

std::vector<std::tuple<int, int, int>> Input::compute_trios() {
  std::vector<std::tuple<int, int, int>> trios;
  auto tree1 = trees_[0].get();
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.find(a) != excluded_leafs_.end())
      continue;
    for (auto b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.find(b) != excluded_leafs_.end())
        continue;
      auto node1_a_b = tree1->lca_query(a, b);
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (excluded_leafs_.find(c) != excluded_leafs_.end())
          continue;
        if (c == b || c == a)
          continue;
        auto node1_ab_c = tree1->lca_query(a, b, c);
        // Either one is below and the other match or the other way around.
        auto c_below_ab_1 = node1_a_b == node1_ab_c;
        if (!c_below_ab_1) {
          for (auto &&tree2 : get_trees()) {
            if (tree1->get_root()->get_value() ==
                tree2->get_root()->get_value())
              continue;
            auto node2_a_b = tree2->lca_query(a, b);
            auto node2_ab_c = tree2->lca_query(a, b, c);
            auto c_below_ab_2 = node2_a_b == node2_ab_c;
            if (c_below_ab_2) {
              // We found triplet.
              trios.push_back(std::make_tuple(a, b, c));
              break;
            }
          }
        }
      }
    }
  }
  return trios;
}

std::vector<std::tuple<int, int, int, int>> Input::compute_quartets() {
  std::vector<std::tuple<int, int, int, int>> quartets;
  auto tree1 = trees_[0].get();
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.find(a) != excluded_leafs_.end())
      continue;
    for (auto b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.find(b) != excluded_leafs_.end())
        continue;
      auto node1_a_b = tree1->lca_query(a, b);
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (c == b || c == a ||
            excluded_leafs_.find(c) != excluded_leafs_.end())
          continue;
        for (auto d = c + 1; d <= get_leaf_count(); ++d) {
          if (d == c || d == a || d == b ||
              excluded_leafs_.find(d) != excluded_leafs_.end())
            continue;
          auto node1_ab_c = tree1->lca_query(a, b, c);
          auto node1_ab_d = tree1->lca_query(a, b, d);
          auto c_below_ab_1 = node1_a_b == node1_ab_c;
          auto d_below_ab_1 = node1_a_b == node1_ab_d;
          auto node1_cd_a = tree1->lca_query(c, d, a);
          auto node1_cd_b = tree1->lca_query(c, d, b);
          auto node1_c_d = tree1->lca_query(c, d);
          auto a_below_cd_1 = node1_c_d == node1_cd_a;
          auto b_below_cd_1 = node1_c_d == node1_cd_b;
          if (!c_below_ab_1 && !d_below_ab_1) {
            if (!a_below_cd_1 && !b_below_cd_1 && a > c)
              continue;
            for (auto &&tree2 : get_trees()) {
              if (tree1->get_root()->get_value() ==
                  tree2->get_root()->get_value())
                continue;
              auto node2_a_b = tree2->lca_query(a, b);
              auto node2_c_d = tree2->lca_query(c, d);
              auto node2_ab_c = tree2->lca_query(a, b, c);
              auto node2_ab_d = tree2->lca_query(a, b, d);
              auto node2_cd_a = tree2->lca_query(a, d, c);
              auto node2_cd_b = tree2->lca_query(c, b, d);
              auto c_below_ab_2 = node2_a_b == node2_ab_c;
              auto d_below_ab_2 = node2_a_b == node2_ab_d;
              auto a_below_cd_2 = node2_c_d == node2_cd_a;
              auto b_below_cd_2 = node2_c_d == node2_cd_b;
              // We look whether it is mashed up or not.
              // Therefore at least c or d must be below and a or b ust be
              // below.
              if ((c_below_ab_2 || d_below_ab_2) &&
                  (a_below_cd_2 || b_below_cd_2)) {
                quartets.push_back(std::make_tuple(a, b, c, d));
                break;
              }
            }
          }
        }
      }
    }
  }
  return quartets;
}

std::unordered_map<int, std::set<int>> Input::compute_parents() {
  std::unordered_map<int, std::set<int>> parents{};
  if (trees_.empty())
    return parents;
  std::set<int> current{};
  compute_parents_(trees_[0]->get_root(), parents, current);
  return parents;
}

void Input::compute_parents_(Node *node,
                             std::unordered_map<int, std::set<int>> &parents,
                             std::set<int> &current) {
  if (!current.empty())
    parents.insert_or_assign(node->get_value(), current);
  if (!node->is_leaf()) {
    auto left = node->get_left();
    current.insert(node->get_value());
    compute_parents_(left, parents, current);
    auto right = node->get_right();
    compute_parents_(right, parents, current);
    current.erase(node->get_value());
  }
}

void Input::contract_chains_(Node *n, std::vector<int> &candidates) {
  if (n == nullptr || n->is_leaf()) {
    return;
  }
  bool chain = false;
  if (candidates.size() == 4) {
    chain = true;
    for (auto &&tree : get_trees()) {
      chain = chain && tree->is_chain(candidates[0], candidates[1],
                                      candidates[2], candidates[3]);
    }
    if (!chain) {
      candidates.erase(candidates.begin());
    }
  }
  if (chain) {
    for (auto &&tree : get_trees()) {
      tree->contract_chain(candidates[0], candidates[1], candidates[2],
                           candidates[3]);
    }
    excluded_leafs_.insert(candidates[1]);
    std::cout << "# Contracted part of a chain: " << CYAN << candidates[1]
              << RESET << "." << std::endl;
    candidates.erase(candidates.begin() + 1);
  }
  if (n->get_left()->is_leaf() && !n->get_right()->is_leaf()) {
    candidates.push_back(n->get_left()->get_value());
    contract_chains_(n->get_right(), candidates);
  } else if (n->get_right()->is_leaf() && !n->get_left()->is_leaf()) {
    candidates.push_back(n->get_right()->get_value());
    contract_chains_(n->get_left(), candidates);
  } else {
    candidates.clear();
    contract_chains_(n->get_left(), candidates);
    contract_chains_(n->get_left(), candidates);
  }
}

void Input::contract_cherries_chains() {
  contract_cherries_(trees_[0]->get_root());
  compute_all_lca();
  for (auto &&[key, val] : contracted_) {
    std::cout << "# Replacing cherry: " << RED << key << RESET << " <- " << RED
              << val << RESET << std::endl;
  }
  if (contracted_.empty()) {
    std::cout << "# No " << RED << "cherry" << RESET << " found." << std::endl;
  }
  // auto cherries = excluded_leafs_.size();
  // std::vector<int> candidates;
  // contract_chains_(trees_[0]->get_root(), candidates);
  // compute_all_lca();
  // if (excluded_leafs_.size() == cherries) {
  //   std::cout << "# No " << CYAN << "chain" << RESET << " was contracted."
  //             << std::endl;
  // }
  // std::cout << "# Number of leafs reduced by " << RED << cherries << RESET
  //           << " + " << CYAN << excluded_leafs_.size() - cherries << RESET
  //           << " = " << excluded_leafs_.size() << "." << std::endl;
}

void Input::contract_cherries_(Node *n) {
  if (n->is_leaf()) {
    return;
  }
  // First contract all possbile descendants.
  contract_cherries_(n->get_left());
  contract_cherries_(n->get_right());
  // Now also this one if it is a cherry.
  auto left = n->get_left();
  auto right = n->get_right();
  auto left_value = left->get_value();
  auto right_value = right->get_value();
  if (left->is_leaf() && right->is_leaf()) {
    for (auto &&tree : trees_) {
      if (!tree->is_cherry(left_value, right_value)) {
        return;
      }
    }
    for (auto &&tree : trees_) {
      tree->contract_cherry(left_value, right_value);
    }
    add_contracted_(left_value, right_value);
    return;
  }
}

void Input::set_tree_decomposition(const std::string &str) {
  decomp_ = TreeDecomposition(str);
}

std::vector<std::unique_ptr<Tree>>
Input::remove_edges(const std::set<int> &edges_to_remove) {
  std::vector<std::unique_ptr<Node>> output;
  std::vector<std::unique_ptr<Tree>> trees;
  if (trees_.size() == 0) {
    return trees;
  }
  output.push_back(std::make_unique<Node>(*trees_[0]->get_root()));
  remove_edges_(edges_to_remove, output, output.at(0).get());
  for (auto &&tree : output) {
    trees.push_back(std::make_unique<Tree>(std::move(tree)));
  }
  for (auto &&tree : trees) {
    tree->consolidate();
  }
  return trees;
}

void Input::remove_edges_(const std::set<int> &edges_to_remove,
                          std::vector<std::unique_ptr<Node>> &trees,
                          Node *current_tree) {
  if (current_tree->is_leaf()) {
    return;
  }
  auto left_val = current_tree->get_left()->get_value();
  if (edges_to_remove.find(left_val) != edges_to_remove.end()) {
    trees.push_back(std::move(current_tree->remove_left()));
    remove_edges_(edges_to_remove, trees, trees.at(trees.size() - 1).get());
  } else {
    remove_edges_(edges_to_remove, trees, current_tree->get_left());
  }
  auto right_val = current_tree->get_right()->get_value();
  if (edges_to_remove.find(right_val) != edges_to_remove.end()) {
    trees.push_back(current_tree->remove_right());
    remove_edges_(edges_to_remove, trees, trees.at(trees.size() - 1).get());
  } else {
    remove_edges_(edges_to_remove, trees, current_tree->get_right());
  }
}

void Input::add_contracted_(int first, int second) {
  std::ostringstream oss;
  if (contracted_.find(first) != contracted_.end() &&
      contracted_.find(second) != contracted_.end()) {
    oss << "(" << contracted_.at(first) << "," << contracted_.at(second) << ")";
    contracted_.erase(second);
  } else if (contracted_.find(first) != contracted_.end()) {
    oss << "(" << contracted_.at(first) << "," << second << ")";
  } else if (contracted_.find(second) != contracted_.end()) {
    oss << "(" << first << "," << contracted_.at(second) << ")";
    contracted_.erase(second);
  } else {
    oss << "(" << first << "," << second << ")";
  }
  excluded_leafs_.insert(second);
  contracted_.insert_or_assign(first, oss.str());
}
