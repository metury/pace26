#ifndef tree_h_
#define tree_h_

#include <memory>

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

#endif
