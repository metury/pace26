#include "ilp.h"
#include "tree.h"
#include "utils.h"
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

  // Contract cherries to reduce size.
  input.contract_cherries_chains();
  auto lower_bound = lp(input);
  std::cout << "# Lower bound: " << YELLOW << lower_bound << RESET << "."
            << std::endl;
  // Find the solution.
  auto edges_to_erase = ilp(input);
  // Remove the edges.
  auto output = input.remove_edges(edges_to_erase);
  // Print the result.
  auto result = 0;
  for (auto &&tree : output) {
    if (!tree->is_empty()) {
      tree->write(input.get_contractions());
      ++result;
    }
  }
  std::cout << "# Size of the solution: " << VIOLET << result << RESET << "."
            << std::endl;
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
