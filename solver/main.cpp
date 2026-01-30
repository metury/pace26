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

  int i = 1;
  for (auto &&tree : input.get_trees()) {
    // Output the tree.
    std::cout << "# Tree " << i++ << ": ";
    tree.write();
    // Write the descendants of the root.
    std::cout << "# Descendants of the root[" << tree.get_root()->get_value()
              << "]: ";
    for (auto &&[key, value] : tree.get_descendants()) {
      std::cout << key << ", ";
    }

    tree.updated_descendants();

    auto lca = LCA();
    std::cout << tree;
    lca.compute(&tree);
    lca.write();
    std::cout << "LCA for 1 and 2: " << lca.query(1, 2) << std::endl;
  }

  i = 1;
  input.add_root_leafs();
  for (auto &&tree : input.get_trees()) {
    std::cout << "# Tree with added root " << i++ << ": ";
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
