#include "compact_trie.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

CompactTrie::CompactTrie(CompactNode *nodes, size_t num_nodes)
    : nodes_(nodes), file_size_(0), fd_(-1) {}

CompactTrie::~CompactTrie() {
  // Clean up mmap'd memory.
  if (nodes_) {
    munmap(nodes_, file_size_);
  }
  if (fd_ >= 0) {
    close(fd_);
  }
}

// Replaces "qu" with "q" in-place; returns true if the word is a valid boggle word
// (IsBoggleWord).
bool IsBoggleWord(const char *wd) {
  int size = strlen(wd);
  if (size < 3) return false;
  for (int i = 0; i < size; ++i) {
    int c = wd[i];
    if (c < 'a' || c > 'z') return false;
    if (c == 'q' && (i + 1 >= size || wd[1 + i] != 'u')) return false;
  }
  return true;
}

bool BogglifyWord(char *word) {
  if (!IsBoggleWord(word)) return false;
  int src, dst;
  for (src = 0, dst = 0; word[src]; src++, dst++) {
    word[dst] = word[src];
    if (word[src] == 'q') src += 1;
  }
  word[dst] = word[src];
  return true;
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

  // Replace a trailing ".bin" with ".txt" and try to load words from that file.
  std::string txt_name = filename;
  const std::string bin_suffix = ".bin";
  assert(txt_name.ends_with(".bin"));
  txt_name.replace(txt_name.size() - bin_suffix.size(), bin_suffix.size(), ".txt");

  FILE *f = fopen(txt_name.c_str(), "r");
  if (f == nullptr) {
    fprintf(stderr, "No corresponding word list found: %s\n", txt_name.c_str());
  } else {
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
      size_t len = strlen(buf);
      while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
      }
      if (len == 0) continue;
      if (BogglifyWord(buf)) {
        root->words_.emplace_back(buf);
      }
    }
    fclose(f);
    fprintf(
        stderr, "Loaded %zu words from %s\n", root->words_.size(), txt_name.c_str()
    );
    fprintf(stderr, "First word is '%s'\n", root->words_[0].c_str());
  }

  return unique_ptr<CompactTrie>(root);
}
