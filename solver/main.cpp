#include "ilp.h"
#include "tree.h"
#include "utils.h"
#include <csignal>
#include <iostream>
#include <istream>
#include <memory>

// SCIILP *global_ilp = nullptr;
//
// void handle_signal(int signal) {
//   if (global_ilp != nullptr) {
//     std::cout << "\n[!] Received signal " << signal
//               << ". Safely interrupting SCIP..." << std::endl;
//
//     // global_ilp->terminate();
//     // SCIPinterruptSolve(global_scip_env);
//   }
// }
//
// void process_heuristics(std::istream &is) {
//   std::signal(SIGTERM, handle_signal);
//   std::signal(SIGINT, handle_signal);
//   // Load the input from the file.
//   auto input = Input(is);
//   input.compute_all_lca();
//
//   auto result = 0;
//   // Contract cherries to reduce size.
//   bool single_node = input.contract_cherries();
//
//   if (single_node) {
//     // Trees are already identical.
//     input.get_trees()[0]->write(input.get_contractions());
//     result = 1;
//   } else {
//     auto ilp = SCIILP(input);
//     global_ilp = &ilp;
//     // ilp.initialize(); All constraints.
//     ilp.set_priorities();
//     auto edges_to_erase = ilp.run();
//     auto output = input.remove_edges(edges_to_erase);
//
//     for (auto &&tree : output) {
//       if (!tree->is_empty()) {
//         tree->write(input.get_contractions());
//       }
//     }
//
//     result = edges_to_erase.size() + 1;
//   }
//   std::cout << "# Size of the solution: " << VIOLET << result << RESET << "."
//             << std::endl;
// }

void process(std::istream &is) {
  // Load the input from the file.
  auto input = Input(is);
  input.compute_all_lca();

  auto result = 0;
  // Contract cherries to reduce size.
  bool single_node = input.contract_cherries();

  if (single_node) {
    // Trees are already identical.
    input.get_trees()[0]->write(input.get_contractions());
    result = 1;
  } else {
    auto ilp = SCIILP(input);
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

    result = edges_to_erase.size() + 1;
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
