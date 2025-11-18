#!/usr/bin/env python
"""I/O-free performance test.

On my M2 Macbook:

$ poetry run python -m boggle.perf --size 44 1000000 --random_seed 808813
total_score=41134010
4.67s, 214206.42 bds/sec
"""

import argparse
import random
import time
from typing import Sequence

from boggle.args import add_standard_args, get_trie_and_boggler_from_args
from boggle.constants import neighbors


def random_board(n: int, letters: Sequence[str]) -> str:
    return "".join(random.choice(letters) for _ in range(n))


def main():
    parser = argparse.ArgumentParser(
        prog="Boggler perf test",
        description="Measure the speed of board evaluation, free from I/O.",
    )
    add_standard_args(parser, random_seed=True, python=True)
    parser.add_argument(
        "--input_file",
        type=str,
        help="Use this file instead of generating random boards.",
    )
    parser.add_argument(
        "--variations_on",
        type=str,
        help="Generate 1- and 2-cell variations on this board. Useful for "
        "testing performance on high-scoring boards.",
    )
    parser.add_argument(
        "num_boards",
        type=int,
        help="Number of boards to evaluate",
        default=100_000,
        nargs="?",
    )
    parser.add_argument(
        "--jpa14",
        action="store_true",
        help="Generate random boards using a 14-letter alphabet instead of 26.",
    )
    args = parser.parse_args()
    if args.random_seed >= 0:
        random.seed(args.random_seed)

    n = args.num_boards
    t, boggler = get_trie_and_boggler_from_args(args)

    w, h = args.size // 10, args.size % 10
    letters = "acdegilmnoprst" if args.jpa14 else "abcdefghijklmnopqrstuvwxyz"
    if args.variations_on:
        board = args.variations_on
        assert len(board) == w * h
        alphabet = [ord(let) for let in letters]
        boards1 = neighbors(board, alphabet)
        boards = {bd for n1 in boards1 for bd in neighbors(n1, alphabet)}
        boards.update(boards1)
        boards.add(board)
        boards = [*boards]
        print(f"Generated {len(boards)} neighbors of {board}")
    elif args.input_file:
        boards = [line.strip() for line in open(args.input_file)]
        for board in boards:
            assert len(board) == w * h
        print(f"Read {len(boards)} boards from {args.input_file}")
    else:
        print(f"Generating {n} {w}x{h} boards...")
        boards = [random_board(w * h, letters) for _ in range(n)]

    total_score = 0
    print("Scoring boards...")
    start_s = time.time()
    for board in boards:
        total_score += boggler.score(board)
    end_s = time.time()

    elapsed_s = end_s - start_s
    pace = len(boards) / elapsed_s

    print(f"{total_score=}")
    print(f"{elapsed_s:.02f}s, {pace:.02f} bds/sec")


if __name__ == "__main__":
    main()
