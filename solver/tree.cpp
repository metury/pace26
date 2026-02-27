#include "tree.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

// ==========
// == Node ==
// ==========

Node::Node(const Node &other)
    : type_(other.get_type()), value_(other.get_value()) {
  if (type_ != LEAF) {
    left_ = std::make_unique<Node>(*other.get_left());
    right_ = std::make_unique<Node>(*other.get_right());
    left_->set_parent(this);
    right_->set_parent(this);
  }
}

Node &Node::operator=(const Node &other) {
  type_ = other.get_type();
  value_ = other.get_value();
  left_ = std::make_unique<Node>(*other.get_left());
  right_ = std::make_unique<Node>(*other.get_right());
  left_->set_parent(this);
  right_->set_parent(this);
  return *this;
}

Node *Node::add_left() {
  if (type_ == LEAF)
    type_ = INTERNAL;
  left_ = std::make_unique<Node>(this);
  return left_.get();
}

Node *Node::add_right() {
  if (type_ == LEAF)
    type_ = INTERNAL;
  right_ = std::make_unique<Node>(this);
  return right_.get();
}

void Node::assign_numbers(int i, int n) {
  get_root()->assign_numbers_(i * (n - 1) + 2);
}

void Node::consolidate() { get_root()->consolidate_(); }

Node *Node::get_root() {
  auto root = this;
  while (root->get_parent() != nullptr)
    root = root->parent_;
  return root;
}

void Node::get_edges(Node &above, std::set<int> &edges) const {
  auto current = this;
  while (current->get_value() != above.get_value()) {
    edges.insert(current->get_value());
    current = current->get_parent();
  }
}

std::unique_ptr<Node> Node::remove_left() {
  auto tmp = std::move(left_);
  consolidate();
  return std::move(tmp);
}

std::unique_ptr<Node> Node::remove_right() {
  auto tmp = std::move(right_);
  consolidate();
  return std::move(tmp);
}

void Node::set_left(Node n) {
  left_ = std::make_unique<Node>(n);
  type_ = INTERNAL;
  n.parent_ = this;
}

void Node::set_right(Node n) {
  right_ = std::make_unique<Node>(n);
  type_ = INTERNAL;
  n.parent_ = this;
}

void Node::set_parent(Node *parent) { parent_ = parent; }

void Node::set_value(int value) { value_ = value; }

void Node::sort() { get_root()->sort_by_swaps_(); }

void Node::write(std::ostream &os) const { os << *this << ";" << std::endl; }

// ====================
// == Node - private ==
// ====================

int Node::assign_numbers_(int counter) {
  if (type_ != LEAF) {
    value_ = counter;
    auto next = left_->assign_numbers_(counter + 1);
    next = right_->assign_numbers_(next);
    return next;
  }
  return counter;
}

void Node::consolidate_() {
  if (type_ != LEAF) {
    //! Test this consolidation.
    if (left_ == nullptr && right_ == nullptr) {
      if (parent_ != nullptr) {
        if (parent_->get_left() == this) {
          parent_->remove_left();
        } else {
          parent_->remove_right();
        }
        parent_->consolidate_();
      }
    }
    if (left_ == nullptr) {
      left_ = std::move(right_->left_);
      right_ = std::move(right_->right_);
      consolidate_();
    } else if (right_ == nullptr) {
      left_ = std::move(left_->left_);
      right_ = std::move(left_->right_);
      consolidate_();
    } else {
      left_->consolidate_();
      right_->consolidate_();
    }
  }
}

std::tuple<int, int> Node::sort_by_swaps_() {
  if (type_ == LEAF) {
    return {value_, value_};
  } else {
    auto left = left_->sort_by_swaps_();
    auto right = right_->sort_by_swaps_();
    if (std::get<1>(right) < std::get<1>(left) ||
        (std::get<1>(right) == std::get<1>(left) &&
         std::get<0>(right) < std::get<0>(left))) {
      std::swap(left_, right_);
    }
    return {std::get<0>(left) < std::get<0>(right) ? std::get<0>(left)
                                                   : std::get<0>(right),
            std::get<1>(left) + std::get<1>(right)};
  }
}

void Node::updated_descendants_() {
  if (type_ == LEAF) {
    descendants_.clear();
    descendants_.insert_or_assign(value_, this);
    return;
  }
  left_->updated_descendants_();
  right_->updated_descendants_();
  descendants_.clear();
  descendants_.insert(left_->get_descendants().begin(),
                      left_->get_descendants().end());
  descendants_.insert(right_->get_descendants().begin(),
                      right_->get_descendants().end());
}

// ======================
// == Node - operators ==
// ======================

std::ostream &operator<<(std::ostream &os, const Node &n) {
  if (n.get_type() == LEAF) {
    os << n.get_value();
  } else {
    os << "(" << *(n.get_left()) << "," << *(n.get_right()) << ")";
  }
  return os;
}

std::istream &operator>>(std::istream &is, Node &n) {
  if (is.eof())
    return is;
  char next_char = is.peek();
  if (next_char == '(') {
    char delimiter;
    is >> delimiter;
    is >> *(n.add_left());
    is >> delimiter;
    if (delimiter != ',') {
      is.setstate(std::ios::failbit);
      return is;
    }
    is >> *(n.add_right());
    is >> delimiter;
    if (delimiter != ')') {
      is.setstate(std::ios::failbit);
    }
  } else {
    int value;
    is >> value;
    n.set_value(value);
  }
  return is;
}

bool operator==(Node &lhs, Node &rhs) {
  // Simplest way to compare if two trees are exactly the same.
  // But it probably underperforms.
  std::ostringstream los;
  std::ostringstream ros;
  lhs.sort();
  rhs.sort();
  lhs.write(los);
  rhs.write(ros);
  return los.str() == ros.str();
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
      Node tree;
      std::istringstream iss(line);
      iss >> tree;
      trees_.push_back(std::make_unique<Node>(tree));
    }
  }
  ifs.close();
}

