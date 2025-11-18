#!/bin/bash

c++ --version | grep Apple > /dev/null
if [[ $? == 0 ]]; then
    EXTRA_FLAGS="-undefined dynamic_lookup"
else
    EXTRA_FLAGS=""
fi

set -o errexit
cd cpp
c++ -Wall -shared -std=c++20 -fPIC -march=native \
    -Wno-sign-compare \
    -Wshadow \
    -Werror \
    -O3 \
    $(uv run python -m pybind11 --includes | perl -pe 's/-I/-isystem /g') \
    cpp_boggle.cc trie.cc \
    -o ../cpp_boggle$(python3-config --extension-suffix) \
    $EXTRA_FLAGS
