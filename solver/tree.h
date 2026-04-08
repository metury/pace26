/// @file tree.h
/// @brief Common framework for tree and input.
/// Classes for trees and input consisting of several trees. Trees contain LCA
/// tables. Also a class for tree decomposition.
#ifndef tree_h_
#define tree_h_

#include "node.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_map>
#include <vector>

/// Tree class with rooted node, leafs and lca tables.
class Tree {
public:
  /// Basic constructor with empty root.
  inline Tree() : root_(std::make_unique<Node>()) {}
  /// Constructor with given root.
  /// @param root Which node will be the root.
  Tree(std::unique_ptr<Node> root);
  /// Assign numbers to internal nodes.
  /// @param i Number of this tree.
  /// @param n Number of leafs.
  void assign_numbers(int i, int n);
  /// Force contractions and remove empty branches.
  void consolidate();
  /// Contract a cherry consisting of two nodes.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  void contract_cherry(int first, int second);
  void contract_chain(int a, int b, int c, int d);
  /// Compute both leaf pointers and lca values.
  void compute_lca_leafs();
  /// Add all edges between the nodes.
  /// @param below Pointer to the node below.
  /// @param above Pointer to the node above.
  /// @param edges Where to store all such edges.
  void get_edges(Node *below, Node *above, std::set<int> &edges) const;
  /// Get pointer to a leaf by its label.
  /// @param value Label of that leaf.
  /// @return Pointer to such leaf.
  inline Node *get_leaf(int value) const { return descendants_.at(value); }
  /// Get pointer to the root node.
  /// @return Pointer to the root node.
  inline Node *get_root() const { return root_.get(); }
  /// Find if two leafs create a cherry.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @return True if they are siblings.
  bool is_cherry(int first, int second) const;
  bool is_chain(int a, int b, int c, int d) const;
  /// Return whether a tree has (almost) no nodes.
  /// @return True if the root is empty.
  bool is_empty() const;
  /// Return a LCA node for two leafs.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @return Pointer to their common lca.
  Node *lca_query(int first, int second) const;
  /// Return a LCA node for three leafs.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @param third Label of the third leaf.
  /// @return Pointer to their common lca.
  Node *lca_query(int first, int second, int third) const;
  /// Output the tree to some stream in a Newick notation.
  /// @param os Which output stream to use.
  /// @param subst Which substitutions have to be propagated.
  void write(std::ostream &os,
             const std::unordered_map<int, std::string> &subst) const;
  /// Output the tree to standard output.
  /// @param subst Which substitutions have to be propagated.
  inline void write(const std::unordered_map<int, std::string> &subst) const {
    write(std::cout, subst);
  };

private:
  /// Map of descendant. Also can be used as a set of descendants.
  std::unordered_map<int, Node *> descendants_;
  /// Rot of the tree.
  std::unique_ptr<Node> root_;
  /// LCA table for pairs.
  LCA_TABLE pairs_;
  /// LCA table for triples.
  LCA_TABLE triples_;
};

/// Parse the tree from input stream in Newick format using `>>`.
/// @param is Which input stream to use.
/// @param t Where to store the tree.
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
  /// Constructor for parsing input from a file also assign numbers and compute
  /// LCA.
  /// @param file_path Path to the file.
  Input(const std::string &file_path);
  /// Default destructor.
  ~Input() = default;
  /// Assign numbers to all trees.
  void assign_numbers();
  /// Contract all cherries.
  void contract_cherries_chains();
  /// Compute all LCA values for all trees.
  void compute_all_lca();
  /// Compute all incompatible trios.
  /// @return List of all incompatible trios.
  std::vector<std::tuple<int, int, int>> compute_trios();
  /// Compute all incompatible quartets.
  /// @return List of all incompatible quartets.
  std::vector<std::tuple<int, int, int, int>> compute_quartets();
  /// Get all contracted parts from the input tree.
  /// @return All contractions.
  inline std::unordered_map<int, std::string> &get_contractions() {
    return contracted_;
  }
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
  /// Take the first tree and remove all edges, consolidate and output.
  /// @param edges_to_remove Which edges have to be removed.
  /// @return List of created trees from such removal and consolidations.
  std::vector<std::unique_ptr<Tree>>
  remove_edges(const std::set<int> &edges_to_remove);
  /// Set the tree decomposition by parsing its string representation.
  /// @param str Its string representation.
  void set_tree_decomposition(const std::string &str);

private:
  /// Add contracted cherry or chain.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  void add_contracted_(int first, int second);
  /// Recursively construct chains we find.
  /// @param node Which node we are considering now.
  void contract_chains_(Node *node, std::vector<int> &candidates);
  /// Recursively construct cherries we find.
  /// @param node Which node we are considering now.
  void contract_cherries_(Node *node);
  /// Recursively remove all edges.
  /// @param edges_to_remove Which edges to remove.
  /// @param trees Which trees we are considering.
  /// @param current_tree Where are we right now.
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
  /// Hash map of all contracted leafs.
  std::unordered_map<int, std::string> contracted_;
  /// Which leafs are exluded due to their contractions.
  std::set<int> excluded_leafs_;
};
#endif
