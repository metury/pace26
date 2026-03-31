#include "ilp.h"
#include "tree.h"
#include <iostream>

void process_input_file(const std::string &file) {
  std::cout << "# Processing file \"" << BLUE << file << RESET << "\"."
            << std::endl;

  // Load the input from the file.
  auto input = Input(file);

  // Output what is inside.
  std::cout << "# File \"" << GREEN << file << RESET << "\" contains " << GREEN
            << input.get_tree_count() << RESET << " trees with " << GREEN
            << input.get_leaf_count() << RESET << " leafs each." << std::endl;
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
