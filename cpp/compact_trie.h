#ifndef COMPACT_TRIE_H__
#define COMPACT_TRIE_H__

#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Binary format node
struct CompactNode {
  uint32_t child_mask_;
  uint32_t words_under_;
  int32_t children[];  // Child indices

  bool StartsWord(int i) const { return child_mask_ & (1 << i); }
  bool IsWord() const { return child_mask_ & (1 << 31); }

  CompactNode *Descend(int i, uint32_t &word_id) {
    uint32_t letter_bit = 1u << i;
    if (!(child_mask_ & letter_bit)) {
      return nullptr;
    }
    uint32_t mask_before = child_mask_ & (letter_bit - 1);
    int child_index = std::popcount(mask_before);
    for (int idx = 0; idx < child_index; idx++) {
      auto child_offset = children[idx];
      auto child = (CompactNode *)((uint32_t *)this + child_offset);
      word_id += child->words_under_;
    }
    auto child_offset = children[child_index];
    auto child = (CompactNode *)((uint32_t *)this + child_offset);
    return child;
  };
};

// Trie-like interface backed by mmap'd CompactNode array
class CompactTrie {
 public:
  // Constructor for a view into a specific node
  CompactTrie(CompactNode *nodes);

  CompactNode *GetRoot() { return nodes_; }

  const string &WordAtIndex(uint32_t index) { return words_[index]; }

  // Loading from binary file
  static unique_ptr<CompactTrie> CreateFromBinaryFile(const char *filename);

  // Cleanup
  ~CompactTrie();

 private:
  CompactNode *nodes_;  // mmap'd array of nodes (owned by root only)

  vector<string> words_;

  // For memory management
  size_t file_size_;
  int fd_;
};

#endif  // COMPACT_TRIE_H__
