// Solver for MxN Boggle
#ifndef BOGGLER_4
#define BOGGLER_4

#include <unordered_set>

#include "neighbors.h"
#include "constants.h"
#include "prefix_dictionary.h"

template <int M, int N>
class Boggler {
 public:
  Boggler(PrefixDictionary* dict) : dict_(dict), runs_(0) {
    static_assert(
        sizeof(kWordScores) / sizeof(kWordScores[0]) - 1 >= M * N,
        "kWordScores must have at least M * N + 1 elements"
    );
  }

  int Score(const char* lets);

  unsigned int NumCells() { return M * N; }

  // Set a cell on the current board. Must have 0 <= x < M, 0 <= y < N and 0 <=
  // c < 26. These constraints are NOT checked.
  void SetCell(int x, int y, unsigned int c);
  unsigned int Cell(int x, int y) const;

  // This is used by the web Boggle UI
  vector<vector<int>> FindWords(const string& lets, bool multiboggle);

 private:
  void DoDFS(unsigned int i, unsigned int len);
  void FindWordsDFS(
      unsigned int i, bool multiboggle, vector<vector<int>>& out
  );
  unsigned int InternalScore();
  bool ParseBoard(const char* bd);

  PrefixDictionary* dict_;
  unsigned int used_;
  int bd_[M * N];
  unsigned int score_;
  unsigned int runs_;
  string word_;  // Current word being built during DFS
  vector<int> seq_;
  unordered_set<uint64_t> found_words_;
};

template <int M, int N>
void Boggler<M, N>::SetCell(int x, int y, unsigned int c) {
  bd_[(x * N) + y] = c;
}

template <int M, int N>
unsigned int Boggler<M, N>::Cell(int x, int y) const {
  return bd_[(x * N) + y];
}

template <int M, int N>
int Boggler<M, N>::Score(const char* lets) {
  if (!ParseBoard(lets)) {
    return -1;
  }
  return InternalScore();
}

template <int M, int N>
bool Boggler<M, N>::ParseBoard(const char* bd) {
  unsigned int expected_len = M * N;
  if (strlen(bd) != expected_len) {
    fprintf(
        stderr,
        "Board strings must contain %d characters, got %zu ('%s')\n",
        expected_len,
        strlen(bd),
        bd
    );
    return false;
  }

  for (unsigned int i = 0; i < expected_len; i++) {
    if (bd[i] == '.') {
      bd_[i] = -1;  // explicit "do not go here"; only supported by FindWords()
      continue;
    }
    if (bd[i] >= 'A' && bd[i] <= 'Z') {
      fprintf(stderr, "Found uppercase letter '%c'\n", bd[i]);
      return false;
    } else if (bd[i] < 'a' || bd[i] > 'z') {
      fprintf(stderr, "Found unexpected letter: '%c'\n", bd[i]);
      return false;
    }
    bd_[i] = bd[i] - 'a';
  }
  return true;
}

template <int M, int N>
unsigned int Boggler<M, N>::InternalScore() {
  runs_++;
  dict_->ResetMarks();
  used_ = 0;
  score_ = 0;
  word_.clear();
  word_.reserve(M * N);
  for (int i = 0; i < M * N; i++) {
    int c = bd_[i];
    word_.push_back('a' + c);
    if (dict_->HasPrefix(word_)) {
      DoDFS(i, 0);
    }
    word_.pop_back();
  }
  return score_;
}

#define REC(idx)                              \
  do {                                        \
    if ((used_ & (1 << idx)) == 0) {          \
      cc = bd_[idx];                          \
      word_.push_back('a' + cc);              \
      if (dict_->HasPrefix(word_)) {          \
        DoDFS(idx, len);                      \
      }                                       \
      word_.pop_back();                       \
    }                                         \
  } while (0)

#define REC3(a, b, c) \
  REC(a);             \
  REC(b);             \
  REC(c)

#define REC5(a, b, c, d, e) \
  REC3(a, b, c);            \
  REC(d);                   \
  REC(e)

#define REC8(a, b, c, d, e, f, g, h) \
  REC5(a, b, c, d, e);               \
  REC3(f, g, h)

// PREFIX and SUFFIX could be inline methods instead, but this incurs a ~5% perf hit.
#define PREFIX()                        \
  int c = bd_[i], cc;                   \
  used_ ^= (1 << i);                    \
  len += (c == kQ ? 2 : 1);             \
  if (dict_->IsWord(word_)) {           \
    if (dict_->GetMark(word_) != runs_) { \
      dict_->MarkWord(word_, runs_);    \
      score_ += kWordScores[len];       \
    }                                   \
  }

#define SUFFIX() used_ ^= (1 << i)

// clang-format off

