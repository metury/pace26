#include "tree.h"
#include "utils.h"
#include <cstddef>
#include <fstream>
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

void Tree::assign_numbers(int i, int n) {
  root_->assign_numbers(i * (n - 1) + 2);
}

void Tree::consolidate() {
  root_->consolidate();
  while (root_ != std::nullptr_t() && root_->get_type() != LEAF &&
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
      node_a->get_parent() == node_b->get_parent()) {
    auto parent = node_a->get_parent();
    parent->set_value(first);
    parent->set_type(LEAF);
    parent->remove_left();
    parent->remove_right();
  }
}

void Tree::compute_lca_leafs() {
  descendants_ = root_->compute_lca_leafs(pairs_, triples_);
}

void Tree::get_edges(Node *below, Node *above, std::set<int> &edges) const {
  auto current = below;
  while (current->get_value() != above->get_value() &&
         current->get_parent() != nullptr) {
    edges.insert(current->get_value());
    current = current->get_parent();
  }
}

Node *Tree::lca_query(int first, int second) {
  return pairs_[get_lca_key(first, second)];
}

Node *Tree::lca_query(int first, int second, int third) {
  return triples_[get_lca_key(first, second, third)];
}

bool Tree::is_cherry(int first, int second) const {
  auto node_a = descendants_.at(first);
  auto node_b = descendants_.at(second);
  return node_a->get_parent() != nullptr &&
         node_a->get_parent() == node_b->get_parent();
}

bool Tree::is_empty() const { return root_ == std::nullptr_t(); }

void Tree::write(std::ostream &os,
                 const std::unordered_map<int, std::string> &subst) const {
  get_root()->write_with_substitution(os, subst);
  os << ";" << std::endl;
}

std::istream &operator>>(std::istream &is, Tree &t) {
  is >> *(t.get_root());
  return is;
}

// ===========
// == Input ==
// ===========

Input::Input(const std::string &file_path) {
  std::ifstream ifs(file_path);
  std::string line;
  while (getline(ifs, line)) {
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
    } else {
      trees_.push_back(std::make_unique<Tree>());
      std::istringstream iss(line);
      iss >> *(trees_.at(trees_.size() - 1).get());
    }
  }
  ifs.close();
  assign_numbers();
  for (auto &&tree : trees_) {
    tree->compute_lca_leafs();
  }
}

void Input::assign_numbers() {
  for (int i = 0; i < t_; ++i) {
    trees_[i]->assign_numbers(i + 1, n_);
  }
}

void Input::set_tree_decomposition(const std::string &str) {
  decomp_ = TreeDecomposition(str);
}

// This needs to be redone! Compute all wrong triples.
std::vector<std::tuple<int, int, int>> Input::compute_trios() {
  std::vector<std::tuple<int, int, int>> trios;
  auto tree1 = trees_[0].get();
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.contains(a))
      continue;
    for (auto b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.contains(b))
        continue;
      auto node1_a_b = tree1->lca_query(a, b);
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (excluded_leafs_.contains(c))
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

void Input::contract_cherries() {
  while (contract_cherries_(trees_[0]->get_root())) {
  };
  for (auto &&[key, val] : contracted_) {
    std::cout << "# Replacing cherry: " << RED << key << RESET << " <- " << RED
              << val << RESET << std::endl;
  }
}

bool Input::contract_cherries_(Node *n) {
  if (n->get_type() == LEAF) {
    return false;
  }
  auto left = n->get_left();
  auto right = n->get_right();
  if (left->get_type() == LEAF && right->get_type() == LEAF) {
    for (auto &&tree : trees_) {
      if (!tree->is_cherry(left->get_value(), right->get_value())) {
        return false;
      }
    }
    for (auto &&tree : trees_) {
      tree->contract_cherry(left->get_value(), right->get_value());
      tree->compute_lca_leafs();
    }
    add_contracted_(left->get_value(), right->get_value());
    return true;
  }
  return contract_cherries_(left) || contract_cherries_(right);
}

std::vector<std::tuple<int, int, int, int>> Input::compute_quartets() {
  std::vector<std::tuple<int, int, int, int>> quartets;
  auto tree1 = trees_[0].get();
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    if (excluded_leafs_.contains(a))
      continue;
    for (auto b = a + 1; b <= get_leaf_count(); ++b) {
      if (excluded_leafs_.contains(b))
        continue;
      auto node1_a_b = tree1->lca_query(a, b);
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (c == b || c == a || excluded_leafs_.contains(c))
          continue;
        for (auto d = c + 1; d <= get_leaf_count(); ++d) {
          if (d == c || d == a || d == b || excluded_leafs_.contains(d))
            continue;
          auto node1_ab_c = tree1->lca_query(a, b, c);
          auto node1_ab_d = tree1->lca_query(a, b, d);
          auto c_below_ab_1 = node1_a_b == node1_ab_c;
          auto d_below_ab_1 = node1_a_b == node1_ab_d;
          if (!c_below_ab_1 && !d_below_ab_1) {
            for (auto &&tree2 : get_trees()) {
              if (tree1->get_root()->get_value() ==
                  tree2->get_root()->get_value())
                continue;
              auto node2_a_b = tree2->lca_query(a, b);
              auto node2_ab_c = tree2->lca_query(a, b, c);
              auto node2_ab_d = tree2->lca_query(a, b, d);
              // Either one is below and the other match or the other way
              // around.
              auto c_below_ab_2 = node2_a_b == node2_ab_c;
              auto d_below_ab_2 = node2_a_b == node2_ab_d;
              if (c_below_ab_2 || d_below_ab_2) {
                // We found triplet.
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
  if (current_tree->get_type() == LEAF) {
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
