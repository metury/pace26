#include "tree.h"
#include <iostream>

void test_of_lib() {
  // Create simple tree.
  Node tree;
  Node *l = tree.add_left();
  l->set_value(5);
  Node *r = tree.add_right();
  r->change_type();
  Node *rr = r->add_right();
  Node *rl = r->add_left();
  rr->set_value(6);
  rl->set_value(4444);
  std::cout << tree << std::endl;

  // Remove left tree and consolidate.
  std::unique_ptr<Node> left = tree.remove_left();

  std::cout << "Removed left " << *(left.get()) << " and obtained " << tree
            << std::endl;
}

void process_input_file(const std::string &file) {
  std::cout << "# Processing file \"" << file << "\"." << std::endl;
  auto input = Input(file);
  std::cout << "# Read file \"" << file << "\" containing "
            << input.get_tree_count() << " trees with "
            << input.get_leaf_count() << " leafs each:" << std::endl;
  input.assign_numbers();
  if (input.are_identical()) {
    std::cout << "# The trees are exactly same." << std::endl;
    input.get_trees()[0].write(std::cout);
    return;
  }
  for (auto &&tree : input.get_trees()) {
    std::cout << "# ";
    tree.write();
  }
  input.get_tree_decomposition().write(std::cout);
}

int main(int argc, char **argv) {
  try {
    std::vector<std::string> arguments(argv + 1, argv + argc);
    for (auto &&file : arguments) {
      process_input_file(file);
    }
    return 0;
  } catch (...) {
    std::cerr << "# Something went wrong." << std::endl;
  }
}