/*[[[cog
from boggle.neighbors import NEIGHBORS

for (w, h), neighbors in NEIGHBORS.items():
    print(f"""
// {w}x{h}
template<>
void Boggler<{w}, {h}>::DoDFS(unsigned int i, unsigned int len, Trie* t) {{
  PREFIX();
  switch(i) {{""")
    for i, ns in enumerate(neighbors):
        csv = ", ".join(str(n) for n in ns)
        print(f"    case {i}: REC{len(ns)}({csv}); break;")

    print("""  }
  SUFFIX();
}""")
]]]*/

// 2x2
template<>
void Boggler<2, 2>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 2, 3); break;
    case 1: REC3(0, 2, 3); break;
    case 2: REC3(0, 1, 3); break;
    case 3: REC3(0, 1, 2); break;
  }
  SUFFIX();
}

// 2x3
template<>
void Boggler<2, 3>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 3, 4); break;
    case 1: REC5(0, 2, 3, 4, 5); break;
    case 2: REC3(1, 4, 5); break;
    case 3: REC3(0, 1, 4); break;
    case 4: REC5(0, 1, 2, 3, 5); break;
    case 5: REC3(1, 2, 4); break;
  }
  SUFFIX();
}

// 3x3
template<>
void Boggler<3, 3>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 3, 4); break;
    case 1: REC5(0, 2, 3, 4, 5); break;
    case 2: REC3(1, 4, 5); break;
    case 3: REC5(0, 1, 4, 6, 7); break;
    case 4: REC8(0, 1, 2, 3, 5, 6, 7, 8); break;
    case 5: REC5(1, 2, 4, 7, 8); break;
    case 6: REC3(3, 4, 7); break;
    case 7: REC5(3, 4, 5, 6, 8); break;
    case 8: REC3(4, 5, 7); break;
  }
  SUFFIX();
}

// 3x4
template<>
void Boggler<3, 4>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 4, 5); break;
    case 1: REC5(0, 2, 4, 5, 6); break;
    case 2: REC5(1, 3, 5, 6, 7); break;
    case 3: REC3(2, 6, 7); break;
    case 4: REC5(0, 1, 5, 8, 9); break;
    case 5: REC8(0, 1, 2, 4, 6, 8, 9, 10); break;
    case 6: REC8(1, 2, 3, 5, 7, 9, 10, 11); break;
    case 7: REC5(2, 3, 6, 10, 11); break;
    case 8: REC3(4, 5, 9); break;
    case 9: REC5(4, 5, 6, 8, 10); break;
    case 10: REC5(5, 6, 7, 9, 11); break;
    case 11: REC3(6, 7, 10); break;
  }
  SUFFIX();
}

// 4x4
template<>
void Boggler<4, 4>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 4, 5); break;
    case 1: REC5(0, 2, 4, 5, 6); break;
    case 2: REC5(1, 3, 5, 6, 7); break;
    case 3: REC3(2, 6, 7); break;
    case 4: REC5(0, 1, 5, 8, 9); break;
    case 5: REC8(0, 1, 2, 4, 6, 8, 9, 10); break;
    case 6: REC8(1, 2, 3, 5, 7, 9, 10, 11); break;
    case 7: REC5(2, 3, 6, 10, 11); break;
    case 8: REC5(4, 5, 9, 12, 13); break;
    case 9: REC8(4, 5, 6, 8, 10, 12, 13, 14); break;
    case 10: REC8(5, 6, 7, 9, 11, 13, 14, 15); break;
    case 11: REC5(6, 7, 10, 14, 15); break;
    case 12: REC3(8, 9, 13); break;
    case 13: REC5(8, 9, 10, 12, 14); break;
    case 14: REC5(9, 10, 11, 13, 15); break;
    case 15: REC3(10, 11, 14); break;
  }
  SUFFIX();
}

// 4x5
template<>
void Boggler<4, 5>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 5, 6); break;
    case 1: REC5(0, 2, 5, 6, 7); break;
    case 2: REC5(1, 3, 6, 7, 8); break;
    case 3: REC5(2, 4, 7, 8, 9); break;
    case 4: REC3(3, 8, 9); break;
    case 5: REC5(0, 1, 6, 10, 11); break;
    case 6: REC8(0, 1, 2, 5, 7, 10, 11, 12); break;
    case 7: REC8(1, 2, 3, 6, 8, 11, 12, 13); break;
    case 8: REC8(2, 3, 4, 7, 9, 12, 13, 14); break;
    case 9: REC5(3, 4, 8, 13, 14); break;
    case 10: REC5(5, 6, 11, 15, 16); break;
    case 11: REC8(5, 6, 7, 10, 12, 15, 16, 17); break;
    case 12: REC8(6, 7, 8, 11, 13, 16, 17, 18); break;
    case 13: REC8(7, 8, 9, 12, 14, 17, 18, 19); break;
    case 14: REC5(8, 9, 13, 18, 19); break;
    case 15: REC3(10, 11, 16); break;
    case 16: REC5(10, 11, 12, 15, 17); break;
    case 17: REC5(11, 12, 13, 16, 18); break;
    case 18: REC5(12, 13, 14, 17, 19); break;
    case 19: REC3(13, 14, 18); break;
  }
  SUFFIX();
}

