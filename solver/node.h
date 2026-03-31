#ifndef node_h_
#define node_h_

#include "utils.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <unordered_map>

class Node;
using lca = std::unordered_map<LCA_KEY, Node *>;

/// Type of nodes, either LEAF or INTERNAL.
enum NodeType {
  LEAF,
  INTERNAL,
};

/// Node class for trees.
class Node {
public:
  /// Constructor. Set 0 value, LEAF and no null pointers.
  inline Node() : type_(LEAF), value_(0), parent_(nullptr) {}
  /// Constructor with setting parent.
  /// @param parent Pointer to its parent.
  inline Node(Node *parent) : type_(LEAF), value_(0), parent_(parent) {}
  /// Constructor with known value.
  /// @param value Its initial value.
  inline Node(int value) : type_(LEAF), value_(value), parent_(nullptr) {}
  /// Constructor with setting parent and value.
  /// @param parent Pointer to its parent.
  /// @param value Its initial value.
  inline Node(Node *parent, int value)
      : type_(LEAF), value_(value), parent_(parent) {}
  /// (Deep) Copy constructor.
  /// @param other Second node from which we copy.
  Node(const Node &other);
  /// (Deep) Copy assignement.
  /// @param other Second node from which we copy.
  Node &operator=(const Node &other);
  /// Move constructor.
  /// @param other Second node from which we move.
  Node(Node &&other) = default;
  /// Move assignement.
  /// @param other Second node from which we move.
  Node &operator=(Node &&other) = default;
  /// Destructor.
  ~Node() = default;
  /// Add left descendant and set type to LEAF.
  /// @return Pointer to the new descendant.
  Node *add_left();
  /// Add right descendant and set type to LEAF.
  /// @return Pointer to the new descendant.
  Node *add_right();
  /// Assign numbers to all nodes including INTERNAL nodes by a predefined way.
  /// @param counter Which counter we are starting with.
  int assign_numbers(int counter);
  /// Force iterative contraction of all 2 degree inner vertices.
  void consolidate();
  std::unordered_map<int, Node *> compute_lca_leafs(lca &pairs, lca &triples);
  /// Get pointer to the left descendant.
  /// @return Pointer (monitor) to its left descendant.
  inline Node *get_left() const { return left_.get(); }
  /// Get pointer to the parent.
  /// @return Pointer to the parent.
  inline Node *get_parent() const { return parent_; }
  /// Get pointer to the right descendant.
  /// @return Pointer (monitor) to its right descendant.
  inline Node *get_right() const { return right_.get(); }
  /// Get current node type.
  /// @return Current node type.
  inline NodeType get_type() const { return type_; }
  /// Get the value stored in this node.
  /// @return Its stored value.
  inline int get_value() const { return value_; }
  /// Remove the left descendant and also consolidate the tree.
  /// @return Its left descendant which was moved.
  std::unique_ptr<Node> remove_left();
  /// Remove the right descendant and also consolidate the tree.
  /// @return Its right descendant which was moved.
  std::unique_ptr<Node> remove_right();
  inline void set_left(std::unique_ptr<Node> node) {
    left_ = std::move(node);
    left_->set_parent(this);
  }
  inline void set_right(std::unique_ptr<Node> node) {
    right_ = std::move(node);
    right_->set_parent(this);
  }
  /// Set the pointer to its parent.
  /// @param parent Pointer to the parent.
  inline void set_parent(Node *parent) { parent_ = parent; }
  /// Set type of current node.
  /// @param type Its new type.
  inline void set_type(NodeType type) { type_ = type; }
  /// Set value of current node.
  /// @param value Its new value.
  inline void set_value(int value) { value_ = value; }

  void write_with_substitution(
      std::ostream &os,
      const std::unordered_map<int, std::string> &subst) const;

private:
  /// Left child.
  std::unique_ptr<Node> left_;
  /// Right child.
  std::unique_ptr<Node> right_;
  /// Monitor pointer to parent if exists.
  Node *parent_;
  /// Type of this node.
  NodeType type_;
  /// Value stored in this node.
  int value_;
};

/// Write current node in Newick format on an output stream by using `<<`.
/// @param os Which output stream to use.
/// @param n Which node to write.
/// @return Changed output stream.
std::ostream &operator<<(std::ostream &os, const Node &n);

/// Parse the node from input stream in Newick format using `>>`.
/// @param is Which input stream to use.
/// @param n Where to store the node.
/// @return Changed input stream.
std::istream &operator>>(std::istream &is, Node &n);

inline bool operator==(Node &lhs, Node &rhs) {
  return lhs.get_value() == rhs.get_value();
}

inline bool operator!=(Node &lhs, Node &rhs) { return !(lhs == rhs); }
#endif
