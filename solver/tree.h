#ifndef tree_h_
#define tree_h_

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

/// Type of nodes, either LEAF or INTERNAL.
enum NodeType {
  LEAF,
  INTERNAL,
};

/// Special label for root as an extra leaf.
const int ROOT_LABEL = -1; // It can be changed!

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
  /// Add values and pointers to the descendants, propagate upwards.
  /// @param value What is the value of the descendant.
  /// @param n Pointer to that descendant.
  void add_descendant(int value, Node *n);
  /// Add left descendant and set type to LEAF.
  /// @return Pointer to the new descendant.
  Node *add_left();
  /// Add right descendant and set type to LEAF.
  /// @return Pointer to the new descendant.
  Node *add_right();
  /// Add new leaf "rho" as a root.
  Node add_root_leaf();
  /// Assign numbers to all nodes including INTERNAL nodes by a predefined way.
  /// @param i Which tree is this.
  /// @param n How many leafs the tree has.
  void assign_numbers(int i, int n);
  /// Change the type of this node.
  /// @return New and changed type.
  NodeType change_type();
  /// Force iterative contraction of all 2 degree inner vertices.
  void consolidate();
  /// Get the map of descendants and its pointers.
  /// @return Reference to the map.
  inline const std::unordered_map<int, Node *> &get_descendants() const {
    return descendants_;
  }
  /// Get pointer to the left descendant.
  /// @return Pointer (monitor) to its left descendant.
  inline Node *get_left() const { return left_.get(); }
  /// Get pointer to the parent.
  /// @return Pointer to the parent.
  inline Node *get_parent() const { return parent_; }
  /// Get pointer to the right descendant.
  /// @return Pointer (monitor) to its right descendant.
  inline Node *get_right() const { return right_.get(); }
  /// Traverse the tree and return the root.
  /// @return Root of this tree.
  Node *get_root();
  /// Traverse the tree and return constant root pointer.
  /// @preturn Constant root of this tree.
  inline const Node *get_root() const { return get_root(); };
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
  /// Set left child to given node.
  /// @param n Given new left child.
  void set_left(Node n);
  /// Set right child to given node.
  /// @param n Given new right child.
  void set_right(Node n);
  /// Set value of current node.
  /// @param value Its new value.
  void set_value(int value);
  /// Pseudo sort the tree based on its leafs, left is smaller than right
  /// descendant. Lexicographically: first, sum of leaf labels and second,
  /// minimum label.
  void sort();
  /// Update all descendants in the tree.
  inline void updated_descendants() { get_root()->updated_descendants_(); }
  /// Output the tree to some ostream in a Newick notation.
  /// @param os Which output stream to use.
  void write(std::ostream &os) const;
  /// Output the tree to standard outpu.
  inline void write() const { write(std::cout); };

private:
  /// Assign numbers to INTERNAL nodes.
  /// @param counter What is the counter for this node.
  /// @return Next free number from the tree below.
  int assign_numbers_(int counter);
  /// Recursive call of consolidation.
  void consolidate_();
  /// Sort the tree by swapping descendants.
  /// @return Both minimal value and the sum.
  std::tuple<int, int> sort_by_swaps_();
  /// Recursive call for updating descendants.
  void updated_descendants_();
  /// Map of descendant. Also can be used as a set of descendants.
  std::unordered_map<int, Node *> descendants_;
  /// Left descendant.
  std::unique_ptr<Node> left_;
  /// Right descendant.
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

/// Compare two trees if they are exactly same.
/// @param lhs Tree on the left hand side.
/// @param rhs Tree on the right hand side.
/// @return True if the trees exactly match.
bool operator==(Node &lhs, Node &rhs);

/// Compare two trees if they are not exactly same.
/// @param lhs Tree on the left hand side.
/// @param rhs Tree on the right hand side.
/// @return False if the trees exactly match.
inline bool operator!=(Node &lhs, Node &rhs) { return !(lhs == rhs); }

/// Compare two nodes if the left hand side is above right hand side.
/// @param lhs Left hand side of the comparison.
/// @param rhs Right hand side of the comparison.
/// @return If rhs is below lhs.
inline bool operator<(Node &lhs, Node &rhs) {
  return lhs.get_descendants().contains(rhs.get_value());
}

/// Compare two nodes if the left hand side is below right hand side.
/// @param lhs Left hand side of the comparison.
/// @param rhs Right hand side of the comparison.
/// @return If rhs is above lhs.
inline bool operator>(Node &lhs, Node &rhs) { return rhs < lhs; }

/// Precomputing and quering LCA for leafs.
class LCA {
public:
  /// Default constructor.
  LCA() = default;
  /// Precomputing all pairs for leafs.
  /// @param node For which node we precompute recursively.
  void compute(Node *node);
  /// Return LCA for two leafs.
  /// @param first First leaf.
  /// @param second Second leaf.
  /// @return Value and node pointer to their LCA.
  std::pair<int, Node *> query(int first, int second) const;
  /// Return LCA for three leafs.
  /// @param first First leaf.
  /// @param second Second leaf.
  /// @param third Third leaf.
  /// @return Value and node pointer to their LCA.
  std::pair<int, Node *> query(int first, int second, int third) const;

  /// Write all the LCA pairs.
  inline void write() const {
    for (auto &&[val, node] : pairs_) {
      std::cout << val << ": " << std::get<0>(node) << std::endl;
    }
    for (auto &&[val, node] : triples_) {
      std::cout << val << ": " << std::get<0>(node) << std::endl;
    }
  }

private:
  /// Create a string key to the map as "first#second".
  /// @param first First value. Will be the lower.
  /// @param second Second value.
  /// @return Hash string key.
  std::string get_name_(int first, int second) const;
  /// Create a string key to the map as "first#second#third".
  /// @param first First value. Will be the lowest.
  /// @param second Second value.
  /// @param third Third value. Will be the highest.
  /// @return Hash string key.
  std::string get_name_(int first, int second, int third) const;
  /// ALl LCAs for leaf pairs.
  std::unordered_map<std::string, std::pair<int, Node *>> pairs_;
  /// All LCAs for leaf triples.
  ///! NOT YET implemented.
  std::unordered_map<std::string, std::pair<int, Node *>> triples_;
};

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
  /// Add all extra leaves.
  void add_root_leafs();
  /// Check if the trees are exactly same.
  /// @return True if all are exactly same, false otherwise.
  bool are_identical();
  /// Assign numbers to all trees.
  void assign_numbers();
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
  inline std::vector<Node> &get_trees() { return trees_; }
  /// Set the tree decomposition by parsing its string representation.
  /// @param str Its string representation.
  void set_tree_decomposition(const std::string &str);

private:
  /// The tree decomposition.
  TreeDecomposition decomp_;
  /// Number of leafs in each tree.
  int n_;
  /// Number of trees.
  int t_;
  /// Array of all trees.
  std::vector<Node> trees_;
};
#endif
