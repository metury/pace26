/// @file tree.h
/// @brief Common framework for tree and input.
/// Classes for trees and input consisting of several trees. Trees contain LCA
/// tables. Also a class for tree decomposition.
#ifndef tree_h_
#define tree_h_

#include "node.h"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <ranges>
#include <set>
#include <unordered_map>
#include <vector>

/// Tree class with rooted node, leafs and lca table.
class Tree {
public:
  /// Basic constructor with empty root.
  inline Tree(uint16_t n) : root_(std::make_unique<Node>()), lca_table_(n) {}
  /// Constructor with given root.
  /// @param root Which node will be the root.
  /// @param n Number of nodes total.
  Tree(std::unique_ptr<Node> root, uint16_t n);
  /// Assign numbers to internal nodes.
  /// @param i Number of this tree.
  /// @param n Number of leafs.
  inline void assign_numbers(uint8_t i, uint16_t n) {
    root_->assign_numbers(i * (n - 1) + 2);
  }
  /// Force contractions and remove empty branches.
  void consolidate();
  /// Contract a cherry consisting of two nodes.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  void contract_cherry(uint16_t first, uint16_t second);
  /// Compute both leaf pointers and lca values.
  void compute_lca_leafs();
  /// Compute the depth of this tree.
  /// @return The number of edges to the lowest leaf.
  inline uint16_t get_depth() const { return root_->get_depth(); }
  /// Add all edges between the nodes.
  /// @param below Pointer to the node below.
  /// @param above Pointer to the node above.
  /// @param edges Where to store all such edges.
  void get_edges(Node *below, Node *above, std::set<uint16_t> &edges) const;
  /// Get pointer to a leaf by its label.
  /// @param value Label of that leaf.
  /// @return Pointer to such leaf.
  inline Node *get_leaf(uint16_t value) const { return descendants_.at(value); }
  /// Get set of all taxa in this tree.
  /// @return Set of taxa present in this tree.
  std::set<uint16_t> get_leafs() const;
  /// Get pointer to the root node.
  /// @return Pointer to the root node.
  inline Node *get_root() const { return root_.get(); }
  /// Get violated edges for a trio.
  /// @param trio Which trio is violated.
  /// @return Set of edges that has to be in a constraint.
  std::set<uint16_t> get_trio_edges(const Trio &trio);
  /// Get violated edges for a quartet.
  /// @param quartet Which quartet is violated.
  /// @return Set of edges that has to be in a constraint.
  std::set<uint16_t> get_quartet_edges(const Quartet &quartet);
  /// Check if paths between a-b and x-y intersect or not.
  /// @param a Vertex a.
  /// @param b Vertex b.
  /// @param x Vertex x.
  /// @param y Vertex y.
  /// @return If a-b and x-y paths intersect.
  bool has_disjoint_paths(uint16_t a, uint16_t b, uint16_t x, uint16_t y);
  /// Check if the provided trio a,b|x is proper or not.
  /// @param a Vertex a.
  /// @param b Vertex b.
  /// @param x Vertex x.
  /// @return True if x is not below a and b.
  bool has_disjoint_trio(uint16_t a, uint16_t b, uint16_t x);
  /// Find if two leafs create a cherry.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @return True if they are siblings.
  bool is_cherry(uint16_t first, uint16_t second) const;
  /// Return whether a tree has (almost) no nodes.
  /// @return True if the root is empty.
  inline bool is_empty() const { return root_ == std::nullptr_t(); }
  /// Return a LCA node for two leafs.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @return Pointer to their common lca.
  Node *lca_query(uint16_t first, uint16_t second);
  /// Return a LCA node for three leafs.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  /// @param third Label of the third leaf.
  /// @return Pointer to their common lca.
  Node *lca_query(uint16_t first, uint16_t second, uint16_t third);
  /// Output the tree to some stream in a Newick notation.
  /// @param os Which output stream to use.
  /// @param subst Which substitutions have to be propagated.
  void write(std::ostream &os,
             const std::unordered_map<uint16_t, std::string> &subst) const;
  /// Output the tree to standard output.
  /// @param subst Which substitutions have to be propagated.
  inline void
  write(const std::unordered_map<uint16_t, std::string> &subst) const {
    write(std::cout, subst);
  };

private:
  /// Map of descendant. Also can be used as a set of descendants.
  std::unordered_map<uint16_t, Node *> descendants_;
  /// Rot of the tree.
  std::unique_ptr<Node> root_;
  /// LCA table.
  LcaTable lca_table_;
};

