#include <iostream>
#include "compact_dictionary.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <binary_dict_file>\n", argv[0]);
    return 1;
  }

  auto dict = CompactDictionary::CreateFromBinaryFile(argv[1]);
  if (!dict) {
    fprintf(stderr, "Failed to load dictionary\n");
    return 1;
  }

  printf("Loaded dictionary with %zu nodes\n", dict->NumNodes());

  // Test some words
  const char* test_words[] = {
      "wood", "woxd", "tea", "teapot", "agriculture", "quinquennia", nullptr
  };

  printf("\nTesting word lookups:\n");
  for (int i = 0; test_words[i]; i++) {
    const char* word = test_words[i];
    bool is_word = dict->IsWord(word);
    printf("  %-15s: %s\n", word, is_word ? "YES" : "NO");
  }

  // Test prefix lookups
  const char* test_prefixes[] = {"te", "wood", "wox", "agri", "xyz", nullptr};

  printf("\nTesting prefix lookups:\n");
  for (int i = 0; test_prefixes[i]; i++) {
    const char* prefix = test_prefixes[i];
    bool has_prefix = dict->HasPrefix(prefix);
    printf("  %-15s: %s\n", prefix, has_prefix ? "YES" : "NO");
  }

  // Test mark operations
  printf("\nTesting mark operations:\n");
  dict->MarkWord("tea", 12345);
  printf("  Mark for 'tea': %lu\n", dict->GetMark("tea"));
  printf("  Mark for 'wood': %lu\n", dict->GetMark("wood"));

  return 0;
}
