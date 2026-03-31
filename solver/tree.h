#ifndef tree_h_
#define tree_h_

#include "utils.h"
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

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
  /// Set the pointer to its parent.
  /// @param parent Pointer to the parent.
  inline void set_parent(Node *parent) { parent_ = parent; }
  /// Set type of current node.
  /// @param type Its new type.
  inline void set_type(NodeType type) { type_ = type; }
  /// Set value of current node.
  /// @param value Its new value.
  inline void set_value(int value) { value_ = value; }

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

class Tree {
public:
  inline Tree() : root_(std::make_unique<Node>()) {}
  /// Assign numbers to internal nodes.
  /// @param i Number of this tree.
  /// @param n Number of leafs.
  void assign_numbers(int i, int n);
  /// Force contractions and remove empty branches.
  void consolidate();
  void contract_cherry(int first, int second);
  /// Compute both leaf pointers and lca values.
  void compute_lca_leafs();
  Node *lca_query(int first, int second);
  Node *lca_query(int first, int second, int third);
  void get_edges(Node *below, Node *above, std::set<int> &edges) const;
  inline Node *get_leaf(int value) const { return descendants_.at(value); }
  /// Get pointer to the root node.
  /// @return Pointer to the root node.
  inline Node *get_root() const { return root_.get(); }
  bool is_cherry(int first, int second) const;
  /// Output the tree to some ostream in a Newick notation.
  /// @param os Which output stream to use.
  void write(std::ostream &os) const;
  /// Output the tree to standard outpu.
  inline void write() const { write(std::cout); };

private:
  /// Map of descendant. Also can be used as a set of descendants.
  std::unordered_map<int, Node *> descendants_;
  /// Rot of the tree.
  std::unique_ptr<Node> root_;
  lca pairs_;
  lca triples_;
};

/// Parse the tree from input stream in Newick format using `>>`.
/// @param is Which input stream to use.
/// @param n Where to store the node.
/// @return Changed input stream.
std::istream &operator>>(std::istream &is, Tree &t);

/// Tree decomposition of the display graph.
class TreeDecomposition {
public:
  /// Default constructor.
  TreeDecomposition() = default;
  /// Constructor from its string representation.
  /// @param str String representation in semi JSON format.
  TreeDecomposition(const std::string &str);
  /// Write the tree decomposition to ouptut. Debugging mainly.
  /// @param os Which output stream to use.
  void write(std::ostream &os);

private:
  /// Set of bags. IDs are for node ids in the trees.
  std::vector<std::vector<int>> bags_;
  /// Set of edges between bags. IDs are for bags in their ordering.
  std::vector<std::tuple<int, int>> edges_;
  /// Treewidth of the tree decomposition.
  int treewidth_;
};

/// All the input provided. Including trees and tree decomposition.
class Input {
public:
  /// Default constructor.
  inline Input() : t_(0), n_(0) {}
  /// Constructor for parsing input from a file.
  /// @param file_path Path to the file.
  Input(const std::string &file_path);
  ~Input() = default;
  /// Assign numbers to all trees.
  void assign_numbers();
  /// Compute all LCA values for all trees.
  void compute_all_lca();
  void contract_cherries();
  /// Get the leaf count, which is same for all trees.
  /// @return Leaf count.
  inline int get_leaf_count() const { return n_; }
  /// Get the tree count.
  /// @return Tree count.
  inline int get_tree_count() const { return t_; }
  /// Get reference to the tree decomposition.
  /// @return Tree decomposition.
  inline TreeDecomposition &get_tree_decomposition() { return decomp_; }
  /// Get reference to all trees.
  /// @return Reference to all trees.
  inline std::vector<std::unique_ptr<Tree>> &get_trees() { return trees_; }
  /// Set the tree decomposition by parsing its string representation.
  /// @param str Its string representation.
  void set_tree_decomposition(const std::string &str);

  std::vector<std::tuple<int, int, int>> compute_trios();
  std::vector<std::tuple<int, int, int, int>> compute_quartets();

private:
  void add_contracted_(int first, int second);
  bool contract_cherries_(Node *node);
  /// The tree decomposition.
  TreeDecomposition decomp_;
  /// Number of leafs in each tree.
  int n_;
  /// Number of trees.
  int t_;
  /// Array of all trees.
  std::vector<std::unique_ptr<Tree>> trees_;
  std::unordered_map<int, std::string> contracted_;
  std::set<int> excluded_leafs_;
};
#endif
