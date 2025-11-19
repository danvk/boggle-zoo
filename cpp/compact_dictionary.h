#ifndef COMPACT_DICTIONARY_H__
#define COMPACT_DICTIONARY_H__

#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

const int kNumLetters = 26;
const int kQ = 'q' - 'a';

// Binary format node matching Python CompactNode (without tracking_)
struct Node {
  uint32_t child_mask;  // Bitmask for which children exist (26 bits)
  int32_t first_child;  // Index of first child in nodes array (-1 if no children)
  uint8_t is_word;      // 1 if this node represents a complete word
  uint8_t padding[3];   // Padding for alignment

  // Total: 12 bytes per node
};

class CompactDictionary {
 public:
  ~CompactDictionary();

  // Query operations - traverse the node array
  bool HasPrefix(const string& prefix) const;
  bool IsWord(const string& word) const;

  // Mark tracking - separate from nodes
  void MarkWord(const string& word, uintptr_t mark);
  uintptr_t GetMark(const string& word) const;
  void ResetMarks();

  // Load from binary file using mmap
  static unique_ptr<CompactDictionary> CreateFromBinaryFile(const char* filename);

  // Stats
  size_t NumNodes() const { return num_nodes_; }

 private:
  CompactDictionary(Node* nodes, size_t num_nodes, size_t file_size, int fd);

  // Helper to traverse to a node given a string
  const Node* FindNode(const string& str) const;

  // Descend one letter from a node
  const Node* Descend(const Node* node, int letter_idx) const;

  Node* nodes_;           // mmap'd array of nodes
  size_t num_nodes_;      // Number of nodes in the array
  size_t file_size_;      // Size of mmap'd file
  int fd_;                // File descriptor for mmap

  // Mark tracking (word -> mark)
  mutable unordered_map<string, uintptr_t> marks_;
};

#endif  // COMPACT_DICTIONARY_H__