/// Parse the tree from input stream in Newick format using `>>`.
/// @param is Which input stream to use.
/// @param t Where to store the tree.
/// @return Changed input stream.
std::istream &operator>>(std::istream &is, Tree &t);

/// All the input provided. Including trees and tree decomposition.
class Input {
public:
  /// Default constructor.
  inline Input() : t_(0), n_(0), chosen_tree_(0) {}
  /// Constructor for parsing input from a file also assign numbers and compute
  /// LCA.
  /// @param is On which stream the input will be provided.
  Input(std::istream &is);
  /// Default destructor.
  ~Input() = default;
  /// Assign numbers to all trees.
  void assign_numbers();
  /// Contract all cherries.
  void contract_cherries();
  /// Compute all LCA values for all trees.
  void compute_lca_tables();
  /// Compute all incompatible trios and quartets
  /// @param trios List of all incompatible trios.
  /// @param quartets List of all incompatible quartets.
  void compute_trios_quartets(std::vector<Trio> &trios,
                              std::vector<Quartet> &quartets);
  /// Comput all forks the break symmetries.
  /// @param forks Where to put found forks.
  void compute_breakable_forks(std::vector<Fork> &forks) const;
  /// Return the chosen representative tree.
  /// @return Pointer to the chosen tree.
  inline Tree *get_chosen_tree() const { return trees_[chosen_tree_].get(); }
  /// Get all contracted parts from the input tree.
  /// @return All contractions.
  inline std::unordered_map<uint16_t, std::string> &get_contractions() {
    return contracted_;
  }
  /// Get the leaf count, which is same for all trees.
  /// @return Leaf count.
  inline uint16_t get_leaf_count() const { return n_; }
  /// Get the recuded leaf count, this differs after some cherries were
  /// contracted.
  /// @return Number of leaf which were not excluded.
  inline uint16_t get_reduced_leaf_count() const {
    return n_ - excluded_leafs_.size();
  }
  /// Get the node count.
  /// @return Node count.
  inline uint16_t get_node_count() const { return n_ + n_ - 1; }
  /// Get the tree count.
  /// @return Tree count.
  inline uint16_t get_tree_count() const { return t_; }
  /// Get reference to all trees.
  /// @return Reference to all trees.
  inline std::vector<std::unique_ptr<Tree>> &get_trees() { return trees_; }
  inline auto get_other_trees() const {
    return trees_ | std::views::filter([this](const std::unique_ptr<Tree> &t) {
             return t.get() != trees_[chosen_tree_].get();
           });
  }
  /// Take the first tree and remove all edges, consolidate and output.
  /// @param edges_to_remove Which edges have to be removed.
  /// @return List of created trees from such removal and consolidations.
  std::vector<std::unique_ptr<Tree>>
  cut_edges(const std::set<uint16_t> &edges_to_remove);

private:
  /// Add contracted cherry.
  /// @param first Label of the first leaf.
  /// @param second Label of the second leaf.
  void add_contracted_(uint16_t first, uint16_t second);
  /// Recursively construct cherries we find.
  /// @param node Which node we are considering now.
  void contract_cherries_recursive_(Node *node);
  /// Recursively remove all edges.
  /// @param edges_to_remove Which edges to remove.
  /// @param trees Which trees we are considering.
  /// @param current_tree Where are we right now.
  void cut_edges_recursive_(const std::set<uint16_t> &edges_to_remove,
                            std::vector<std::unique_ptr<Node>> &trees,
                            Node *current_tree);
  /// Number of leafs in each tree.
  uint16_t n_;
  /// Number of trees.
  uint16_t t_;
  /// Array of all trees.
  std::vector<std::unique_ptr<Tree>> trees_;
  /// Hash map of all contracted leafs.
  std::unordered_map<uint16_t, std::string> contracted_;
  /// Which leafs are exluded due to their contractions.
  std::set<uint16_t> excluded_leafs_;
  /// Which tree we have chosen.
  uint8_t chosen_tree_;
};
#endif
