#include "tree.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

// ==========
// == Node ==
// ==========

Node::Node() : type_(LEAF), value_(0), parent_(nullptr) {}

Node::Node(Node *parent) : type_(LEAF), value_(0), parent_(parent) {}

Node::Node(int value) : type_(LEAF), value_(value), parent_(nullptr) {}

Node::Node(Node *parent, int value)
    : type_(LEAF), value_(value), parent_(parent) {}

Node::Node(const Node &other)
    : type_(other.get_type()), value_(other.get_value()) {
  if (type_ == INTERNAL) {
    left_ = std::make_unique<Node>(*other.get_left());
    right_ = std::make_unique<Node>(*other.get_right());
    left_.get()->parent_ = this;
    right_.get()->parent_ = this;
  }
}

Node &Node::operator=(const Node &other) {
  type_ = other.get_type();
  value_ = other.get_value();
  left_ = std::make_unique<Node>(*other.get_left());
  right_ = std::make_unique<Node>(*other.get_right());
  left_.get()->parent_ = this;
  right_.get()->parent_ = this;
  return *this;
}

Node *Node::add_left() {
  type_ = INTERNAL;
  left_ = std::make_unique<Node>(this);
  return left_.get();
}

Node *Node::add_right() {
  type_ = INTERNAL;
  right_ = std::make_unique<Node>(this);
  return right_.get();
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

void Node::set_value(int value) { value_ = value; }

int Node::get_value() const { return value_; }

NodeType Node::get_type() const { return type_; }

Node *Node::get_left() const { return left_.get(); }

Node *Node::get_right() const { return right_.get(); }

Node *Node::get_root() {
  auto root = this;
  while (root->parent_ != nullptr) {
    root = root->parent_;
  }
  return root;
}

NodeType Node::change_type() {
  type_ = type_ == LEAF ? INTERNAL : LEAF;
  return type_;
}

std::unique_ptr<Node> Node::remove_left() {
  auto tmp = std::move(left_);
  get_root()->consolidate();
  return std::move(tmp);
}

std::unique_ptr<Node> Node::remove_right() {
  auto tmp = std::move(right_);
  get_root()->consolidate();
  return std::move(tmp);
}

//! Remove vertices with no descendats.
void Node::consolidate() {
  if (type_ == INTERNAL) {
    //! Test this consolidation.
    if (left_ == nullptr && right_ == nullptr) {
      if (parent_ != nullptr) {
        if (parent_->get_left() == this) {
          parent_->remove_left();
        } else {
          parent_->remove_right();
        }
        parent_->consolidate();
      }
    }
    if (left_ == nullptr) {
      left_ = std::move(right_->left_);
      right_ = std::move(right_->right_);
      consolidate();
    } else if (right_ == nullptr) {
      left_ = std::move(left_->left_);
      right_ = std::move(left_->right_);
      consolidate();
    } else {
      left_->consolidate();
      right_->consolidate();
    }
  }
}

Node Node::add_root_leaf() {
  auto root = get_root();
  auto new_root = Node();
  new_root.set_left(std::move(*root));
  new_root.set_right(Node(&new_root, ROOT_LABEL));
  return new_root;
}

void Node::write(std::ostream &os) const { os << *this << ";" << std::endl; }

void Node::assign_numbers(int i, int n) {
  auto root = get_root();
  if (root != this) {
    root->assign_numbers(i, n);
  } else {
    assign_numbers_(i * (n + 1));
  }
}

void Node::sort() { get_root()->sort_by_swaps_(); }

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

int Node::assign_numbers_(int counter) {
  if (type_ == INTERNAL) {
    value_ = counter;
    auto next = left_->assign_numbers_(counter + 1);
    next = right_->assign_numbers_(next);
    return next;
  }
  return counter;
}

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

// ===========
// == Input ==
// ===========

Input::Input() : t_(0), n_(0), trees_() {}

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
      trees_.push_back(std::move(tree));
    }
  }
  ifs.close();
}

std::vector<Node> &Input::get_trees() { return trees_; }

int Input::get_leaf_count() const { return n_; }

int Input::get_tree_count() const { return t_; }

void Input::assign_numbers() {
  for (int i = 0; i < t_; ++i) {
    trees_[i].assign_numbers(i + 1, n_);
  }
}

void Input::set_tree_decomposition(const std::string &str) {
  decomp_ = TreeDecomposition(str);
}

TreeDecomposition &Input::get_tree_decomposition() { return decomp_; }

bool Input::are_identical() {
  trees_[0].sort();
  for (int i = 1; i < trees_.size(); ++i) {
    trees_[i].sort();
    if (trees_[i - 1] != trees_[i]) {
      std::cout << trees_[i - 1] << trees_[i] << std::endl;
      return false;
    }
  }
  return true;
}

void Input::add_root_leafs() {
  for (auto i = 0; i < trees_.size(); ++i) {
    trees_[i] = trees_[i].add_root_leaf();
  }
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
