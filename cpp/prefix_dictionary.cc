#include "prefix_dictionary.h"

#include <cstring>
#include <iostream>

using namespace std;

// Global counter for tracking memory usage
static size_t g_dict_bytes_allocated = 0;

// Helper function to format bytes in human-readable form
static string FormatBytes(size_t bytes) {
  const char* units[] = {"B", "KB", "MB", "GB"};
  int unit_idx = 0;
  double size = static_cast<double>(bytes);

  while (size >= 1024.0 && unit_idx < 3) {
    size /= 1024.0;
    unit_idx++;
  }

  char buffer[64];
  if (unit_idx == 0) {
    snprintf(buffer, sizeof(buffer), "%zu %s", bytes, units[unit_idx]);
  } else {
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit_idx]);
  }
  return string(buffer);
}

PrefixDictionary::PrefixDictionary() {
  g_dict_bytes_allocated += sizeof(PrefixDictionary);
}

PrefixDictionary::~PrefixDictionary() {
  g_dict_bytes_allocated -= sizeof(PrefixDictionary);
}

bool PrefixDictionary::HasPrefix(const string& prefix) const {
  return prefixes_.find(prefix) != prefixes_.end();
}

bool PrefixDictionary::IsWord(const string& word) const {
  return words_.find(word) != words_.end();
}

int PrefixDictionary::GetWordId(const string& word) const {
  auto it = words_.find(word);
  if (it != words_.end()) {
    return it->second;
  }
  return -1;
}

void PrefixDictionary::MarkWord(const string& word, uintptr_t mark) {
  int word_id = GetWordId(word);
  if (word_id >= 0 && word_id < static_cast<int>(marks_.size())) {
    marks_[word_id] = mark;
  }
}

uintptr_t PrefixDictionary::GetMark(const string& word) const {
  int word_id = GetWordId(word);
  if (word_id >= 0 && word_id < static_cast<int>(marks_.size())) {
    return marks_[word_id];
  }
  return 0;
}

void PrefixDictionary::ResetMarks() {
  for (auto& mark : marks_) {
    mark = 0;
  }
}

void PrefixDictionary::AddWord(const string& word, int word_id) {
  // Add all prefixes
  string prefix;
  for (char c : word) {
    prefix += c;
    prefixes_.insert(prefix);
  }

  // Add the complete word
  words_[word] = word_id;

  // Ensure marks vector is large enough
  if (marks_.size() <= static_cast<size_t>(word_id)) {
    marks_.resize(word_id + 1, 0);
  }
}

unique_ptr<PrefixDictionary> PrefixDictionary::CreateFromFile(const char* filename) {
  char line[80];
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    return nullptr;
  }

  size_t bytes_before = g_dict_bytes_allocated;
  int count = 0;
  unique_ptr<PrefixDictionary> dict(new PrefixDictionary);

  while (!feof(f) && fscanf(f, "%s", line)) {
    if (BogglifyWord(line)) {
      dict->AddWord(line, count++);
    }
  }
  fclose(f);

  size_t bytes_used = g_dict_bytes_allocated - bytes_before;

  // Account for the actual memory used by the hash table contents
  size_t prefix_mem = 0;
  for (const auto& prefix : dict->prefixes_) {
    prefix_mem += prefix.capacity() + sizeof(string);
  }
  prefix_mem += dict->prefixes_.bucket_count() * sizeof(void*);

  size_t words_mem = 0;
  for (const auto& pair : dict->words_) {
    words_mem += pair.first.capacity() + sizeof(string) + sizeof(int);
  }
  words_mem += dict->words_.bucket_count() * sizeof(void*);

  size_t marks_mem = dict->marks_.capacity() * sizeof(uintptr_t);

  size_t total_mem = bytes_used + prefix_mem + words_mem + marks_mem;

  fprintf(
      stderr,
      "Loaded %d words with %zu prefixes using %s (dict: %s, prefixes: %s, words: %s, marks: %s)\n",
      count,
      dict->NumPrefixes(),
      FormatBytes(total_mem).c_str(),
      FormatBytes(bytes_used).c_str(),
      FormatBytes(prefix_mem).c_str(),
      FormatBytes(words_mem).c_str(),
      FormatBytes(marks_mem).c_str());

  return dict;
}

unique_ptr<PrefixDictionary> PrefixDictionary::CreateFromFileStr(const string& filename) {
  return CreateFromFile(filename.c_str());
}

unique_ptr<PrefixDictionary> PrefixDictionary::CreateFromWordlist(const vector<string>& words) {
  size_t bytes_before = g_dict_bytes_allocated;
  int count = 0;
  unique_ptr<PrefixDictionary> dict(new PrefixDictionary);

  for (const auto& word : words) {
    dict->AddWord(word, count++);
  }

  size_t bytes_used = g_dict_bytes_allocated - bytes_before;

  // Account for the actual memory used by the hash table contents
  size_t prefix_mem = 0;
  for (const auto& prefix : dict->prefixes_) {
    prefix_mem += prefix.capacity() + sizeof(string);
  }
  prefix_mem += dict->prefixes_.bucket_count() * sizeof(void*);

  size_t words_mem = 0;
  for (const auto& pair : dict->words_) {
    words_mem += pair.first.capacity() + sizeof(string) + sizeof(int);
  }
  words_mem += dict->words_.bucket_count() * sizeof(void*);

  size_t marks_mem = dict->marks_.capacity() * sizeof(uintptr_t);

  size_t total_mem = bytes_used + prefix_mem + words_mem + marks_mem;

  fprintf(
      stderr,
      "Loaded %d words with %zu prefixes using %s (dict: %s, prefixes: %s, words: %s, marks: %s)\n",
      count,
      dict->NumPrefixes(),
      FormatBytes(total_mem).c_str(),
      FormatBytes(bytes_used).c_str(),
      FormatBytes(prefix_mem).c_str(),
      FormatBytes(words_mem).c_str(),
      FormatBytes(marks_mem).c_str());

  return dict;
}

bool PrefixDictionary::IsBoggleWord(const char* wd) {
  int size = strlen(wd);
  if (size < 3) return false;
  for (int i = 0; i < size; ++i) {
    int c = wd[i];
    if (c < 'a' || c > 'z') return false;
    if (c == 'q' && (i + 1 >= size || wd[1 + i] != 'u')) return false;
  }
  return true;
}

bool PrefixDictionary::BogglifyWord(char* word) {
  if (!IsBoggleWord(word)) return false;
  int src, dst;
  for (src = 0, dst = 0; word[src]; src++, dst++) {
    word[dst] = word[src];
    if (word[src] == 'q') src += 1;
  }
  word[dst] = word[src];
  return true;
}
