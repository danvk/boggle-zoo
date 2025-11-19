#include "compact_dictionary.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

using namespace std;

CompactDictionary::CompactDictionary(
    Node* nodes, size_t num_nodes, size_t file_size, int fd
)
    : nodes_(nodes), num_nodes_(num_nodes), file_size_(file_size), fd_(fd) {}

CompactDictionary::~CompactDictionary() {
  if (nodes_) {
    munmap(nodes_, file_size_);
  }
  if (fd_ >= 0) {
    close(fd_);
  }
}

const Node* CompactDictionary::Descend(const Node* node, int letter_idx) const {
  if (!node) return nullptr;

  // Check if this letter exists in the child mask
  uint32_t letter_bit = 1u << letter_idx;
  if (!(node->child_mask & letter_bit)) {
    return nullptr;
  }

  // Count how many children come before this letter
  uint32_t mask_before = node->child_mask & (letter_bit - 1);
  int child_offset = __builtin_popcount(mask_before);

  // Return the child node
  int child_idx = node->first_child + child_offset;
  if (child_idx < 0 || child_idx >= static_cast<int>(num_nodes_)) {
    return nullptr;
  }

  return &nodes_[child_idx];
}

const Node* CompactDictionary::FindNode(const string& str) const {
  const Node* current = &nodes_[0];  // Start at root

  for (char c : str) {
    if (c < 'a' || c > 'z') {
      return nullptr;
    }
    int letter_idx = c - 'a';
    current = Descend(current, letter_idx);
    if (!current) {
      return nullptr;
    }
  }

  return current;
}

bool CompactDictionary::HasPrefix(const string& prefix) const {
  if (prefix.empty()) {
    return true;  // Empty prefix always exists
  }
  return FindNode(prefix) != nullptr;
}

bool CompactDictionary::IsWord(const string& word) const {
  if (word.empty()) {
    return nodes_[0].is_word != 0;  // Check if root is a word
  }
  const Node* node = FindNode(word);
  return node && node->is_word;
}

void CompactDictionary::MarkWord(const string& word, uintptr_t mark) {
  marks_[word] = mark;
}

uintptr_t CompactDictionary::GetMark(const string& word) const {
  auto it = marks_.find(word);
  if (it != marks_.end()) {
    return it->second;
  }
  return 0;
}

void CompactDictionary::ResetMarks() {
  marks_.clear();
}

unique_ptr<CompactDictionary> CompactDictionary::CreateFromBinaryFile(const char* filename) {
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
  if (file_size % sizeof(Node) != 0) {
    fprintf(
        stderr,
        "Invalid binary dictionary size: %zu bytes (not a multiple of %zu)\n",
        file_size,
        sizeof(Node)
    );
    close(fd);
    return nullptr;
  }

  size_t num_nodes = file_size / sizeof(Node);
  if (num_nodes == 0) {
    fprintf(stderr, "Empty binary dictionary\n");
    close(fd);
    return nullptr;
  }

  // mmap the file
  void* mapped = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    fprintf(stderr, "Failed to mmap binary dictionary: %s\n", filename);
    close(fd);
    return nullptr;
  }

  Node* nodes = static_cast<Node*>(mapped);

  fprintf(
      stderr,
      "Loaded binary dictionary: %zu nodes (%zu bytes, %.2f MB)\n",
      num_nodes,
      file_size,
      file_size / (1024.0 * 1024.0)
  );

  return unique_ptr<CompactDictionary>(
      new CompactDictionary(nodes, num_nodes, file_size, fd)
  );
}
