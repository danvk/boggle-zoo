#!/usr/bin/env bash
for dict in wordlists/*.txt; do
    set -x
    uv run boggle/encode_dict.py $dict --binary ${dict/txt/bin}
    set +x
done