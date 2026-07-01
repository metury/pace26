#include "tree.h"
#include "utils.h"
#include <cmath>
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

void Tree::consolidate() {
  root_->consolidate();
  while (root_ != std::nullptr_t() && !root_->is_leaf() &&
         (root_->get_left() == nullptr || root_->get_right() == nullptr)) {
    if (root_->get_left() == nullptr && root_->get_right() == nullptr) {
      root_ = std::nullptr_t();
    } else if (root_->get_left() == nullptr) {
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

void Tree::compute_lca_leafs(int number_of_nodes) {
  lca_table_ = std::vector<std::vector<int>>(
      number_of_nodes, std::vector<int>(number_of_nodes, 1));
  descendants_ = root_->compute_lca_leafs(lca_table_);
}

void Tree::get_edges(Node *below, Node *above, std::set<int> &edges) const {
  auto current = below;
  while (current->get_value() != above->get_value() &&
         current->get_parent() != nullptr) {
    edges.insert(current->get_value() - 1);
    current = current->get_parent();
  }
}

std::set<int> Tree::get_leafs() const {
  std::set<int> leafs;
  root_->get_leafs(leafs);
  return leafs;
}

std::set<int> Tree::get_trio_edges(const Trio &trio) const {
  std::set<int> edges;
  auto [a, b, c, size, used] = trio;
  auto node_a_b = lca_query(a, b);
  auto node_ab_c = lca_query(a, b, c);
  auto node_a = get_leaf(a);
  auto node_b = get_leaf(b);
  auto node_c = get_leaf(c);
  get_edges(node_a, node_a_b, edges);
  get_edges(node_b, node_a_b, edges);
  get_edges(node_c, node_ab_c, edges);
  get_edges(node_a_b, node_ab_c, edges);
  return edges;
}

std::set<int> Tree::get_quartet_edges(const Quartet &quartet) const {
  std::set<int> edges;
  auto [a, b, c, d, size, used] = quartet;
  auto node_a_b = lca_query(a, b);
  auto node_c_d = lca_query(c, d);
  auto node_a = get_leaf(a);
  auto node_b = get_leaf(b);
  auto node_c = get_leaf(c);
  auto node_d = get_leaf(d);
  get_edges(node_a, node_a_b, edges);
  get_edges(node_b, node_a_b, edges);
  get_edges(node_c, node_c_d, edges);
  get_edges(node_d, node_c_d, edges);
  return edges;
}

Fork Tree::get_cherry_edges(int a, int b) const {
  auto node_a = get_leaf(a);
  auto node_b = get_leaf(b);
  auto lca = lca_query(a, b);
  auto parent = node_a->get_parent();
  auto third = parent->get_left();
  if (third == node_a) {
    third = parent->get_right();
  }
  if (parent == lca) {
    parent = node_b->get_parent();
    third = parent->get_left();
    if (third == node_b) {
      third = parent->get_right();
    }
  }
  if (third == node_a) {
    third = node_a->get_parent();
  }
  return {a, b, third->get_value()};
}

bool Tree::has_disjoint_paths(int a, int b, int x, int y) const {
  bool result = lca_query(a, b) != lca_query(x, y);
  if (!result) {
    return false;
  }
  if (lca_query(a, b, x) == lca_query(a, b)) {
    return lca_query(a, x) != lca_query(x, y) &&
           lca_query(b, x) != lca_query(x, y) &&
           lca_query(a, y) != lca_query(x, y) &&
           lca_query(b, y) != lca_query(x, y);
  } else {
    return lca_query(a, x) != lca_query(a, b) &&
           lca_query(b, x) != lca_query(a, b) &&
           lca_query(a, y) != lca_query(a, b) &&
           lca_query(b, y) != lca_query(a, b);
  }
}

bool Tree::has_disjoint_trio(int a, int b, int x) const {
  return lca_query(a, b) != lca_query(a, b, x);
}

bool Tree::is_cherry(int first, int second) const {
  auto node_a = descendants_.at(first);
  auto node_b = descendants_.at(second);
  return node_a->get_parent() != nullptr &&
         *node_a->get_parent() == *node_b->get_parent();
}

Node *Tree::lca_query(int first, int second) const {
  return descendants_.at(lca_table_[first][second]);
}

Node *Tree::lca_query(int first, int second, int third) const {
  auto first_second = lca_table_[first][second];
  return descendants_.at(lca_table_[first_second][third]);
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
        // Do not read tree decomposition!
        // auto tokens = split(line);
        // if (tokens[1] == "treedecomp") {
        //   set_tree_decomposition(tokens[2]);
        // }
      }
    } else if (!line.empty()) {
      trees_.push_back(std::make_unique<Tree>());
      std::istringstream iss(line);
      iss >> *(trees_.at(trees_.size() - 1).get());
    }
  }
  assign_numbers();
  // Output what is inside.
  std::cout << "Instance contains " << GREEN << get_tree_count() << RESET
            << " trees with " << GREEN << get_leaf_count() << RESET
            << " leafs each." << std::endl;
}

void Input::assign_numbers() {
  for (int i = 0; i < t_; ++i) {
    trees_[i]->assign_numbers(1, n_);
  }
}

void Input::compute_all_lca() {
  for (auto &&tree : trees_) {
    tree->compute_lca_leafs(get_node_count() + 1);
  }
}

