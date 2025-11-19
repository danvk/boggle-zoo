"""Standard command-line arguments share across many tools."""

import argparse

from cpp_boggle import CompactTrie

from boggle.dimensional_bogglers import Bogglers


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


def get_trie_from_args(args: argparse.Namespace):
    dict_path = args.dictionary
    assert dict_path.endswith(".bin")
    t = CompactTrie.create_from_binary_file(dict_path)
    assert t
    return t


def get_trie_and_boggler_from_args(args: argparse.Namespace):
    t = get_trie_from_args(args)
    dims = args.size // 10, args.size % 10

    boggler = Bogglers[dims](t)
    return t, boggler
