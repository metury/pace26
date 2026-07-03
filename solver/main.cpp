#include "ilp.h"
#include "utils.h"
#include <cstdint>

int main(int argc, char **argv) {
  try {
    /// Load the input trees and do precomputation.
    auto input = Input(std::cin);
    input.compute_lca_tables();
    input.contract_cherries();

    /// Set up ILP solver.
    auto ilp = ILP(input);
    auto lower_bound = std::max(0, input.get_reduced_leaf_count() - 2);
    ilp.initialize(0, lower_bound, true);

    std::set<uint16_t> cut_edges;
    std::vector<std::unique_ptr<Tree>> forest;

    std::cout << "# Solving in a " << CYAN << "heuristic" << RESET << " mode."
              << std::endl;
    do {
      cut_edges = ilp.run();
      int size = cut_edges.size();
      lower_bound = std::min(lower_bound, size);
      forest = input.cut_edges(cut_edges);
      ilp.set_components(forest);
    } while (ilp.update());

    auto sol_size = -1;

    for (auto &&tree : forest) {
      if (!tree->is_empty()) {
        ++sol_size;
      }
    }

    auto heuristic_edges = cut_edges;

    std::cout << "# Solving in a " << CYAN << "normal" << RESET << " mode."
              << std::endl;
    if (sol_size != lower_bound) {
      ilp.drop_ilp();
      ilp.initialize(lower_bound, sol_size, false);
      ilp.warm_start(heuristic_edges, true);

      do {
        ilp.warm_start(heuristic_edges, false);
        cut_edges = ilp.run();
        forest = input.cut_edges(cut_edges);
        ilp.set_components(forest);
        if (sol_size == cut_edges.size()) {
          forest = input.cut_edges(heuristic_edges);
          cut_edges = heuristic_edges;
          break;
        }
      } while (ilp.update());
    }

    for (auto &&tree : forest) {
      if (!tree->is_empty()) {
        tree->write(input.get_contractions());
      }
    }

    std::cout << "# Size of the solution: " << VIOLET << cut_edges.size() + 1
              << RESET << "." << std::endl;
    return 0;
  } catch (...) {
    std::cerr << "# Something went wrong." << std::endl;
  }
}
