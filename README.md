# boggle-zoo

Experiments with different data structures for scoring Boggle boards. Each branch in this repo has a different data structure. These define the memory/speed tradeoff.

See [jpa-boggle](https://github.com/danvk/jpa-boggle) for a write-up of the results. The ultimate goal is to understand JohnPaul Adamovsky's [ADTDAWG] data structure, and whether it represents a significant advance over a more standard [Trie]. (TL;DR: it doesn't.)

Implementations:

- hash_map
- Indexed trie
- Popcount trie
- Pure DAWG ("multiboggle" only -- no word de-duping)
- DAWG + count below
- DAWG + tracking number
- ADTDAWG

## Setup

This project uses [uv](https://github.com/astral-sh/uv) for dependency management.

```bash
# Install dependencies
uv sync --dev

# Build the C++ extension
make

# Run tests
make test

# Generate binary-encoded dictionaries
./encode_all.sh

# Run performance tests
./perf.sh
```

## Usage

### Score Boggle Boards

Score one or more boards from stdin:

```bash
# Score a 3x3 board
echo "streaedlp" | uv run python -m boggle.score --size 33
# Output: streaedlp: 545

# Score a 4x4 board
echo "perslatgsineters" | uv run python -m boggle.score --size 44
# Output: perslatgsineters: 3625
```

### Performance Testing

```bash
# Generate binary-encoded dictionaries
./encode_all.sh

# Run performance tests
./perf.sh
```

This reports RAM usage and performance numbers (boards/sec) on random and "good" boards (boards that are variations on the highest-scoring board).

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

## Credits

Extracted from [hybrid-boggle](https://github.com/danvk/hybrid-boggle) by Dan Vanderkam.
