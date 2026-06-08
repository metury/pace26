#include "ilp.h"
#include "tree.h"
#include "utils.h"
#include <iostream>
#include <memory>

void process(std::istream &is) {
  // Load the input from the file.
  auto input = Input(is);
  input.compute_all_lca();

  // Output what is inside.
  std::cout << "Instance contains " << GREEN << input.get_tree_count() << RESET
            << " trees with " << GREEN << input.get_leaf_count() << RESET
            << " leafs each." << std::endl;

  // Contract cherries to reduce size.
  input.contract_cherries();
  auto ilp = ILP(input);
  std::set<int> edges_to_erase;
  std::vector<std::unique_ptr<Tree>> output;
  do {
    edges_to_erase = ilp.run(input);
    output = input.remove_edges(edges_to_erase);
  } while (ilp.update(input, output));

  auto result = 0;
  for (auto &&tree : output) {
    if (!tree->is_empty()) {
      tree->write(input.get_contractions());
    }
    ++result;
  }
  if (output.empty()) {
    input.get_trees()[0]->write(input.get_contractions());
    ++result;
  }
  std::cout << "# Size of the solution: " << VIOLET << result << RESET << "."
            << std::endl;
}

int main(int argc, char **argv) {
  try {
    process(std::cin);
    return 0;
  } catch (...) {
    std::cerr << "# Something went wrong." << std::endl;
  }
}
