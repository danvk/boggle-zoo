#ifndef PREFIX_DICTIONARY_H__
#define PREFIX_DICTIONARY_H__

#include <stdint.h>
#include <sys/types.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

const int kNumLetters = 26;
const int kQ = 'q' - 'a';

class PrefixDictionary
{
public:
  PrefixDictionary();
  ~PrefixDictionary();

  // Query operations
  bool HasPrefix(const string &prefix) const;
  bool IsWord(const string &word) const;
  int GetWordId(const string &word) const;

  // Mark tracking for scoring
  void MarkWord(const string &word, uintptr_t mark);
  uintptr_t GetMark(const string &word) const;
  void ResetMarks();

  // Construction
  void AddWord(const string &word, int word_id);
  static unique_ptr<PrefixDictionary> CreateFromFile(const char *filename);
  static unique_ptr<PrefixDictionary> CreateFromFileStr(const string &filename);
  static unique_ptr<PrefixDictionary> CreateFromWordlist(const vector<string> &words);

  // Stats
  size_t NumWords() const { return words_.size(); }
  size_t NumPrefixes() const { return prefixes_.size(); }

  // Utility methods
  static bool BogglifyWord(char *word);
  static bool IsBoggleWord(const char *word);

private:
  unordered_set<string> prefixes_;   // All valid word prefixes
  unordered_map<string, int> words_; // word -> word_id mapping
  vector<uintptr_t> marks_;          // marks_[word_id] = mark value
};

#endif // PREFIX_DICTIONARY_H__
