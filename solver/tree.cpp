#include "tree.h"
#include "utils.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// ======================================================================
// == Tree                                                             ==
// ======================================================================

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

void Tree::contract_cherry(uint16_t first, uint16_t second) {
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

void Tree::compute_lca_leafs(uint16_t number_of_nodes) {
  lca_table_ = std::vector<std::vector<uint16_t>>(
      number_of_nodes, std::vector<uint16_t>(number_of_nodes, 1));
  descendants_ = root_->compute_lca_leafs(lca_table_);
}

void Tree::get_edges(Node *below, Node *above,
                     std::set<uint16_t> &edges) const {
  auto current = below;
  while (current->get_value() != above->get_value() &&
         current->get_parent() != nullptr) {
    edges.insert(current->get_value() - 1);
    current = current->get_parent();
  }
}

std::set<uint16_t> Tree::get_leafs() const {
  std::set<uint16_t> leafs;
  root_->get_leafs(leafs);
  return leafs;
}

std::set<uint16_t> Tree::get_trio_edges(const Trio &trio) const {
  std::set<uint16_t> edges;
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

std::set<uint16_t> Tree::get_quartet_edges(const Quartet &quartet) const {
  std::set<uint16_t> edges;
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

bool Tree::has_disjoint_paths(uint16_t a, uint16_t b, uint16_t x,
                              uint16_t y) const {
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

bool Tree::has_disjoint_trio(uint16_t a, uint16_t b, uint16_t x) const {
  return lca_query(a, b) != lca_query(a, b, x);
}

bool Tree::is_cherry(uint16_t first, uint16_t second) const {
  auto node_a = descendants_.at(first);
  auto node_b = descendants_.at(second);
  return node_a->get_parent() != nullptr &&
         *node_a->get_parent() == *node_b->get_parent();
}

Node *Tree::lca_query(uint16_t first, uint16_t second) const {
  return descendants_.at(lca_table_[first][second]);
}

Node *Tree::lca_query(uint16_t first, uint16_t second, uint16_t third) const {
  auto first_second = lca_table_[first][second];
  return descendants_.at(lca_table_[first_second][third]);
}

void Tree::write(std::ostream &os,
                 const std::unordered_map<uint16_t, std::string> &subst) const {
  get_root()->write_with_substitution(os, subst);
  os << ";" << std::endl;
}

std::istream &operator>>(std::istream &is, Tree &t) {
  is >> *(t.get_root());
  return is;
}

// ======================================================================
// == Input                                                            ==
// ======================================================================

Input::Input(std::istream &is) : chosen_tree_(0) {
  std::string line;
  while (getline(is, line)) {
    if (line.size() > 0 && line[0] == '#') {
      if (line.size() > 1 && line[1] == 'p') {
        auto tokens = split(line);
        t_ = std::stoi(tokens[1]);
        n_ = std::stoi(tokens[2]);
      } else if (line.size() > 1 && line[1] == 'x') {
        // Do not read tree decomposition!
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

  auto lowest_depth = n_;
  for (auto i = 0; i < trees_.size(); ++i) {
    auto current_depth = trees_[i]->get_depth();
    if (lowest_depth > current_depth) {
      lowest_depth = current_depth;
      chosen_tree_ = i;
    }
  }
  std::cout << "# Chosen tree with number: " << GREEN << chosen_tree_ + 1
            << RESET << std::endl;
}

void Input::assign_numbers() {
  for (int i = 0; i < t_; ++i) {
    trees_[i]->assign_numbers(1, n_);
  }
}

void Input::compute_lca_tables() {
  for (auto &&tree : trees_) {
    tree->compute_lca_leafs(get_node_count() + 1);
  }
}

void Input::compute_trios_quartets(std::vector<Trio> &trios,
                                   std::vector<Quartet> &quartets) {
  auto tree1 = get_chosen_tree();
  for (uint16_t a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.contains(a))
      continue;
    for (uint16_t b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.contains(b))
        continue;
      for (uint16_t c = 1; c <= get_leaf_count(); ++c) {
        if (excluded_leafs_.contains(c) || c == b || c == a)
          continue;
        if (tree1->has_disjoint_trio(a, b, c)) {
          for (auto &&tree2 : get_other_trees()) {
            if (!tree2->has_disjoint_trio(a, b, c)) {
              Trio trio = {a, b, c, 0};
              trio.size = tree1->get_trio_edges(trio).size();
              trios.push_back(trio);
              break;
            }
          }
        }
        if (c >= a + 1) {
          for (uint16_t d = c + 1; d <= get_leaf_count(); ++d) {
            if (d == a || d == b || excluded_leafs_.contains(d))
              continue;
            if (tree1->has_disjoint_paths(a, b, c, d)) {
              for (auto &&tree2 : get_other_trees()) {
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

void Input::compute_breakable_forks(std::vector<Fork> &forks) const {
  get_chosen_tree()->get_root()->compute_forks(forks);
}

void Input::contract_cherries() {
  contract_cherries_recursive_(get_chosen_tree()->get_root());
  compute_lca_tables();
  for (auto &&[key, val] : contracted_) {
    std::cout << "# Replacing cherry: " << RED << key << RESET << " <- " << RED
              << val << RESET << std::endl;
  }
  if (contracted_.empty()) {
    std::cout << "# No " << RED << "cherry" << RESET << " was found."
              << std::endl;
  }
  std::cout << "# Number of leafs reduced by " << RED << excluded_leafs_.size()
            << RESET << std::endl;
  auto lowest_depth = n_;
  for (auto i = 0; i < trees_.size(); ++i) {
    auto current_depth = trees_[i]->get_depth();
    if (lowest_depth > current_depth) {
      lowest_depth = current_depth;
      chosen_tree_ = i;
    }
  }
  std::cout << "# Chosen tree with number: " << GREEN << chosen_tree_ + 1
            << RESET << std::endl;
}

void Input::contract_cherries_recursive_(Node *n) {
  if (n->is_leaf()) {
    return;
  }
  // First contract all possbile descendants.
  contract_cherries_recursive_(n->get_left());
  contract_cherries_recursive_(n->get_right());
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

std::vector<std::unique_ptr<Tree>>
Input::cut_edges(const std::set<uint16_t> &edges_to_remove) {
  std::vector<std::unique_ptr<Node>> output;
  std::vector<std::unique_ptr<Tree>> trees;
  if (trees_.size() == 0) {
    return trees;
  }
  Node tree(*get_chosen_tree()->get_root());
  output.push_back(std::make_unique<Node>(tree));
  cut_edges_recursive_(edges_to_remove, output, output.at(0).get());
  for (auto &&tree : output) {
    trees.push_back(std::make_unique<Tree>(std::move(tree)));
  }
  for (auto &&tree : trees) {
    tree->consolidate();
  }
  return trees;
}

void Input::cut_edges_recursive_(const std::set<uint16_t> &edges_to_remove,
                                 std::vector<std::unique_ptr<Node>> &trees,
                                 Node *current_tree) {
  if (current_tree->is_leaf()) {
    return;
  }
  auto left_val = current_tree->get_left()->get_value();
  if (edges_to_remove.contains(left_val)) {
    trees.push_back(std::move(current_tree->remove_left()));
    cut_edges_recursive_(edges_to_remove, trees,
                         trees.at(trees.size() - 1).get());
  } else {
    cut_edges_recursive_(edges_to_remove, trees, current_tree->get_left());
  }
  auto right_val = current_tree->get_right()->get_value();
  if (edges_to_remove.contains(right_val)) {
    trees.push_back(current_tree->remove_right());
    cut_edges_recursive_(edges_to_remove, trees,
                         trees.at(trees.size() - 1).get());
  } else {
    cut_edges_recursive_(edges_to_remove, trees, current_tree->get_right());
  }
}

void Input::add_contracted_(uint16_t first, uint16_t second) {
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
