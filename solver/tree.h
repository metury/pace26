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
    Node();
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
    Node* addLeft();
    Node* addRight();
    void setValue(int value);
    int getValue() const;
    NodeType changeType();
    NodeType getType() const;
    Node* getLeft() const;
    Node* getRight() const;
    std::unique_ptr<Node> removeLeft();
    std::unique_ptr<Node> removeRight();
    void consolidate();
  private:
    NodeType type_;
    std::unique_ptr<Node> left_;
    std::unique_ptr<Node> right_;
    Node* parent_;
    int value_;
};

std::ostream& operator<<(std::ostream& os, const Node& n);

std::istream& operator>>(std::istream& is, Node& n);

void readFile(const std::string& filePath, std::vector<Node>& trees);

#endif
