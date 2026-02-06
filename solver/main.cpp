#include "tree.h"
#include <iostream>

void process_input_file(const std::string &file) {
  std::cout << "# Processing file \"" << file << "\"." << std::endl;

  // Load the input from the file.
  auto input = Input(file);

  // Output what is inside.
  std::cout << "# File \"" << file << "\" contains " << input.get_tree_count()
            << " trees with " << input.get_leaf_count() << " leafs each."
            << std::endl;

  // Assign numbers to internal nodes.
  input.assign_numbers();
  input.compute_all_lca();

  int i = 1;
  for (auto &&[tree, lca] : input) {
    // Output the tree.
    std::cout << "# Tree " << i++ << ": ";
    tree.write();
    lca.write();
  }

  auto triplets = input.compute_triplets();

  for (auto &&[a, b, c] : triplets) {
    std::cout << a << "," << b << "|" << c << std::endl;
  }
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
