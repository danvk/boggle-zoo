#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using std::string;
using std::vector;

#include "boggler.h"
#include "compact_trie.h"
#include "trie.h"

template <int M, int N>
void declare_boggler(py::module &m, const string &pyclass_name) {
  using BB = Boggler<M, N, Trie>;
  py::class_<BB>(m, pyclass_name.c_str())
      .def(py::init<Trie *>())
      .def("score", &BB::Score)
      .def("find_words", &BB::FindWords)
      .def("cell", &BB::Cell)
      .def("set_cell", &BB::SetCell);
}

template <int M, int N>
void declare_compact_boggler(py::module &m, const string &pyclass_name) {
  using BB = Boggler<M, N, CompactTrie>;
  py::class_<BB>(m, pyclass_name.c_str())
      .def(py::init<CompactTrie *>())
      .def("score", &BB::Score)
      .def("find_words", &BB::FindWords)
      .def("cell", &BB::Cell)
      .def("set_cell", &BB::SetCell);
}

PYBIND11_MODULE(cpp_boggle, m) {
  m.doc() = "C++ Boggle Scoring Tools";

  py::class_<Trie>(m, "Trie")
      .def(py::init())
      .def("starts_word", &Trie::StartsWord)
      .def("descend", &Trie::Descend, py::return_value_policy::reference)
      .def("is_word", &Trie::IsWord)
      .def("mark", py::overload_cast<>(&Trie::Mark))
      .def("set_mark", py::overload_cast<uintptr_t>(&Trie::Mark))
      .def("add_word", &Trie::AddWord, py::return_value_policy::reference)
      .def("find_word", &Trie::FindWord, py::return_value_policy::reference)
      .def("size", &Trie::Size)
      .def("num_nodes", &Trie::NumNodes)
      .def("reset_marks", &Trie::ResetMarks)
      .def("set_all_marks", &Trie::SetAllMarks)
      .def_static(
          "reverse_lookup",
          py::overload_cast<const Trie *, const Trie *>(&Trie::ReverseLookup)
      )
      .def_static("create_from_file", &Trie::CreateFromFile)
      .def_static("create_from_wordlist", &Trie::CreateFromWordlist);

  py::class_<CompactTrie>(m, "CompactTrie")
      .def("get_root", &CompactTrie::GetRoot)
      .def_static("create_from_binary_file", &CompactTrie::CreateFromBinaryFile);

  py::class_<CompactNode>(m, "CompactNode")
      .def("starts_word", &CompactNode::StartsWord)
      .def("descend", &CompactNode::Descend, py::return_value_policy::reference)
      .def("is_word", &CompactNode::IsWord)
      .def("mark", py::overload_cast<>(&CompactNode::Mark))
      .def("set_mark", py::overload_cast<uintptr_t>(&CompactNode::SetMark));

  declare_compact_boggler<2, 2>(m, "CompactBoggler22");
  declare_compact_boggler<2, 3>(m, "CompactBoggler23");
  declare_compact_boggler<3, 3>(m, "CompactBoggler33");
  declare_compact_boggler<3, 4>(m, "CompactBoggler34");
  declare_compact_boggler<4, 4>(m, "CompactBoggler44");
  declare_compact_boggler<4, 5>(m, "CompactBoggler45");
  declare_compact_boggler<5, 5>(m, "CompactBoggler55");
}
