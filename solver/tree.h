#ifndef tree_h_
#define tree_h_

#include "node.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_map>
#include <vector>

class Tree {
public:
  inline Tree() : root_(std::make_unique<Node>()) {}
  inline Tree(std::unique_ptr<Node> root) : root_(std::move(root)) {
    root_->set_parent(nullptr);
  }
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
  bool is_empty() const;
  /// Output the tree to some ostream in a Newick notation.
  /// @param os Which output stream to use.
  void write(std::ostream &os,
             const std::unordered_map<int, std::string> &subst) const;
  /// Output the tree to standard outpu.
  inline void write(const std::unordered_map<int, std::string> &subst) const {
    write(std::cout, subst);
  };

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
  std::vector<std::unique_ptr<Tree>>
  remove_edges(const std::set<int> &edges_to_remove);
  inline std::unordered_map<int, std::string> &get_contractions() {
    return contracted_;
  }

private:
  void add_contracted_(int first, int second);
  bool contract_cherries_(Node *node);
  void remove_edges_(const std::set<int> &edges_to_remove,
                     std::vector<std::unique_ptr<Node>> &trees,
                     Node *current_tree);
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
