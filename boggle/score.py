#!/usr/bin/env python
"""Score boggle boards."""

import argparse
import fileinput
import sys
import time

from boggle.args import add_standard_args, get_trie_and_boggler_from_args


def main():
    parser = argparse.ArgumentParser(description="Score boggle boards")
    add_standard_args(parser, python=True)
    parser.add_argument(
        "files", metavar="FILE", nargs="*", help="Files containing boards, or stdin"
    )

    args = parser.parse_args()
    _, boggler = get_trie_and_boggler_from_args(args)

    start_s = time.time()
    n = 0
    for line in fileinput.input(files=args.files):
        board = line.strip()
        score = boggler.score(board)
        print(f"{board}: {score}")
        n += 1
    end_s = time.time()
    elapsed_s = end_s - start_s
    rate = n / elapsed_s
    sys.stderr.write(f"{n} boards in {elapsed_s:.2f}s = {rate:.2f} boards/s\n")


if __name__ == "__main__":
    main()
