/// @file node.h
/// @brief Common framework with nodes.
/// A common framework for nodes. That is an object having two children, value,
/// type and possibly parent.
#ifndef node_h_
#define node_h_

#include "utils.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_map>
#include <vector>

/// Type of nodes, either LEAF or INTERNAL.
enum NodeType {
  LEAF,
  INTERNAL,
};

/// Basic Node class.
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
  /// @param other The other node from which we copy.
  Node(const Node &other);
  /// (Deep) Copy assignement.
  /// @param other The other node from which we copy.
  /// @return Reference to the copied node.
  Node &operator=(const Node &other);
  /// Move constructor.
  /// @param other The other node from which we move.
  Node(Node &&other) = default;
  /// Move assignement.
  /// @param other The other node from which we move.
  /// @return Reference to the moved node.
  Node &operator=(Node &&other) = default;
  /// Destructor. All pointers are unique_ptr hence no need to specific
  /// deletion.
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
  std::array<int, 3>
  compute_forks(std::vector<Fork> &forks,
                std::vector<ExtendedFork> &extended_forks) const;
  /// Compute both LCA tables and pointers to all leafs.
  /// @param lca_table LCA table for all pairs.
  /// @return A map of pointers to all leafs.
  std::unordered_map<int, Node *>
  compute_lca_leafs(std::vector<std::vector<int>> &lca_table);
  /// Get a pointer to the left descendant.
  /// @return Pointer (monitor) to its left descendant.
  inline Node *get_left() const { return left_.get(); }
  /// Get a pointer to the parent.
  /// @return Raw pointer to the parent.
  inline Node *get_parent() const { return parent_; }
  /// Get a pointer to the right descendant.
  /// @return Pointer (monitor) to its right descendant.
  inline Node *get_right() const { return right_.get(); }
  /// Get current node type.
  /// @return Current node type.
  inline NodeType get_type() const { return type_; }
  /// Get the value stored in this node.
  /// @return Its stored value.
  inline int get_value() const { return value_; }
  /// Get all leafs underneath this node.
  /// @param taxa Where to store found leafs.
  void get_leafs(std::set<int> &taxa) const;
  /// Return whether a node is a leaf or not.
  /// @return If the node is leaf or not.
  inline bool is_leaf() const { return type_ == LEAF; }
  /// Remove the left descendant.
  /// @return Its left descendant which was moved.
  std::unique_ptr<Node> remove_left();
  /// Remove the right descendant.
  /// @return Its right descendant which was moved.
  std::unique_ptr<Node> remove_right();
  /// Set new left child.
  /// @param node New left child.
  void set_left(std::unique_ptr<Node> node);
  /// Set the pointer to its parent.
  /// @param parent Pointer to the parent.
  inline void set_parent(Node *parent) { parent_ = parent; }
  /// Set new right child.
  /// @param node New right child.
  void set_right(std::unique_ptr<Node> node);
  /// Set type of current node.
  /// @param type Its new type.
  inline void set_type(NodeType type) { type_ = type; }
  /// Set value of current node.
  /// @param value Its new value.
  inline void set_value(int value) { value_ = value; }
  /// Write the node and its subtree with possible substitutions. Does not break
  /// on empty subtrees.
  /// @param os Which output stream to use.
  /// @param subst List of substitutions.
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

/// Compare two nodes based on their values.
/// @param lhs Left hand side.
/// @param rhs Right hand side.
/// @return If the values of both nodes are the same.
inline bool operator==(Node &lhs, Node &rhs) {
  return lhs.get_value() == rhs.get_value();
}

/// Compare two nodes based on their values.
/// @param lhs Left hand side.
/// @param rhs Right hand side.
/// @return If the values of both nodes are not the same.
inline bool operator!=(Node &lhs, Node &rhs) { return !(lhs == rhs); }
#endif
