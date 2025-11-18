#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using std::string;
using std::vector;

#include "boggler.h"
#include "prefix_dictionary.h"

template <int M, int N>
void declare_boggler(py::module &m, const string &pyclass_name) {
  using BB = Boggler<M, N>;
  py::class_<BB>(m, pyclass_name.c_str())
      .def(py::init<PrefixDictionary *>())
      .def("score", &BB::Score)
      .def("find_words", &BB::FindWords)
      .def("cell", &BB::Cell)
      .def("set_cell", &BB::SetCell);
}

PYBIND11_MODULE(cpp_boggle, m) {
  m.doc() = "C++ Boggle Scoring Tools";

  py::class_<PrefixDictionary>(m, "Trie")  // Keep old name for compatibility
      .def(py::init())
      .def("has_prefix", &PrefixDictionary::HasPrefix)
      .def("is_word", &PrefixDictionary::IsWord)
      .def("get_word_id", &PrefixDictionary::GetWordId)
      .def("mark_word", &PrefixDictionary::MarkWord)
      .def("get_mark", &PrefixDictionary::GetMark)
      .def("reset_marks", &PrefixDictionary::ResetMarks)
      .def("add_word", &PrefixDictionary::AddWord)
      .def("size", &PrefixDictionary::NumWords)
      .def("num_nodes", &PrefixDictionary::NumPrefixes)
      .def_static("create_from_file", &PrefixDictionary::CreateFromFile)
      .def_static("create_from_wordlist", &PrefixDictionary::CreateFromWordlist);

  declare_boggler<2, 2>(m, "Boggler22");
  declare_boggler<2, 3>(m, "Boggler23");
  declare_boggler<3, 3>(m, "Boggler33");
  declare_boggler<3, 4>(m, "Boggler34");
  declare_boggler<4, 4>(m, "Boggler44");
  declare_boggler<4, 5>(m, "Boggler45");
  declare_boggler<5, 5>(m, "Boggler55");
}
