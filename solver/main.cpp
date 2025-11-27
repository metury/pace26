#include <iostream>
#include "tree.h"

int main(int argc, char** argv) {
  try {
    //Create simple tree.
    Node tree;
    tree.changeType();
    Node* l = tree.addLeft();
    l->setValue(5);
    Node* r = tree.addRight();
    r->changeType();
    Node* rr = r->addRight();
    Node* rl = r->addLeft();
    rr->setValue(6);
    rl->setValue(4444);
    std::cout << tree << std::endl;
    
    // Remove left tree and consolidate.
    std::unique_ptr<Node> left = tree.removeLeft();
    tree.consolidate();

    std::cout << "Removed left " << *(left.get()) << " and obtained " << tree << std::endl;

    // Read tree from sting.
    Node second;
    std::cin >> second;

    std::cout << second << std::endl;
    return 0;
  } catch(...) {
    std::cerr << "Something went wrong." << std::endl;
  }
}
