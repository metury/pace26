#include "node.h"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
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
  type_ = INTERNAL;
  left_ = std::make_unique<Node>(this);
  return left_.get();
}

Node *Node::add_right() {
  type_ = INTERNAL;
  right_ = std::make_unique<Node>(this);
  return right_.get();
}

int Node::assign_numbers(int counter) {
  if (type_ != LEAF) {
    value_ = counter;
    auto next = left_->assign_numbers(counter + 1);
    next = right_->assign_numbers(next);
    return next;
  }
  return counter;
}

void Node::consolidate() {
  if (type_ != LEAF) {
    if (left_) {
      left_->consolidate();
    }
    if (right_) {
      right_->consolidate();
    }
    if (!left_ && !right_) {
      if (parent_ != nullptr) {
        if (parent_->get_left() == this) {
          parent_->remove_left();
        } else {
          parent_->remove_right();
        }
      }
    } else if (right_ && !left_) {
      if (parent_ != nullptr) {
        if (parent_->get_left() == this) {
          parent_->set_left(remove_right());
        } else {
          parent_->set_right(remove_right());
        }
      }
    } else if (left_ && !right_) {
      if (parent_ != nullptr) {
        if (parent_->get_left() == this) {
          parent_->set_left(remove_left());
        } else {
          parent_->set_right(remove_left());
        }
      }
    }
  }
}

std::unordered_map<int, Node *>
Node::compute_lca_leafs(std::vector<std::vector<int>> &lca_table) {
  if (type_ == LEAF) {
    std::unordered_map<int, Node *> descendants;
    descendants.insert_or_assign(value_, this);
    return descendants;
  }
  auto left = left_->compute_lca_leafs(lca_table);
  auto right = right_->compute_lca_leafs(lca_table);
  for (auto &&[first, first_node] : left) {
    lca_table[first][value_] = lca_table[value_][first] = this->get_value();
    for (auto &&[second, second_node] : right) {
      lca_table[first][second] = lca_table[second][first] = this->get_value();
      lca_table[second][value_] = lca_table[value_][second] = this->get_value();
    }
  }
  left.insert_or_assign(this->get_value(), this);
  left.merge(right);
  return left;
}

std::unique_ptr<Node> Node::remove_left() {
  left_->set_parent(nullptr);
  return std::move(left_);
}

std::unique_ptr<Node> Node::remove_right() {
  right_->set_parent(nullptr);
  return std::move(right_);
}

void Node::set_left(std::unique_ptr<Node> node) {
  left_ = std::move(node);
  left_->set_parent(this);
}

void Node::set_right(std::unique_ptr<Node> node) {
  right_ = std::move(node);
  right_->set_parent(this);
}

void Node::write_with_substitution(
    std::ostream &os, const std::unordered_map<int, std::string> &subst) const {
  if (get_type() == LEAF) {
    if (subst.find(get_value()) != subst.end()) {
      os << subst.at(get_value());
    } else {
      os << get_value();
    }
  } else {
    os << "(";
    if (get_left() != nullptr) {
      get_left()->write_with_substitution(os, subst);
    }
    os << ",";
    if (get_right() != nullptr) {
      get_right()->write_with_substitution(os, subst);
    }
    os << ")";
  }
}

// ======================
// == Node - operators ==
// ======================

std::ostream &operator<<(std::ostream &os, const Node &n) {
  n.write_with_substitution(os, std::unordered_map<int, std::string>{});
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
