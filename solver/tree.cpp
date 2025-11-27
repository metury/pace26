#include "tree.h"
#include <iostream>

Node::Node() : type_(LEAF), value_(0), parent_(nullptr) {}

Node::Node(Node* parent) : type_(LEAF), value_(0), parent_(parent) {}

Node* Node::addLeft() {
  left_ = std::make_unique<Node>(this);
  return left_.get();
}

Node* Node::addRight() {
  right_ = std::make_unique<Node>(this);
  return right_.get();
}

void Node::setValue(int value) {
  value_ = value;
}

int Node::getValue() const {
  return value_;
}

NodeType Node::getType() const {
  return type_;
}

Node* Node::getLeft() const {
  return left_.get();
}

Node* Node::getRight() const {
  return right_.get();
}

NodeType Node::changeType() {
  type_ = type_ == LEAF ? type_ = INNER : type_ = LEAF;
  return type_;
}

std::unique_ptr<Node> Node::removeLeft() {
  return std::move(left_);
}

std::unique_ptr<Node> Node::removeRight() {
  return std::move(right_);
}

void Node::consolidate() {
  if(type_ == INNER) {
    if(left_ == nullptr) {
      left_ = std::move(right_->left_);
      right_ = std::move(right_->right_);
      consolidate();
    } else if(right_ == nullptr) {
      left_ = std::move(left_->left_);
      right_ = std::move(left_->right_);
    consolidate();
    } else {
      left_->consolidate();
      right_->consolidate();
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Node& n) {
  if (n.getType() == LEAF) {
    os << n.getValue();
  } else {
    os << "(" << *(n.getLeft()) << "," << *(n.getRight()) << ")";
  }
  return os;
}

std::istream& operator>>(std::istream& is, Node& n) {
    if (is.eof()) return is;
    char next_char = is.peek();
    if (next_char == '(') {
        n.changeType();
        char delimiter;
        is >> delimiter; 
        is >> *(n.addLeft());
        is >> delimiter;
        if (delimiter != ',') {
            is.setstate(std::ios::failbit);
            return is;
        }
        is >> *(n.addRight());
        is >> delimiter;
        if (delimiter != ')') {
            is.setstate(std::ios::failbit);
        } 
    } else {
      int value;
      is >> value;
      n.setValue(value);
    }
    return is;
}