// 5x5
template<>
void Boggler<5, 5>::DoDFS(unsigned int i, unsigned int len) {
  PREFIX();
  switch(i) {
    case 0: REC3(1, 5, 6); break;
    case 1: REC5(0, 2, 5, 6, 7); break;
    case 2: REC5(1, 3, 6, 7, 8); break;
    case 3: REC5(2, 4, 7, 8, 9); break;
    case 4: REC3(3, 8, 9); break;
    case 5: REC5(0, 1, 6, 10, 11); break;
    case 6: REC8(0, 1, 2, 5, 7, 10, 11, 12); break;
    case 7: REC8(1, 2, 3, 6, 8, 11, 12, 13); break;
    case 8: REC8(2, 3, 4, 7, 9, 12, 13, 14); break;
    case 9: REC5(3, 4, 8, 13, 14); break;
    case 10: REC5(5, 6, 11, 15, 16); break;
    case 11: REC8(5, 6, 7, 10, 12, 15, 16, 17); break;
    case 12: REC8(6, 7, 8, 11, 13, 16, 17, 18); break;
    case 13: REC8(7, 8, 9, 12, 14, 17, 18, 19); break;
    case 14: REC5(8, 9, 13, 18, 19); break;
    case 15: REC5(10, 11, 16, 20, 21); break;
    case 16: REC8(10, 11, 12, 15, 17, 20, 21, 22); break;
    case 17: REC8(11, 12, 13, 16, 18, 21, 22, 23); break;
    case 18: REC8(12, 13, 14, 17, 19, 22, 23, 24); break;
    case 19: REC5(13, 14, 18, 23, 24); break;
    case 20: REC3(15, 16, 21); break;
    case 21: REC5(15, 16, 17, 20, 22); break;
    case 22: REC5(16, 17, 18, 21, 23); break;
    case 23: REC5(17, 18, 19, 22, 24); break;
    case 24: REC3(18, 19, 23); break;
  }
  SUFFIX();
}
// [[[end]]]
// clang-format on

#undef REC
#undef REC3
#undef REC5
#undef REC8
#undef PREFIX
#undef SUFFIX

template <int M, int N>
vector<vector<int>> Boggler<M, N>::FindWords(const string& lets, bool multiboggle) {
  found_words_.clear();
  seq_.clear();
  seq_.reserve(M * N);
  word_.clear();
  word_.reserve(M * N);
  vector<vector<int>> out;
  if (!ParseBoard(lets.c_str())) {
    out.push_back({-1});
    return out;
  }

  runs_++;
  dict_->ResetMarks();
  used_ = 0;
  score_ = 0;
  for (int i = 0; i < M * N; i++) {
    int c = bd_[i];
    if (c != -1) {
      word_.push_back('a' + c);
      if (dict_->HasPrefix(word_)) {
        FindWordsDFS(i, multiboggle, out);
      }
      word_.pop_back();
    }
  }
  return out;
}

// This could be specialized, but it's not as performance-sensitive as DoDFS()
template <int M, int N>
void Boggler<M, N>::FindWordsDFS(
    unsigned int i, bool multiboggle, vector<vector<int>>& out
) {
  used_ ^= (1 << i);
  seq_.push_back(i);
  if (dict_->IsWord(word_)) {
    bool should_count;
    if (multiboggle) {
      int word_id = dict_->GetWordId(word_);
      uint64_t key = (((uint64_t)word_id) << 32) + used_;
      auto result = found_words_.emplace(key);
      should_count = result.second;
    } else {
      should_count = (dict_->GetMark(word_) != runs_);
    }
    if (should_count) {
      dict_->MarkWord(word_, runs_);
      out.push_back(seq_);
    }
  }

  auto& neighbors = Neighbors<M, N>::NEIGHBORS[i];
  auto n_neighbors = neighbors[0];
  for (int j = 1; j <= n_neighbors; j++) {
    auto idx = neighbors[j];
    if ((used_ & (1 << idx)) == 0) {
      int cc = bd_[idx];
      if (cc != -1) {
        word_.push_back('a' + cc);
        if (dict_->HasPrefix(word_)) {
          FindWordsDFS(idx, multiboggle, out);
        }
        word_.pop_back();
      }
    }
  }

  used_ ^= (1 << i);
  seq_.pop_back();
}

#endif  // BOGGLER_4
