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

// Binary format node using bitfields (8 bytes total)
struct CompactNode {
  uint64_t child_mask : 26;   // Bitmask for which children exist
  uint64_t is_word : 1;       // 1 if this node represents a complete word
  uint64_t first_child : 21;  // Offset to first child
  uint64_t mark : 16;         // Mark for tracking during searches

  // Fast operations matching old Trie interface
  bool StartsWord(int i) const { return child_mask & (1 << i); }
  bool IsWord() const { return is_word; }
  CompactNode *Descend(int i) {
    uint32_t letter_bit = 1u << i;
    if (!(child_mask & letter_bit)) {
      return nullptr;
    }
    uint32_t mask_before = child_mask & (letter_bit - 1);
    int child_offset = std::popcount(mask_before);
    CompactNode *child = this + first_child + child_offset;
    return child;
  };

  void SetMark(uintptr_t m) { mark = m; }
  uintptr_t Mark() { return mark; }
};

// Trie-like interface backed by mmap'd CompactNode array
class CompactTrie {
 public:
  // Constructor for a view into a specific node
  CompactTrie(CompactNode *nodes, size_t num_nodes);

  CompactNode *GetRoot() { return nodes_; }

  // Loading from binary file
  static unique_ptr<CompactTrie> CreateFromBinaryFile(const char *filename);

  // Stats
  size_t NumNodes() const { return num_nodes_; }

  // Cleanup
  ~CompactTrie();

 private:
  CompactNode *nodes_;  // mmap'd array of nodes (owned by root only)
  size_t num_nodes_;    // Total number of nodes

  // For memory management
  size_t file_size_;
  int fd_;
};

#endif  // COMPACT_TRIE_H__
