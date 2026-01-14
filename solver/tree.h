#ifndef tree_h_
#define tree_h_

#include <memory>
#include <vector>

/// Type of nodes.
enum NodeType {
  LEAF,
  INNER,
};

/// Node class.
class Node {
  public:
    /// Constructor.
    Node();
    /// Constructor with setting parent.
    Node(Node* parent);
    /// Copy constructor.
    Node(const Node& other);
    /// Copy assignement.
    Node& operator=(const Node& other);
    /// Move constructor.
    Node(Node&& other) = default;
    /// Move assignement.
    Node& operator=(Node&& other) = default;
    /// Delete.
    ~Node() = default;
    Node* add_left();
    Node* add_right();
    void set_value(int value);
    int get_value() const;
    NodeType change_type();
    NodeType get_type() const;
    Node* get_left() const;
    Node* get_right() const;
    Node* get_root();
    std::unique_ptr<Node> remove_left();
    std::unique_ptr<Node> remove_right();
    /// Force iterative contraction of all 2 degree inner vertices.
    void consolidate();
    void write(std::ostream& os) const;
    void assign_numbers(int i, int n);
  private:
    int assign_numbers_(int counter);
    NodeType type_;
    std::unique_ptr<Node> left_;
    std::unique_ptr<Node> right_;
    Node* parent_;
    int value_;
};

std::ostream& operator<<(std::ostream& os, const Node& n);
std::istream& operator>>(std::istream& is, Node& n);

class TreeDecomposition {
  public:
    TreeDecomposition() = default;
    TreeDecomposition(const std::string& str);
    void write(std::ostream& os);
  private:
    int treewidth_;
    std::vector<std::vector<int>> bags_;
    std::vector<std::tuple<int,int>> edges_;
};

class Input {
  public:
    Input();
    Input(const std::string& filePath);
    std::vector<Node>& get_trees();
    int get_leaf_count() const;
    int get_tree_count() const;
    void assign_numbers();
    void set_tree_decomposition(const std::string& str);
  TreeDecomposition& get_tree_decomposition();
  private:
    std::vector<Node> trees_;
    int n_;
    int t_;
    TreeDecomposition decomp_;
};
#endif
