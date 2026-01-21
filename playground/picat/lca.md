
! Only some sketch of the ideas.

# Characterization of Maximum Agreement Forest by LCA

First recall that LCA stands for least common ancestor. We now try to approach
maximum agreement forest by stating some constraints on the forest itself.

## Necessary conditions

We split our potential trees into several cases, which will also help
define these properties recursively.

1. **Only one leaf.** -- This is actually very easy to do. Just cut off the very last
edge.
2. **Two leafs.** -- This is also easily doable for any two leafs. The main difference
with the first case is that it may overlap with other trees in some of the input
trees. Therefore it is *not sufficient.*
3. **One leaf and one tree.** -- In this case we have pre-computed LCA of the tree.
We only need to check that the given leaf is not *under* this LCA.
4. **Two trees.** -- Lastly we have computed two LCAs $x$ and $y$. We only need
to check that neither $x$ is below $y$ nor $y$ is below $x$.

These conditions must be met if we want to have proper agreement forest. But on
the other hand as we already stated they are not sufficient. Meaning that if we have
some tuples of leafs it clearly satisfies these, since no conditions were presented.
But one can easily find a counterexample, where this is not true.

## Two trees in the forest

When we consider two trees $T_1$ and $T_2$ in our agreement forest subject to
some tree from the input. It must be one of the following cases.

1. LCA of $T_1$ and LCA of $T_2$ are incomparable.
2. If, say LCA of $T_1$, is below LCA of $T_2$ then it must be that all
leafs from $T_2$ are not below LCA of $T_1$.