void Input::compute_trios_quartets(std::vector<Trio> &trios,
                                   std::vector<Quartet> &quartets) {
  auto tree1 = trees_[0].get();
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.contains(a))
      continue;
    for (auto b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.contains(b))
        continue;
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (excluded_leafs_.contains(c) || c == b || c == a)
          continue;
        if (tree1->has_disjoint_trio(a, b, c)) {
          for (auto &&tree2 : get_trees()) {
            if (!tree2->has_disjoint_trio(a, b, c)) {
              Trio trio = {a, b, c, 0};
              trio.size = tree1->get_trio_edges(trio).size();
              trios.push_back(trio);
              break;
            }
          }
        }
        if (c >= a + 1) {
          for (auto d = c + 1; d <= get_leaf_count(); ++d) {
            if (d == a || d == b || excluded_leafs_.contains(d))
              continue;
            if (tree1->has_disjoint_paths(a, b, c, d)) {
              for (auto &&tree2 : get_trees()) {
                if (!tree2->has_disjoint_paths(a, b, c, d)) {
                  Quartet quartet = {a, b, c, d, 0};
                  quartet.size = tree1->get_quartet_edges(quartet).size();
                  quartets.push_back(quartet);
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
}

void Input::compute_trios_quartets_h(std::vector<Trio> &trios,
                                     std::vector<Quartet> &quartets,
                                     std::vector<int> &components) {
  auto tree1 = trees_[0].get();
  auto tree2 = trees_[1].get();
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.contains(a))
      continue;
    for (auto b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.contains(b) || b <= a)
        continue;
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        bool satisfied =
            components[a] != components[b] || components[b] != components[c];
        if (excluded_leafs_.contains(c) || c == b || c == a) {
          continue;
        }
        if (tree1->has_disjoint_trio(a, b, c)) {
          if (!tree2->has_disjoint_trio(a, b, c) && !satisfied) {
            Trio trio = {a, b, c};
            trios.push_back(trio);
            break;
          }
        }
        if (c >= a + 1) {
          for (auto d = c + 1; d <= get_leaf_count(); ++d) {
            bool satisfied = components[a] != components[b] ||
                             components[d] != components[c];
            if (d == a || d == b || excluded_leafs_.contains(d) || satisfied) {
              continue;
            }
            if (tree1->has_disjoint_paths(a, b, c, d)) {
              if (!tree2->has_disjoint_paths(a, b, c, d)) {
                Quartet quartet = {a, b, c, d};
                quartets.push_back(quartet);
                break;
              }
            }
          }
        }
      }
    }
  }
}

void Input::compute_breakable_forks(
    std::vector<Fork> &forks, std::vector<ExtendedFork> &extended_forks) const {
  trees_[0]->get_root()->compute_forks(forks, extended_forks);
}

void Input::compute_fake_cherries(std::vector<Fork> &edges) const {
  auto tree = trees_[0].get();
  for (int i = 1; i < trees_.size(); ++i) {
    std::vector<std::pair<int, int>> cherries;
    trees_[i]->compute_fake_cherries(cherries);
    for (auto &&[a, b] : cherries) {
      edges.push_back(tree->get_cherry_edges(a, b));
    }
  }
}

void Input::contract_cherries() {
  contract_cherries_(trees_[0]->get_root());
  compute_all_lca();
  for (auto &&[key, val] : contracted_) {
    std::cout << "# Replacing cherry: " << RED << key << RESET << " <- " << RED
              << val << RESET << std::endl;
  }
  if (contracted_.empty()) {
    std::cout << "# No " << RED << "cherry" << RESET << " found." << std::endl;
  }
  std::cout << "# Number of leafs reduced by " << RED << excluded_leafs_.size()
            << RESET << std::endl;
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

void Input::delete_lca_tables() {
  for (auto &&tree : trees_) {
    tree->delete_lca_table();
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
  Node tree(*trees_[0]->get_root());
  output.push_back(std::make_unique<Node>(tree));
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
  if (edges_to_remove.contains(left_val)) {
    trees.push_back(std::move(current_tree->remove_left()));
    remove_edges_(edges_to_remove, trees, trees.at(trees.size() - 1).get());
  } else {
    remove_edges_(edges_to_remove, trees, current_tree->get_left());
  }
  auto right_val = current_tree->get_right()->get_value();
  if (edges_to_remove.contains(right_val)) {
    trees.push_back(current_tree->remove_right());
    remove_edges_(edges_to_remove, trees, trees.at(trees.size() - 1).get());
  } else {
    remove_edges_(edges_to_remove, trees, current_tree->get_right());
  }
}

void Input::add_contracted_(int first, int second) {
  std::ostringstream oss;
  if (contracted_.contains(first) && contracted_.contains(second)) {
    oss << "(" << contracted_.at(first) << "," << contracted_.at(second) << ")";
    contracted_.erase(second);
  } else if (contracted_.contains(first)) {
    oss << "(" << contracted_.at(first) << "," << second << ")";
  } else if (contracted_.contains(second)) {
    oss << "(" << first << "," << contracted_.at(second) << ")";
    contracted_.erase(second);
  } else {
    oss << "(" << first << "," << second << ")";
  }
  excluded_leafs_.insert(second);
  contracted_.insert_or_assign(first, oss.str());
}
