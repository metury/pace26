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

  std::vector<std::tuple<int, int, int>> trios;
  std::vector<std::tuple<int, int, int, int>> quartets;
  auto result = 0;
  auto output = std::vector<std::unique_ptr<Tree>>{};
  auto components = std::vector<int>(input.get_leaf_count() + 1, 1);
  auto counter = 0;
  while (true) {
    input.compute_trios_quartets(trios, quartets, 1000, components);
    if (counter == trios.size() + quartets.size()) {
      break;
    }
    counter = trios.size() + quartets.size();
    //  Find the solution.
    auto edges_to_erase = ilp(input, result - 1, trios, quartets);
    output = input.remove_edges(edges_to_erase);
    // Print the result.
    result = 0;
    auto tree_counter = 0;
    for (auto &&tree : output) {
      ++tree_counter;
      if (!tree->is_empty()) {
        ++result;
        auto leafs = tree->get_leafs();
        for (auto &&taxa : leafs) {
          components[taxa] = tree_counter;
        }
      }
    }
  }
  for (auto &&tree : output) {
    if (!tree->is_empty()) {
      tree->write(input.get_contractions());
    }
  }
  if (output.empty()) {
    input.get_trees()[0]->write(input.get_contractions());
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
