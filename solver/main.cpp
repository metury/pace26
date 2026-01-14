#include <iostream>
#include "tree.h"
#include "utils.h"

int main(int argc, char** argv) {
  try {
    //Create simple tree.
    Node tree;
    Node* l = tree.add_left();
    l->set_value(5);
    Node* r = tree.add_right();
    r->change_type();
    Node* rr = r->add_right();
    Node* rl = r->add_left();
    rr->set_value(6);
    rl->set_value(4444);
    std::cout << tree << std::endl;
    
    // Remove left tree and consolidate.
    std::unique_ptr<Node> left = tree.remove_left();

    std::cout << "Removed left " << *(left.get()) << " and obtained " << tree << std::endl;

    // Get file names in arguments.
    std::vector<std::string> arguments(argv + 1, argv + argc);

    for(auto&& file : arguments){
      std::cout << "# Processing file \"" << file << "\"." << std::endl;
      auto input = Input(file);
      std::cout << "# Read file \"" << file << "\" containing " << input.get_tree_count() << " trees with " << input.get_leaf_count() << " leafs each:" << std::endl;
      input.assign_numbers();
      for(auto&& tree : input.get_trees()){
        std::cout << "# ";
        tree.write(std::cout);
      }
      input.get_tree_decomposition().write(std::cout);
    }

    return 0;
  } catch(...) {
    std::cerr << "# Something went wrong." << std::endl;
  }
}
