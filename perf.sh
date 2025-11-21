#!/usr/bin/env bash

echo "JPA14 5x5 good"
uv run boggle/perf.py --dictionary wordlists/twl06.jpa14.bin --size 55 --input_file good20k.txt

echo "JPA14 5x5 random"
uv run boggle/perf.py --dictionary wordlists/twl06.jpa14.bin --size 55 --random_seed 2025 --jpa

echo "TWL06 5x5 good"
uv run boggle/perf.py --dictionary wordlists/twl06.bin --size 55 --input_file good20k.txt

echo "TWL06 5x5 random"
uv run boggle/perf.py --dictionary wordlists/twl06.bin --size 55 --random_seed 2025
