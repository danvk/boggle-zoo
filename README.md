# boggle-zoo

Fast Boggle board scoring library. This is a stripped-down version of [hybrid-boggle](https://github.com/danvk/hybrid-boggle) focused solely on scoring Boggle boards as quickly as possible.

## Features

- **Fast C++ implementation** with Python bindings via pybind11
- **Pure Python fallback** for development and testing
- Support for multiple board sizes: 2x2, 2x3, 3x3, 3x4, 4x4, 4x5, 5x5
- Command-line tools for scoring boards and performance testing

## Setup

This project uses [uv](https://github.com/astral-sh/uv) for dependency management.

```bash
# Install dependencies
uv sync --dev

# Build the C++ extension
make

# Run tests
make test
```

## Usage

### Score Boggle Boards

Score one or more boards from stdin:

```bash
# Score a 3x3 board
echo "streaedlp" | uv run python -m boggle.score --size 33
# Output: streaedlp: 545

# Score multiple boards
echo -e "abcdefghi\nstreaedlp" | uv run python -m boggle.score --size 33

# Show all words found (Python implementation only)
echo "streaedlp" | uv run python -m boggle.score --size 33 --python --print_words

# Score a 4x4 board
echo "perslatgsineters" | uv run python -m boggle.score --size 44
# Output: perslatgsineters: 3625
```

### Performance Testing

Test board scoring performance without I/O overhead:

```bash
# Score 100,000 random 3x3 boards
uv run python -m boggle.perf --size 33 100000

# Score 1,000 random 4x4 boards
uv run python -m boggle.perf --size 44 1000

# Use Python implementation (much slower)
uv run python -m boggle.perf --size 33 1000 --python
```

## Board Representation

Boards are represented as strings read column-wise. For a 3x3 board:

```
S T R     -->  "streaedlp"
E A E
A D L
```

For non-square boards like 3x4:

```
A B C     -->  "adgjbehkcfil"
D E F
G H I
J K L
```

The letter "Q" represents "Qu" and is worth 2 letters for scoring purposes.

## Supported Board Sizes

- `--size 22`: 2×2 (4 cells)
- `--size 23`: 2×3 (6 cells)
- `--size 33`: 3×3 (9 cells, classic Boggle)
- `--size 34`: 3×4 (12 cells)
- `--size 44`: 4×4 (16 cells, Big Boggle)
- `--size 45`: 4×5 (20 cells)
- `--size 55`: 5×5 (25 cells)

## Performance

On an M2 MacBook, the C++ implementation can score:
- ~200,000 boards/second for 4×4 boards
- ~100,000 boards/second for 3×3 boards

The Python implementation is approximately 50× slower.

## Dictionary

By default, the `enable2k` word list is used. You can specify a different dictionary with:

```bash
uv run python -m boggle.score --dictionary wordlists/twl06.txt
```

## Development

Build the extension:
```bash
make
```

Run tests:
```bash
make test
```

Format code:
```bash
make format
```

Clean build artifacts:
```bash
make clean
```

## License

Same as hybrid-boggle (check the original repository for details).

## Credits

Extracted from [hybrid-boggle](https://github.com/danvk/hybrid-boggle) by Dan Vanderkam.
