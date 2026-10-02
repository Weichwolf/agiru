#!/usr/bin/env python3
"""Format changed handwritten C++ without touching unchanged file timestamps."""
import os
from pathlib import Path
import shutil
import subprocess

root = Path(__file__).resolve().parents[1]
formatter = shutil.which('clang-format-19') or shutil.which('clang-format')
if not formatter:
    raise SystemExit('format: clang-format is required')
changed = set()
for command in (
    ['git', 'diff', '--name-only', '-z', 'HEAD'],
    ['git', 'ls-files', '--others', '--exclude-standard', '-z'],
):
    for name in subprocess.check_output(command, cwd=root).split(b'\0'):
        if name:
            path = root / os.fsdecode(name)
            if path.is_file() and path.suffix in ('.h', '.cpp') and not path.is_relative_to(root / 'apps'):
                changed.add(path)
for path in sorted(changed):
    formatted = subprocess.check_output([formatter, str(path)], cwd=root)
    if formatted != path.read_bytes():
        path.write_bytes(formatted)
