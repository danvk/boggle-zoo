#include "compact_trie.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

CompactTrie::CompactTrie(CompactNode* nodes, size_t num_nodes, size_t node_idx)
    : nodes_(nodes),
      num_nodes_(num_nodes),
      node_idx_(node_idx),
      owns_memory_(false),
      file_size_(0),
      fd_(-1) {
  for (int i = 0; i < 26; i++) {
    children_cache_[i] = nullptr;
  }
}

CompactTrie::~CompactTrie() {
  // Clean up cached children
  for (int i = 0; i < 26; i++) {
    delete children_cache_[i];
  }

  // Clean up mmap'd memory if we own it (root node)
  if (owns_memory_) {
    if (nodes_) {
      munmap(nodes_, file_size_);
    }
    if (fd_ >= 0) {
      close(fd_);
    }
  }
}

bool CompactTrie::StartsWord(int letter_idx) const {
  if (letter_idx < 0 || letter_idx >= 26) {
    return false;
  }

  const CompactNode& node = nodes_[node_idx_];
  uint32_t letter_bit = 1u << letter_idx;
  return (node.child_mask & letter_bit) != 0;
}

CompactTrie* CompactTrie::Descend(int letter_idx) const {
  if (letter_idx < 0 || letter_idx >= 26) {
    return nullptr;
  }

  // Check cache first
  if (children_cache_[letter_idx]) {
    return children_cache_[letter_idx];
  }

  const CompactNode& node = nodes_[node_idx_];

  // Check if this letter exists
  uint32_t letter_bit = 1u << letter_idx;
  if (!(node.child_mask & letter_bit)) {
    return nullptr;
  }

  // Count how many children come before this letter
  uint32_t mask_before = node.child_mask & (letter_bit - 1);
  int child_offset = __builtin_popcount(mask_before);

  // Calculate child index
  int child_idx = node.first_child + child_offset;
  if (child_idx < 0 || child_idx >= static_cast<int>(num_nodes_)) {
    return nullptr;
  }

  // Create and cache the child view
  CompactTrie* child = new CompactTrie(nodes_, num_nodes_, child_idx);
  children_cache_[letter_idx] = child;
  return child;
}

void CompactTrie::Mark(uintptr_t m) {
  nodes_[node_idx_].mark = static_cast<uint64_t>(m & 0xFFFF);
}

uintptr_t CompactTrie::Mark() {
  return nodes_[node_idx_].mark;
}

unique_ptr<CompactTrie> CompactTrie::CreateFromBinaryFile(const char* filename) {
  // Open file
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Failed to open binary dictionary: %s\n", filename);
    return nullptr;
  }

  // Get file size
  struct stat st;
  if (fstat(fd, &st) < 0) {
    fprintf(stderr, "Failed to stat binary dictionary: %s\n", filename);
    close(fd);
    return nullptr;
  }

  size_t file_size = st.st_size;
  if (file_size % sizeof(CompactNode) != 0) {
    fprintf(
        stderr,
        "Invalid binary dictionary size: %zu bytes (not a multiple of %zu)\n",
        file_size,
        sizeof(CompactNode)
    );
    close(fd);
    return nullptr;
  }

  size_t num_nodes = file_size / sizeof(CompactNode);
  if (num_nodes == 0) {
    fprintf(stderr, "Empty binary dictionary\n");
    close(fd);
    return nullptr;
  }

  // mmap the file with PROT_WRITE for copy-on-write mark updates
  void* mapped = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    fprintf(stderr, "Failed to mmap binary dictionary: %s\n", filename);
    close(fd);
    return nullptr;
  }

  CompactNode* nodes = static_cast<CompactNode*>(mapped);

  fprintf(
      stderr,
      "Loaded binary trie: %zu nodes (%zu bytes, %.2f MB)\n",
      num_nodes,
      file_size,
      file_size / (1024.0 * 1024.0)
  );

  // Create root CompactTrie
  CompactTrie* root = new CompactTrie(nodes, num_nodes, 0);
  root->owns_memory_ = true;
  root->file_size_ = file_size;
  root->fd_ = fd;

  return unique_ptr<CompactTrie>(root);
}
