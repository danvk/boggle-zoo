from cpp_boggle import (
    CompactBoggler22,
    CompactBoggler23,
    CompactBoggler33,
    CompactBoggler34,
    CompactBoggler44,
    CompactBoggler45,
    CompactBoggler55,
)

Bogglers = {
    (2, 2): CompactBoggler22,
    (2, 3): CompactBoggler23,
    (3, 3): CompactBoggler33,
    (3, 4): CompactBoggler34,
    (4, 4): CompactBoggler44,
    (4, 5): CompactBoggler45,
    (5, 5): CompactBoggler55,
}


# Matches PyBoggler constructor
def cpp_boggler(t, dims):
    return Bogglers[dims](t)


LEN_TO_DIMS = {
    4: (2, 2),
    6: (2, 3),
    9: (3, 3),
    12: (3, 4),
    16: (4, 4),
    20: (4, 5),
    25: (5, 5),
}
