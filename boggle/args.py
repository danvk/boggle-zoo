"""Standard command-line arguments share across many tools."""

import argparse

from cpp_boggle import CompactTrie

from boggle.boggler import PyBoggler
from boggle.dimensional_bogglers import Bogglers
from boggle.trie import make_py_trie


def add_standard_args(
    parser: argparse.ArgumentParser, *, random_seed=False, python=False
):
    parser.add_argument(
        "--size",
        type=int,
        choices=(22, 23, 33, 34, 44, 45, 55),
        default=33,
        help="Size of the boggle board.",
    )
    parser.add_argument(
        "--dictionary",
        type=str,
        default="wordlists/enable2k.bin",
        help="Path to dictionary file (.bin for C++, .txt for Python).",
    )

    if random_seed:
        parser.add_argument(
            "--random_seed",
            help="Explicitly set the random seed.",
            type=int,
            default=-1,
        )
    if python:
        parser.add_argument(
            "--python",
            action="store_true",
            help="Use Python implementation instead of C++. This is ~50x slower!",
        )


def get_trie_from_args(args: argparse.Namespace):
    if args.python:
        # Python mode needs .txt dictionary
        dict_path = args.dictionary
        if dict_path.endswith(".bin"):
            dict_path = dict_path.replace(".bin", ".txt")
        t = make_py_trie(dict_path)
        assert t
    else:
        # C++ mode uses CompactTrie with .bin dictionary
        dict_path = args.dictionary
        if not dict_path.endswith(".bin"):
            dict_path = dict_path.replace(".txt", ".bin")
        t = CompactTrie.create_from_binary_file(dict_path)
        assert t
    return t


def get_trie_and_boggler_from_args(args: argparse.Namespace):
    t = get_trie_from_args(args)
    dims = args.size // 10, args.size % 10

    if args.python:
        boggler = PyBoggler(t, dims)
    else:
        boggler = Bogglers[dims](t)
    return t, boggler
