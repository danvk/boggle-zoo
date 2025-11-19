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

const int kNumLetters_CT = 26;
const int kQ_CT = 'q' - 'a';

// Binary format node using bitfields (8 bytes total)
struct CompactNode {
  uint64_t child_mask : 26;   // Bitmask for which children exist
  uint64_t is_word : 1;       // 1 if this node represents a complete word
  uint64_t first_child : 21;  // Index of first child (-1 if no children)
  uint64_t mark : 16;         // Mark for tracking during searches
};

// Trie-like interface backed by mmap'd CompactNode array
class CompactTrie {
 public:
  // Constructor for a view into a specific node
  CompactTrie(CompactNode* nodes, size_t num_nodes, size_t node_idx);

  // Fast operations matching old Trie interface
  bool StartsWord(int i) const;
  CompactTrie* Descend(int i) const;

  bool IsWord() const { return nodes_[node_idx_].is_word != 0; }
  void SetIsWord() { /* Not supported for mmap'd data */ }
  void SetWordId(uint32_t word_id) { /* Not supported */ }
  uint32_t WordId() const { return static_cast<uint32_t>(node_idx_); }

  void Mark(uintptr_t m);
  uintptr_t Mark();

  // Loading from binary file
  static unique_ptr<CompactTrie> CreateFromBinaryFile(const char* filename);

  // Stats
  size_t NumNodes() const { return num_nodes_; }

  // Cleanup
  ~CompactTrie();

 private:
  CompactNode* nodes_;     // mmap'd array of nodes (owned by root only)
  size_t num_nodes_;       // Total number of nodes
  size_t node_idx_;        // Index of this node in the array
  bool owns_memory_;       // True for root, false for child views

  // For memory management
  size_t file_size_;
  int fd_;

  // Cache for Descend() calls
  mutable CompactTrie* children_cache_[26];
};

#endif  // COMPACT_TRIE_H__
