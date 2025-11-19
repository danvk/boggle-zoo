#include "compact_trie.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

CompactTrie::CompactTrie(CompactNode *nodes, size_t num_nodes)
    : nodes_(nodes), num_nodes_(num_nodes), file_size_(0), fd_(-1) {}

CompactTrie::~CompactTrie() {
  // Clean up mmap'd memory.
  if (nodes_) {
    munmap(nodes_, file_size_);
  }
  if (fd_ >= 0) {
    close(fd_);
  }
}

unique_ptr<CompactTrie> CompactTrie::CreateFromBinaryFile(const char *filename) {
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
  void *mapped = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    fprintf(stderr, "Failed to mmap binary dictionary: %s\n", filename);
    close(fd);
    return nullptr;
  }

  CompactNode *nodes = static_cast<CompactNode *>(mapped);

  fprintf(
      stderr,
      "Loaded binary trie: %zu nodes (%zu bytes, %.2f MB)\n",
      num_nodes,
      file_size,
      file_size / (1024.0 * 1024.0)
  );

  // Create root CompactTrie
  CompactTrie *root = new CompactTrie(nodes, num_nodes);
  root->file_size_ = file_size;
  root->fd_ = fd;

  return unique_ptr<CompactTrie>(root);
}
