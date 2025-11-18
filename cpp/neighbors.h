#ifndef NEIGHBORS_H
#define NEIGHBORS_H

// Cell neighbor information for different board sizes.
// First entry is the number of neighbors in the list.
// TODO: make these null-terminated rather than "pascal arrays" (may be faster).

// Update the blocks below via
// poetry run cog -r -P cpp/neighbors.h

// clang-format off

/*[[[cog
from boggle.neighbors import NEIGHBORS

for (w, h), neighbors in NEIGHBORS.items():
    print(f"""
// {w}x{h}
const int NEIGHBORS_{w}x{h}[{w}*{h}][9] = {{""")
    for ns in neighbors:
        ns_str = ", ".join(str(n) for n in ns)
        print(f"  {{{len(ns)}, {ns_str}}},")

    print("};")
]]]*/

// 2x2
const int NEIGHBORS_2x2[2*2][9] = {
  {3, 1, 2, 3},
  {3, 0, 2, 3},
  {3, 0, 1, 3},
  {3, 0, 1, 2},
};

// 2x3
const int NEIGHBORS_2x3[2*3][9] = {
  {3, 1, 3, 4},
  {5, 0, 2, 3, 4, 5},
  {3, 1, 4, 5},
  {3, 0, 1, 4},
  {5, 0, 1, 2, 3, 5},
  {3, 1, 2, 4},
};

// 3x3
const int NEIGHBORS_3x3[3*3][9] = {
  {3, 1, 3, 4},
  {5, 0, 2, 3, 4, 5},
  {3, 1, 4, 5},
  {5, 0, 1, 4, 6, 7},
  {8, 0, 1, 2, 3, 5, 6, 7, 8},
  {5, 1, 2, 4, 7, 8},
  {3, 3, 4, 7},
  {5, 3, 4, 5, 6, 8},
  {3, 4, 5, 7},
};

// 3x4
const int NEIGHBORS_3x4[3*4][9] = {
  {3, 1, 4, 5},
  {5, 0, 2, 4, 5, 6},
  {5, 1, 3, 5, 6, 7},
  {3, 2, 6, 7},
  {5, 0, 1, 5, 8, 9},
  {8, 0, 1, 2, 4, 6, 8, 9, 10},
  {8, 1, 2, 3, 5, 7, 9, 10, 11},
  {5, 2, 3, 6, 10, 11},
  {3, 4, 5, 9},
  {5, 4, 5, 6, 8, 10},
  {5, 5, 6, 7, 9, 11},
  {3, 6, 7, 10},
};

// 4x4
const int NEIGHBORS_4x4[4*4][9] = {
  {3, 1, 4, 5},
  {5, 0, 2, 4, 5, 6},
  {5, 1, 3, 5, 6, 7},
  {3, 2, 6, 7},
  {5, 0, 1, 5, 8, 9},
  {8, 0, 1, 2, 4, 6, 8, 9, 10},
  {8, 1, 2, 3, 5, 7, 9, 10, 11},
  {5, 2, 3, 6, 10, 11},
  {5, 4, 5, 9, 12, 13},
  {8, 4, 5, 6, 8, 10, 12, 13, 14},
  {8, 5, 6, 7, 9, 11, 13, 14, 15},
  {5, 6, 7, 10, 14, 15},
  {3, 8, 9, 13},
  {5, 8, 9, 10, 12, 14},
  {5, 9, 10, 11, 13, 15},
  {3, 10, 11, 14},
};

