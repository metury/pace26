#include "tree.h"
#include <iostream>

void process_input_file(const std::string &file) {
  std::cout << "# Processing file \"" << file << "\"." << std::endl;
  auto input = Input(file);
  std::cout << "# File \"" << file << "\" contains " << input.get_tree_count()
            << " trees with " << input.get_leaf_count() << " leafs each."
            << std::endl;
  input.assign_numbers();
  if (input.are_identical()) {
    std::cout << "# The trees are exactly same." << std::endl;
    input.get_trees()[0].write(std::cout);
    // return;
  }
  int i = 1;
  for (auto &&tree : input.get_trees()) {
    std::cout << "# Tree " << i++ << ": ";
    tree.write();
  }
  i = 1;
  input.add_root_leafs();
  for (auto &&tree : input.get_trees()) {
    std::cout << "# Tree with added root " << i << ": ";
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
