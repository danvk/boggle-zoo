from typing import Sequence

#                  1, 2, 3, 4, 5, 6, 7,  8,     9..25
SCORES = tuple([0, 0, 0, 1, 1, 2, 3, 5, 11] + [11 for _ in range(9, 26)])
assert len(SCORES) == 26
LETTER_A = ord("a")
LETTER_Q = ord("q") - LETTER_A
LETTER_Z = ord("z")

A_TO_Z = [*range(LETTER_A, LETTER_Z + 1)]


def neighbors(board: str, valid_letters: Sequence[int]):
    """Find all boards within a distance of 1 of this one.

    This includes:
    - Changing any one cell to another letter.
    - Swapping any two cells.
    """
    out = set()
    out.add(board)

    # single letter changes
    for i, c in enumerate(board):
        current = ord(c)
        prefix = board[:i]
        suffix = board[i + 1 :]
        for j in valid_letters:
            if j == current:
                continue
            n = prefix + chr(j) + suffix
            out.add(n)

    # letter swaps
    for i in range(len(board) - 1):
        prefix = board[:i]
        ci = board[i]
        for j in range(i + 1, len(board)):
            cj = board[j]
            n = prefix + cj + board[i + 1 : j] + ci + board[j + 1 :]
            out.add(n)

    return out