// 4x5
const int NEIGHBORS_4x5[4*5][9] = {
  {3, 1, 5, 6},
  {5, 0, 2, 5, 6, 7},
  {5, 1, 3, 6, 7, 8},
  {5, 2, 4, 7, 8, 9},
  {3, 3, 8, 9},
  {5, 0, 1, 6, 10, 11},
  {8, 0, 1, 2, 5, 7, 10, 11, 12},
  {8, 1, 2, 3, 6, 8, 11, 12, 13},
  {8, 2, 3, 4, 7, 9, 12, 13, 14},
  {5, 3, 4, 8, 13, 14},
  {5, 5, 6, 11, 15, 16},
  {8, 5, 6, 7, 10, 12, 15, 16, 17},
  {8, 6, 7, 8, 11, 13, 16, 17, 18},
  {8, 7, 8, 9, 12, 14, 17, 18, 19},
  {5, 8, 9, 13, 18, 19},
  {3, 10, 11, 16},
  {5, 10, 11, 12, 15, 17},
  {5, 11, 12, 13, 16, 18},
  {5, 12, 13, 14, 17, 19},
  {3, 13, 14, 18},
};

// 5x5
const int NEIGHBORS_5x5[5*5][9] = {
  {3, 1, 5, 6},
  {5, 0, 2, 5, 6, 7},
  {5, 1, 3, 6, 7, 8},
  {5, 2, 4, 7, 8, 9},
  {3, 3, 8, 9},
  {5, 0, 1, 6, 10, 11},
  {8, 0, 1, 2, 5, 7, 10, 11, 12},
  {8, 1, 2, 3, 6, 8, 11, 12, 13},
  {8, 2, 3, 4, 7, 9, 12, 13, 14},
  {5, 3, 4, 8, 13, 14},
  {5, 5, 6, 11, 15, 16},
  {8, 5, 6, 7, 10, 12, 15, 16, 17},
  {8, 6, 7, 8, 11, 13, 16, 17, 18},
  {8, 7, 8, 9, 12, 14, 17, 18, 19},
  {5, 8, 9, 13, 18, 19},
  {5, 10, 11, 16, 20, 21},
  {8, 10, 11, 12, 15, 17, 20, 21, 22},
  {8, 11, 12, 13, 16, 18, 21, 22, 23},
  {8, 12, 13, 14, 17, 19, 22, 23, 24},
  {5, 13, 14, 18, 23, 24},
  {3, 15, 16, 21},
  {5, 15, 16, 17, 20, 22},
  {5, 16, 17, 18, 21, 23},
  {5, 17, 18, 19, 22, 24},
  {3, 18, 19, 23},
};
// [[[end]]]

// clang-format on

// Template to select the right NEIGHBORS array based on dimensions
template <int M, int N>
struct Neighbors {
  static const int (&NEIGHBORS)[M * N][9];
};

// Specializations for each board size
template <>
struct Neighbors<2, 2> {
  static const int (&NEIGHBORS)[2 * 2][9];
};

template <>
struct Neighbors<2, 3> {
  static const int (&NEIGHBORS)[2 * 3][9];
};

template <>
struct Neighbors<3, 3> {
  static const int (&NEIGHBORS)[3 * 3][9];
};

template <>
struct Neighbors<3, 4> {
  static const int (&NEIGHBORS)[3 * 4][9];
};

template <>
struct Neighbors<4, 4> {
  static const int (&NEIGHBORS)[4 * 4][9];
};

template <>
struct Neighbors<4, 5> {
  static const int (&NEIGHBORS)[4 * 5][9];
};

template <>
struct Neighbors<5, 5> {
  static const int (&NEIGHBORS)[5 * 5][9];
};

// Definitions
const int (&Neighbors<2, 2>::NEIGHBORS)[2 * 2][9] = NEIGHBORS_2x2;
const int (&Neighbors<2, 3>::NEIGHBORS)[2 * 3][9] = NEIGHBORS_2x3;
const int (&Neighbors<3, 3>::NEIGHBORS)[3 * 3][9] = NEIGHBORS_3x3;
const int (&Neighbors<3, 4>::NEIGHBORS)[3 * 4][9] = NEIGHBORS_3x4;
const int (&Neighbors<4, 4>::NEIGHBORS)[4 * 4][9] = NEIGHBORS_4x4;
const int (&Neighbors<4, 5>::NEIGHBORS)[4 * 5][9] = NEIGHBORS_4x5;
const int (&Neighbors<5, 5>::NEIGHBORS)[5 * 5][9] = NEIGHBORS_5x5;

#endif  // NEIGHBORS_H
