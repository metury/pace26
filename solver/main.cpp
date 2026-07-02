#include "ilp.h"

int main(int argc, char **argv) {
  try {
    auto input = Input(std::cin);
    input.compute_all_lca();
    input.contract_cherries();

    auto ilp = ILP(input);
    auto bound = std::max(0, input.get_reduced_leaf_count() - 2);
    ilp.initialize(0, bound, true);

    std::set<int> edges_to_erase;
    std::vector<std::unique_ptr<Tree>> output;

    do {
      // ilp.set_priorities();
      edges_to_erase = ilp.run();
      int size = edges_to_erase.size();
      bound = std::min(bound, size);
      output = input.remove_edges(edges_to_erase);
      ilp.set_components(output);
    } while (ilp.update());

    auto sol_size = 0;

    for (auto &&tree : output) {
      if (!tree->is_empty()) {
        ++sol_size;
      }
    }

    if (sol_size != bound) {
      ilp.drop_ilp();
      ilp.initialize(bound, sol_size, false);
      ilp.warm_start(edges_to_erase);

      do {
        // ilp.set_priorities();
        edges_to_erase = ilp.run();
        output = input.remove_edges(edges_to_erase);
        ilp.set_components(output);
      } while (ilp.update());
    }

    for (auto &&tree : output) {
      if (!tree->is_empty()) {
        tree->write(input.get_contractions());
      }
    }

    std::cout << "# Size of the solution: " << VIOLET
              << edges_to_erase.size() + 1 << RESET << "." << std::endl;
    return 0;
  } catch (...) {
    std::cerr << "# Something went wrong." << std::endl;
  }
}
