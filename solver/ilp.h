/// @file ilp.h
/// @brief Creating and running ILP.
/// Definition and computation of the ILP. Also solving the ILP itself.
/// Used solver: highs.dev
#ifndef ilp_h_
#define ilp_h_

#include "tree.h"
#include <tuple>
#include <vector>

/// Solve ILP. Return set of edges as a solution, where each edge has number
/// from its lower vertex.
/// @param input On which input we should solve the ILP.
/// @return Indices of edges to be removed.
std::set<int> ilp(Input &input, int limit,
                  const std::vector<std::tuple<int, int, int>> &trios,
                  const std::vector<std::tuple<int, int, int, int>> &quartets);

/// Solve LP relaxation.
/// @param input On which input we should solve the LP.
// int lp(Input &input);

/// Solve the ilp either with integers or floats.
/// @param input On which input we should solve the (I)LP.
/// @param integer Whether it should be on integer values or not.
/// @return What is the value for each edge in the input tree.
std::unordered_map<int, float>
ilp_general(Input &input, int limit,
            const std::vector<std::tuple<int, int, int>> &trios,
            const std::vector<std::tuple<int, int, int, int>> &quartets,
            bool integer);
#endif
