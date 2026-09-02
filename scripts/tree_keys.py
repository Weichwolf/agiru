#!/usr/bin/env python3
"""One cache key per generated header: its own bytes and the bytes of everything it includes.

A HEADER'S RESULT DEPENDS ON ITS CLOSURE, NOT ON ITSELF. Keying a cache on the file alone would
reuse a verdict for a file that did not change while a header it includes did -- which is the
measurement lying in the one direction that is hardest to notice, because it lies in favour of the
last good answer.

The closure is read from the `#include "..."` lines, which is enough here and nowhere else: a
generated header includes by app-relative path or `agiru.h`, both resolvable without a compiler.
The door is not walked -- its print is passed in and covers all of it.
"""
import hashlib
import os
import sys


def resolve(target, roots):
    for root in roots:
        path = os.path.join(root, target)
        if os.path.isfile(path):
            return os.path.realpath(path)
    return None


def includes(path):
    named = []
    with open(path, "rb") as file:
        for raw in file:
            line = raw.strip()
            if not line.startswith(b"#include"):
                if line and not line.startswith((b"//", b"/*", b"*", b"#")):
                    break
                continue
            start = line.find(b'"')
            if start < 0:
                continue
            end = line.find(b'"', start + 1)
            if end > start:
                named.append(line[start + 1:end].decode())
    return named


def main():
    door, apps = sys.argv[1], sys.argv[2]
    roots = [os.path.join(apps, name) for name in sorted(os.listdir(apps))
             if os.path.isdir(os.path.join(apps, name))]

    own = {}
    reached = {}

    def digest(path):
        if path not in own:
            with open(path, "rb") as file:
                own[path] = hashlib.sha1(file.read()).hexdigest()
        return own[path]

    def closure(path, walking):
        if path in reached:
            return reached[path]
        # A CYCLE IS NOT AN ERROR HERE. Two headers that include each other have one closure
        # between them, and stopping at the second visit is what makes it finite.
        if path in walking:
            return ""
        walking.add(path)
        parts = [digest(path)]
        for name in includes(path):
            found = resolve(name, roots)
            if found is not None:
                parts.append(closure(found, walking))
        walking.discard(path)
        reached[path] = hashlib.sha1("".join(sorted(parts)).encode()).hexdigest()
        return reached[path]

    sys.setrecursionlimit(10000)
    for line in sys.stdin:
        path = line.strip()
        if path:
            print(f"{path}\t{door}-{closure(os.path.realpath(path), set())}")


if __name__ == "__main__":
    main()
