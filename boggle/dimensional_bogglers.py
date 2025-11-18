from cpp_boggle import (
    Boggler22,
    Boggler23,
    Boggler33,
    Boggler34,
    Boggler44,
    Boggler45,
    Boggler55,
)

Bogglers = {
    (2, 2): Boggler22,
    (2, 3): Boggler23,
    (3, 3): Boggler33,
    (3, 4): Boggler34,
    (4, 4): Boggler44,
    (4, 5): Boggler45,
    (5, 5): Boggler55,
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
