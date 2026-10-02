#!/usr/bin/env python3
"""Assign generated slice sources to stable, bounded CMake unity groups."""

from __future__ import annotations

from collections import defaultdict
import hashlib
from pathlib import Path
import sys


ROOT_GROUPS = 896
MAX_GROUP_SIZE = 32


def _digest(label: bytes, source: str) -> int:
    return int.from_bytes(hashlib.sha256(label + source.encode()).digest(), "big")


def stable_root(source: str, root_groups: int = ROOT_GROUPS) -> int:
    if root_groups < 1:
        raise ValueError("the unity root-group count must be positive")
    return _digest(b"root\0", source) % root_groups


def group_sources(sources: list[str], root_groups: int = ROOT_GROUPS,
                  max_group_size: int = MAX_GROUP_SIZE) -> dict[str, str]:
    if max_group_size < 1:
        raise ValueError("the unity group-size limit must be positive")
    if len(sources) != len(set(sources)):
        raise ValueError("the slice contains a duplicate source")

    roots: dict[int, list[tuple[str, int]]] = defaultdict(list)
    for source in sources:
        roots[stable_root(source, root_groups)].append(
            (source, _digest(b"split\0", source)))

    assigned: dict[str, str] = {}

    def split(root: int, entries: list[tuple[str, int]], depth: int, suffix: str) -> None:
        if len(entries) <= max_group_size:
            tail = suffix or "root"
            for source, _ in entries:
                assigned[source] = f"stable/{root:03d}/{tail}"
            return
        zero: list[tuple[str, int]] = []
        one: list[tuple[str, int]] = []
        for entry in entries:
            (one if (entry[1] >> depth) & 1 else zero).append(entry)
        if not zero or not one:
            split(root, entries, depth + 1, suffix)
            return
        split(root, zero, depth + 1, suffix + "0")
        split(root, one, depth + 1, suffix + "1")

    for root, entries in roots.items():
        split(root, entries, 0, "")
    return assigned


def slice_sources(path: Path) -> list[str]:
    return [line.strip() for line in path.read_text().splitlines()
            if line.strip() and not line.lstrip().startswith("#")]


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: unity_groups.py <slice>", file=sys.stderr)
        return 2
    try:
        sources = slice_sources(Path(sys.argv[1]))
        groups = group_sources(sources)
    except (OSError, ValueError) as error:
        print(f"unity groups: {error}", file=sys.stderr)
        return 1
    for source in sources:
        print(f"{source}|{groups[source]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
