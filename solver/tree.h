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
  inline void assign_numbers(int i, int n) {
    root_->assign_numbers(i * (n - 1) + 2);
  }
  /// Force contractions and remove empty branches.
  void consolidate();
  /// Contract a cherry consisting of two nodes.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  void contract_cherry(int first, int second);
  /// Compute both leaf pointers and lca values.
  /// @param number_of_nodes How many nodes does the tree have.
  void compute_lca_leafs(int number_of_nodes);
  /// Delete LCA table to reduce memory consumption.
  inline void delete_lca_table() { lca_table_ = {}; }
  /// Add all edges between the nodes.
  /// @param below Pointer to the node below.
  /// @param above Pointer to the node above.
  /// @param edges Where to store all such edges.
  void get_edges(Node *below, Node *above, std::set<int> &edges) const;
  /// Get pointer to a leaf by its label.
  /// @param value Label of that leaf.
  /// @return Pointer to such leaf.
  inline Node *get_leaf(int value) const { return descendants_.at(value); }
  /// Get set of all taxa in this tree.
  /// @return Set of taxa present in this tree.
  std::set<int> get_leafs() const;
  /// Get pointer to the root node.
  /// @return Pointer to the root node.
  inline Node *get_root() const { return root_.get(); }
  /// Get violated edges for a trio.
  /// @param trio Which trio is violated.
  /// @return Set of edges that has to be in a constraint.
  std::set<int> get_trio_edges(const std::tuple<int, int, int> &trio) const;
  /// Get violated edges for a quartet.
  /// @param quartet Which quartet is violated.
  /// @return Set of edges that has to be in a constraint.
  std::set<int>
  get_quartet_edges(const std::tuple<int, int, int, int> &quartet) const;
  /// Check if paths between a-b and x-y intersect or not.
  /// @param a Vertex a.
  /// @param b Vertex b.
  /// @param x Vertex x.
  /// @param y Vertex y.
  /// @return If a-b and x-y paths intersect.
  bool has_disjoint_paths(int a, int b, int x, int y) const;
  /// Check if the provided trio a,b|x is proper or not.
  /// @param a Vertex a.
  /// @param b Vertex b.
  /// @param x Vertex x.
  /// @return True if x is not below a and b.
  bool has_disjoint_trio(int a, int b, int x) const;
  /// Find if two leafs create a cherry.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @return True if they are siblings.
  bool is_cherry(int first, int second) const;
  /// Return whether a tree has (almost) no nodes.
  /// @return True if the root is empty.
  inline bool is_empty() const { return root_ == std::nullptr_t(); }
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
  /// LCA table.
  std::vector<std::vector<int>> lca_table_;
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
  /// @param is On which stream the input will be provided.
  Input(std::istream &is);
  /// Default destructor.
  ~Input() = default;
  /// Assign numbers to all trees.
  void assign_numbers();
  /// Contract all cherries.
  /// @return True if everything was contracted to single leaf only.
  bool contract_cherries();
  /// Compute all LCA values for all trees.
  void compute_all_lca();
  /// Compute all incompatible trios and quartets
  /// @param trios List of all incompatible trios.
  /// @param quartets List of all incompatible quartets.
  /// @param limit WHat is the limit of number of constraints.
  /// @param components Connected components of taxa.
  /// @param all_constraints Whether we want to add all constraints.
  void compute_trios_quartets(std::vector<std::array<int, 4>> &trios,
                              std::vector<std::array<int, 5>> &quartets,
                              int limit, const std::vector<int> &components,
                              bool all_constraints);
  void compute_breakable_forks(
      std::vector<std::tuple<int, int, int>> &forks,
      std::vector<std::array<int, 7>> &extended_forks) const;
  /// Delete lca tables for every tree.
  void delete_lca_tables();
  /// Get all contracted parts from the input tree.
  /// @return All contractions.
  inline std::unordered_map<int, std::string> &get_contractions() {
    return contracted_;
  }
  /// Get the leaf count, which is same for all trees.
  /// @return Leaf count.
  inline int get_leaf_count() const { return n_; }
  /// Get the recuded leaf count, this differs after some cherries were
  /// contracted.
  /// @return Number of leaf which were not excluded.
  inline int get_reduced_leaf_count() const {
    return n_ - excluded_leafs_.size();
  }
  /// Get the node count.
  /// @return Node count.
  inline int get_node_count() const { return n_ + n_ - 1; }
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
