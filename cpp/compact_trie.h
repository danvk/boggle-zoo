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
  uint32_t child_mask;
  int32_t children[];  // Child indices

  // Fast operations matching old Trie interface
  bool StartsWord(int i) const { return child_mask & (1 << i); }
  bool IsWord() const { return child_mask & (1 << 31); }

  CompactNode *Descend(int i) {
    uint32_t letter_bit = 1u << i;
    if (!(child_mask & letter_bit)) {
      return nullptr;
    }
    uint32_t mask_before = child_mask & (letter_bit - 1);
    int child_index = std::popcount(mask_before);
    auto child_offset = children[child_index];
    return this + child_offset;
  };
};

// Trie-like interface backed by mmap'd CompactNode array
class CompactTrie {
 public:
  // Constructor for a view into a specific node
  CompactTrie(CompactNode *nodes, size_t num_nodes);

  CompactNode *GetRoot() { return nodes_; }

  // Loading from binary file
  static unique_ptr<CompactTrie> CreateFromBinaryFile(const char *filename);

  // Cleanup
  ~CompactTrie();

 private:
  CompactNode *nodes_;  // mmap'd array of nodes (owned by root only)

  // For memory management
  size_t file_size_;
  int fd_;
};

#endif  // COMPACT_TRIE_H__
