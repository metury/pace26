#ifndef tree_h_
#define tree_h_

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

/// Type of nodes, either LEAF or INNER.
enum NodeType {
  LEAF,
  INNER,
};

/// Node class for trees.
class Node {
public:
  /// Constructor. Set 0 value, LEAF and no null pointers.
  Node();
  /// Constructor with setting parent.
  /// @param parent Pointer to its parent.
  Node(Node *parent);
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
  /// Delete.
  ~Node() = default;
  /// Add left descendant and set type to LEAF.
  /// @return Pointer to the new descendant.
  Node *add_left();
  /// Add right descendant and set type to LEAF.
  /// @return Pointer to the new descendant.
  Node *add_right();
  /// Set value of current node.
  /// @param value Its new value.
  void set_value(int value);
  /// Get the value stored in this node.
  /// @return Its stored value.
  int get_value() const;
  /// Change the type of this node.
  /// @return New and changed type.
  NodeType change_type();
  /// Get current node type.
  /// @return Current node type.
  NodeType get_type() const;
  /// Get pointer to the left descendant.
  /// @return Pointer (monitor) to its left descendant.
  Node *get_left() const;
  /// Get pointer to the right descendant.
  /// @return Pointer (monitor) to its right descendant.
  Node *get_right() const;
  /// Traverse the tree and return the root.
  /// @return Root of this tree.
  Node *get_root();
  /// Remove the left descendant and also consolidate the tree.
  /// @return Its left descendant which was moved.
  std::unique_ptr<Node> remove_left();
  /// Remove the right descendant and also consolidate the tree.
  /// @return Its right descendant which was moved.
  std::unique_ptr<Node> remove_right();
  /// Force iterative contraction of all 2 degree inner vertices.
  void consolidate();
  /// Output the tree to some ostream in a Newick notation.
  /// @param os Which output stream to use.
  void write(std::ostream &os) const;
  /// Output the tree to standard outpu.
  inline void write() const { write(std::cout); };
  /// Assign numbers to all nodes including INNER nodes by a predefined way.
  /// @param i Which tree is this.
  /// @param n How many leafs the tree has.
  void assign_numbers(int i, int n);
  /// Pseudo sort the tree based on its leafs, left is smaller than right
  /// descendant. Lexicographically: first, sum of leaf labels and second,
  /// minimum label.
  void sort();

private:
  /// Assign numbers to INNER nodes.
  /// @param counter What is the counter for this node.
  /// @return Next free number from the tree below.
  int assign_numbers_(int counter);
  /// Sort the tree by swapping descendants.
  /// @return Both minimal value and the sum.
  std::tuple<int, int> sort_by_swaps_();
  /// Type of this node.
  NodeType type_;
  /// Left descendant.
  std::unique_ptr<Node> left_;
  /// Right descendant.
  std::unique_ptr<Node> right_;
  /// Monitor pointer to parent if exists.
  Node *parent_;
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
inline bool operator==(Node &lhs, Node &rhs) {
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

/// Compare two trees if they are not exactly same.
/// @param lhs Tree on the left hand side.
/// @param rhs Tree on the right hand side.
/// @return False if the trees exactly match.
inline bool operator!=(Node &lhs, Node &rhs) { return !(lhs == rhs); }

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
  /// Treewidth of the tree decomposition.
  int treewidth_;
  /// Set of bags. IDs are for node ids in the trees.
  std::vector<std::vector<int>> bags_;
  /// Set of edges between bags. IDs are for bags in their ordering.
  std::vector<std::tuple<int, int>> edges_;
};

/// All the input provided. Including trees and tree decomposition.
class Input {
public:
  /// Default constructor.
  Input();
  /// Constructor for parsing input from a file.
  /// @param file_path Path to the file.
  Input(const std::string &file_path);
  /// Get reference to all trees.
  /// @return Reference to all trees.
  std::vector<Node> &get_trees();
  /// Get the leaf count, which is same for all trees.
  /// @return Leaf count.
  int get_leaf_count() const;
  /// Get the tree count.
  /// @return Tree count.
  int get_tree_count() const;
  /// Assign numbers to all trees.
  void assign_numbers();
  /// Set the tree decomposition by parsing its string representation.
  /// @param str Its string representation.
  void set_tree_decomposition(const std::string &str);
  /// Get reference to the tree decomposition.
  /// @return Tree decomposition.
  TreeDecomposition &get_tree_decomposition();
  /// Check if the trees are exactly same.
  /// @return True if all are exactly same, false otherwise.
  bool are_identical();

private:
  /// Array of all trees.
  std::vector<Node> trees_;
  /// Number of leafs in each tree.
  int n_;
  /// Number of trees.
  int t_;
  /// The tree decomposition.
  TreeDecomposition decomp_;
};
#endif
