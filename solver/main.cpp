#include <iostream>
#include "tree.h"
#include "utils.h"

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

    std::cout << "Removed left " << *(left.get()) << " and obtained " << tree << std::endl;

    // Get file names in arguments.
    std::vector<std::string> arguments(argv + 1, argv + argc);

    for(auto&& file : arguments){
      std::cout << "# Processing file \"" << file << "\"." << std::endl;
      auto input = Input(file);
      std::cout << "# Read file \"" << file << "\" containing " << input.getTreeCount() << " trees with " << input.getLeafCount() << " leafs each:" << std::endl;
      for(auto&& tree : input.getTrees()){
        tree.write(std::cout);
      }
    }

    return 0;
  } catch(...) {
    std::cerr << "# Something went wrong." << std::endl;
  }
}
