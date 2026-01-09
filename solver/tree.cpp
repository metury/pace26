#include "tree.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

Node::Node() : type_(LEAF), value_(0), parent_(nullptr) {}

Node::Node(Node* parent) : type_(LEAF), value_(0), parent_(parent) {}

Node::Node(const Node& other) : type_(other.getType()), value_(other.getValue()) {
  if(type_ == INNER) {
    left_ = std::make_unique<Node>(*other.getLeft());
    right_ = std::make_unique<Node>(*other.getRight());
    left_.get()->parent_ = this;
    right_.get()->parent_ = this;
  }
}

Node& Node::operator=(const Node& other) {
  type_ = other.getType();
  value_ = other.getValue();
  left_ = std::make_unique<Node>(*other.getLeft());
  right_ = std::make_unique<Node>(*other.getRight());
  left_.get()->parent_ = this;
  right_.get()->parent_ = this;
  return *this;
}

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

Node* Node::getRoot() {
  auto root = this;
  while(root->parent_ != nullptr) {
    root = root->parent_;
  }
  return root;
}

NodeType Node::changeType() {
  type_ = type_ == LEAF ? INNER : LEAF;
  return type_;
}

std::unique_ptr<Node> Node::removeLeft() {
  auto tmp = std::move(left_);
  getRoot()->consolidate();
  return std::move(tmp);
}

std::unique_ptr<Node> Node::removeRight() {
  auto tmp = std::move(right_);
  getRoot()->consolidate();
  return std::move(tmp);
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

void Node::write(std::ostream& os) const {
  os << *this << ";" << std::endl;
}

void Node::assignNumbers(int i, int n) {
  auto root = getRoot();
  if(root != this) {
    root->assignNumbers(i, n);
  }
  else {
    assignNumbers(i * (n+1));
  }
}

int Node::assignNumbers(int counter) {
  if(type_ == INNER) {
    value_ = counter;
    auto next = left_->assignNumbers(counter+1);
    next = right_->assignNumbers(next);
    return next;
  }
  return counter;
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

Input::Input() : t_(0), n_(0), trees_() {}

Input::Input(const std::string& filePath) {
  std::ifstream ifs(filePath);
  std::string line;
  while (getline (ifs, line)) {
    if(line.size() > 0 && line[0] == '#') {
      if(line.size() > 1 && line[1] == 'p') {
        auto tokens = split(line);
        t_ = std::stoi(tokens[1]);
        n_ = std::stoi(tokens[2]);
      } else if(line.size() > 1 && line[1] == 't') {
        //#t precomputed parameters
        //std::vector<std::string> parts = split(line, ' ');
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

std::vector<Node>& Input::getTrees() {
  return trees_;
}

int Input::getLeafCount() const {
  return n_;
}

int Input::getTreeCount() const {
  return t_;
}

void Input::assignNumbers() {
  for(int i = 0; i < t_; i++) {
    trees_[i].assignNumbers(i + 1, n_);
  }
}
