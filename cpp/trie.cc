#include "trie.h"

#include <stdlib.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <utility>

using namespace std;

static inline int idx(char x) { return x - 'a'; }

// Global counter for tracking Trie memory usage
static size_t g_trie_bytes_allocated = 0;

// Helper function to format bytes in human-readable form
static string FormatBytes(size_t bytes)
{
  const char *units[] = {"B", "KB", "MB", "GB"};
  int unit_idx = 0;
  double size = static_cast<double>(bytes);

  while (size >= 1024.0 && unit_idx < 3)
  {
    size /= 1024.0;
    unit_idx++;
  }

  char buffer[64];
  if (unit_idx == 0)
  {
    snprintf(buffer, sizeof(buffer), "%zu %s", bytes, units[unit_idx]);
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit_idx]);
  }
  return string(buffer);
}

// Initially, this node is empty
Trie::Trie()
{
  for (int i = 0; i < kNumLetters; i++)
    children_[i] = NULL;
  is_word_ = false;
  mark_ = 0;
  g_trie_bytes_allocated += sizeof(Trie);
}

Trie *Trie::AddWord(const char *wd)
{
  if (!wd)
    return NULL;
  if (!*wd)
  {
    SetIsWord();
    return this;
  }
  int c = idx(*wd);
  if (!StartsWord(c))
    children_[c] = new Trie;
  return Descend(c)->AddWord(wd + 1);
}

Trie::~Trie()
{
  for (int i = 0; i < kNumLetters; i++)
  {
    if (children_[i])
      delete children_[i];
  }
  g_trie_bytes_allocated -= sizeof(Trie);
}

size_t Trie::Size()
{
  size_t size = 0;
  if (IsWord())
    size++;
  for (int i = 0; i < kNumLetters; i++)
  {
    if (StartsWord(i))
      size += Descend(i)->Size();
  }
  return size;
}

size_t Trie::NumNodes()
{
  int count = 1;
  for (int i = 0; i < kNumLetters; i++)
    if (StartsWord(i))
      count += Descend(i)->NumNodes();
  return count;
}

// static
bool Trie::ReverseLookup(const Trie *base, const Trie *child, string *out)
{
  if (base == child)
    return true;
  for (int i = 0; i < kNumLetters; i++)
  {
    if (base->StartsWord(i) && ReverseLookup(base->Descend(i), child, out))
    {
      *out = string(1, 'a' + i) + *out;
      return true;
    }
  }
  return false;
}

void Trie::ResetMarks() { SetAllMarks(0); }

// static
string Trie::ReverseLookup(const Trie *base, const Trie *child)
{
  string out;
  ReverseLookup(base, child, &out);
  return out;
}

void Trie::SetAllMarks(unsigned mark)
{
  if (IsWord())
    Mark(mark);
  for (int i = 0; i < kNumLetters; i++)
  {
    if (StartsWord(i))
      Descend(i)->SetAllMarks(mark);
  }
}

Trie *Trie::FindWord(const char *wd)
{
  if (!wd)
    return NULL;
  if (!*wd)
  {
    return IsWord() ? this : NULL;
  }
  int c = idx(*wd);
  if (!StartsWord(c))
    return NULL;
  return Descend(c)->FindWord(wd + 1);
}

Trie *Trie::FindWordId(int word_id)
{
  if (IsWord() && word_id_ == word_id)
  {
    return this;
  }
  for (const auto &c : children_)
  {
    if (!c)
      continue;
    auto t = c->FindWordId(word_id);
    if (t)
    {
      return t;
    }
  }
  return nullptr;
}

unique_ptr<Trie> Trie::CreateFromFile(const char *filename)
{
  char line[80];
  FILE *f = fopen(filename, "r");
  if (!f)
  {
    fprintf(stderr, "Couldn't open %s\n", filename);
    return NULL;
  }

  size_t bytes_before = g_trie_bytes_allocated;
  int count = 0;
  unique_ptr<Trie> t(new Trie);
  while (!feof(f) && fscanf(f, "%s", line))
  {
    if (BogglifyWord(line))
    {
      t->AddWord(line)->SetWordId(count++);
    }
  }
  fclose(f);

  size_t bytes_used = g_trie_bytes_allocated - bytes_before;
  size_t num_nodes = t->NumNodes();
  fprintf(
      stderr,
      "Loaded %d words into Trie with %zu nodes using %zu bytes %s (%zu bytes per node)\n",
      count,
      num_nodes,
      bytes_used,
      FormatBytes(bytes_used).c_str(),
      bytes_used / num_nodes);

  return t;
}

unique_ptr<Trie> Trie::CreateFromFileStr(const string &filename)
{
  return CreateFromFile(filename.c_str());
}

/* static */ bool Trie::IsBoggleWord(const char *wd)
{
  int size = strlen(wd);
  if (size < 3)
    return false;
  for (int i = 0; i < size; ++i)
  {
    int c = wd[i];
    if (c < 'a' || c > 'z')
      return false;
    if (c == 'q' && (i + 1 >= size || wd[1 + i] != 'u'))
      return false;
  }
  return true;
}

/* static */ bool Trie::BogglifyWord(char *word)
{
  if (!IsBoggleWord(word))
    return false;
  int src, dst;
  for (src = 0, dst = 0; word[src]; src++, dst++)
  {
    word[dst] = word[src];
    if (word[src] == 'q')
      src += 1;
  }
  word[dst] = word[src];
  return true;
}

/* static */ unique_ptr<Trie> Trie::CreateFromWordlist(const vector<string> &words)
{
  size_t bytes_before = g_trie_bytes_allocated;
  int count = 0;
  unique_ptr<Trie> t(new Trie);
  for (const auto &word : words)
  {
    t->AddWord(word.c_str())->SetWordId(count++);
  }

  size_t bytes_used = g_trie_bytes_allocated - bytes_before;
  size_t num_nodes = t->NumNodes();
  fprintf(
      stderr,
      "Loaded %d words into Trie with %zu nodes using %s (%zu bytes per node)\n",
      count,
      num_nodes,
      FormatBytes(bytes_used).c_str(),
      bytes_used / num_nodes);

  return t;
}
