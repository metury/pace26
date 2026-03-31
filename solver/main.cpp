#include "ilp.h"
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
  input.contract_cherries();
  ilp(input);
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
