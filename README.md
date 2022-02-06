# bimap-cpp

My bimap implementation [`bimap`](https://en.wikipedia.org/wiki/Bidirectional_map)  

A `bimap` is a data structure that stores a set of pairs and efficiently searches for a key by value. Unlike `map`, the search in `bimap` can be performed both on the left (left) elements of pairs, and on the right (right).

`bimap` is parameterized by 2 types (left and right) and 2 comparators that determine the order on these types.

The `bimap` iterator repeats the corresponding behavior for `map` and allows passing all elements on one side in the order determined by the passed comparator.