void Input::assign_numbers() {
  for (int i = 0; i < t_; ++i) {
    trees_[i]->assign_numbers(i + 1, n_);
  }
}

void Input::set_tree_decomposition(const std::string &str) {
  decomp_ = TreeDecomposition(str);
}

bool Input::are_identical() {
  trees_[0]->sort();
  for (int i = 1; i < trees_.size(); ++i) {
    trees_[i]->sort();
    if (trees_[i - 1] != trees_[i]) {
      std::cout << trees_[i - 1] << trees_[i] << std::endl;
      return false;
    }
  }
  return true;
}

void Input::compute_all_lca() {
  for (auto &&tree : trees_) {
    auto lca = LCA();
    tree->updated_descendants();
    lca.compute(tree.get());
    lcas_.push_back(lca);
  }
}

// This needs to be redone! Compute all wrong triples.
std::vector<std::tuple<int, int, int>> Input::compute_trios() {
  std::vector<std::tuple<int, int, int>> trios;
  auto tree1 = trees_[0].get();
  auto lca1 = lcas_[0];
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    for (auto b = 1; b <= get_leaf_count(); ++b) {
      if (a == b)
        continue;
      auto [lca1_a_b, node1_a_b] = lca1.query(a, b);
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (c == b || c == a)
          continue;
        auto [lca1_ab_c, node1_ab_c] = lca1.query(a, b, c);
        // Either one is below and the other match or the other way around.
        auto c_below_ab_1 = lca1_a_b == lca1_ab_c;
        if (!c_below_ab_1) {
          for (auto &&[tree2, lca2] : *this) {
            if (tree1->get_value() == tree2->get_value())
              continue;
            auto [lca2_a_b, node2_a_b] = lca2.query(a, b);
            auto [lca2_ab_c, node2_ab_c] = lca2.query(a, b, c);
            auto c_below_ab_2 = lca2_a_b == lca2_ab_c;
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
  auto lca1 = lcas_[0];
  for (auto a = 1; a <= get_leaf_count(); ++a) {
    for (auto b = 1; b <= get_leaf_count(); ++b) {
      if (a == b)
        continue;
      auto [lca1_a_b, node1_a_b] = lca1.query(a, b);
      for (auto c = 1; c <= get_leaf_count(); ++c) {
        if (c == b || c == a)
          continue;
        for (auto d = 1; d <= get_leaf_count(); ++d) {
          if (d == c || d == a || d == b)
            continue;
          auto [lca1_ab_c, node1_ab_c] = lca1.query(a, b, c);
          auto [lca1_ab_d, node1_ab_d] = lca1.query(a, b, d);
          auto c_below_ab_1 = lca1_a_b == lca1_ab_c;
          auto d_below_ab_1 = lca1_a_b == lca1_ab_d;
          if (!c_below_ab_1 && !d_below_ab_1) {
            for (auto &&[tree2, lca2] : *this) {
              if (tree1->get_value() == tree2->get_value())
                continue;
              auto [lca2_a_b, node2_a_b] = lca2.query(a, b);
              auto [lca2_ab_c, node2_ab_c] = lca2.query(a, b, c);
              auto [lca2_ab_d, node2_ab_d] = lca2.query(a, b, d);
              // Either one is below and the other match or the other way
              // around.
              auto c_below_ab_2 = lca2_a_b == lca2_ab_c;
              auto d_below_ab_2 = lca2_a_b == lca2_ab_d;
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

// =========
// == LCA ==
// =========

void LCA::compute(Node *node) {
  if (node->get_type() == LEAF)
    return;
  for (auto &&[first, f_n] : node->get_left()->get_descendants()) {
    for (auto &&[second, s_n] : node->get_right()->get_descendants()) {
      pairs_.insert_or_assign(LCA::get_name_(first, second),
                              std::make_pair(node->get_value(), node));
      for (auto &&[third, t_n] : node->get_right()->get_descendants()) {
        if (third > second)
          triples_.insert_or_assign(LCA::get_name_(first, second, third),
                                    std::make_pair(node->get_value(), node));
      }
      for (auto &&[third, t_n] : node->get_left()->get_descendants()) {
        if (third > first)
          triples_.insert_or_assign(LCA::get_name_(first, second, third),
                                    std::make_pair(node->get_value(), node));
      }
    }
  }
  compute(node->get_left());
  compute(node->get_right());
}

std::pair<int, Node *> LCA::query(int first, int second) const {
  return pairs_.at(LCA::get_name_(first, second));
}

std::pair<int, Node *> LCA::query(int first, int second, int third) const {
  return triples_.at(LCA::get_name_(first, second, third));
}

void LCA::write() const {
  for (auto &&[val, node] : pairs_) {
    std::cout << val << ": " << std::get<0>(node) << std::endl;
  }
  for (auto &&[val, node] : triples_) {
    std::cout << val << ": " << std::get<0>(node) << std::endl;
  }
}

// ===================
// == LCA - private ==
// ===================

std::string LCA::get_name_(int first, int second) const {
  std::ostringstream os;
  if (first > second) {
    std::swap(first, second);
  }
  os << first << "#" << second;
  return os.str();
}

std::string LCA::get_name_(int first, int second, int third) const {
  std::ostringstream os;
  if (second < first && second < third) {
    std::swap(first, second);
  } else if (third < first && third < second) {
    std::swap(first, third);
  }
  if (second > third) {
    std::swap(second, third);
  }
  os << first << "#" << second << "#" << third;
  return os.str();
}
