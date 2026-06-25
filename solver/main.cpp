#include "ilp.h"

int main(int argc, char **argv) {
  try {
    auto input = Input(std::cin);
    input.compute_all_lca();
    input.contract_cherries();

    auto ilp = ILP(input);
    ilp.initialize();

    std::set<int> edges_to_erase;
    std::vector<std::unique_ptr<Tree>> output;

    do {
      ilp.set_priorities();
      edges_to_erase = ilp.run();
      output = input.remove_edges(edges_to_erase);
      ilp.set_components(output);
    } while (ilp.update());

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
