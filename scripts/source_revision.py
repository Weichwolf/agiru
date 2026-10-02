#!/usr/bin/env python3
"""Source revisions belong to the requested tree, never an unrelated ancestor checkout."""
import os
from pathlib import Path
import subprocess


def git_revision(path):
    result = subprocess.run(['git', '-C', str(path), 'rev-parse', 'HEAD'],
                            capture_output=True, text=True, check=False)
    return result.stdout.strip() if result.returncode == 0 else None


def bc_revision(path, owner):
    frozen = os.environ.get('AGIRU_BC_REVISION')
    if frozen:
        return frozen
    if Path(path).resolve().is_relative_to(owner):
        return None
    tracked = subprocess.run(['git', '-C', str(path), 'ls-files', '-z', '--', '.'],
                             capture_output=True, check=False)
    if tracked.returncode != 0 or not any(
            name.lower().endswith(b'.al') for name in tracked.stdout.split(b'\0')):
        return None
    return git_revision(path)
