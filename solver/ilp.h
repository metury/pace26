/// @file ilp.h
/// @brief Creating and running ILP.
/// Definition and computation of the ILP. Also solving the ILP itself.
/// Used solver: highs.dev
#ifndef ilp_h_
#define ilp_h_

#include "tree.h"

/// Solve ILP. Return set of edges as a solution, where each edge has number
/// from its lower vertex.
/// @param input On which input we should solve the ILP.
/// @return Indices of edges to be removed.
std::set<int> ilp(Input &input);
#endif
