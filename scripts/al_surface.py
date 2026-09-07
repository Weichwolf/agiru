"""Build the inventory of what the runtime must implement, and measure how much of it exists.

THE DOCUMENTATION IS THE SPECIFICATION and it is complete: `methods-auto/<type>/` carries every
method of every AL type, one file per SIGNATURE, across 135 types. That is the denominator -- what a
faithful runtime owes, whether or not any test asks for it.

THE PREDECESSOR IS THE PRIORITY. openerp is 97 % green on the UT subset, so what it implements is
what a runtime actually needs to get there, and what it does not is what can wait. That is the
ordering, not the target.

    python3 scripts/al_surface.py --write     # refresh doc/al-surface.json
    python3 scripts/al_surface.py             # report coverage against it

The report is three numbers per type: documented, implemented by the predecessor, implemented here.
`make lint` holds the last one to a baseline that may only rise.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DOCS = pathlib.Path.home() / "Git/dynamics365smb-devitpro-pb/dev-itpro/developer/methods-auto"
PREDECESSOR = pathlib.Path.home() / "Git/openerp/openerp/runtime"
SURFACE = ROOT / "doc" / "al-surface.json"


def collapse(name: str) -> str:
    """The comparison key. AL is case-insensitive and its names cross three spellings here --
    `StrSubstNo` in the documentation, `_al_strsubstno` in the predecessor, `StrSubstNo` here --
    so everything is compared with the separators and the case removed. That is the same
    collapse-match the field and enum resolvers use."""
    return re.sub(r"[^a-z0-9]", "", name.lower())


def documented() -> dict[str, list[str]]:
    """Every method of every AL type, from the documentation's own file names.

    A file is `<type>-<method>[-<argtypes>]-method.md`, one per SIGNATURE, so the overloads of a
    method collapse to one entry: a runtime owes the method, and the argument list is a detail of
    how it owes it."""
    surface: dict[str, set[str]] = {}
    for type_dir in sorted(DOCS.iterdir()):
        if not type_dir.is_dir():
            continue
        names: set[str] = set()
        for path in type_dir.glob(f"{type_dir.name}-*-method.md"):
            stem = path.stem[len(type_dir.name) + 1 : -len("-method")]
            # The argument types follow the method name, separated by a hyphen. A trailing empty
            # segment marks the no-argument overload (`record-insert--method.md`).
            method = stem.split("-")[0]
            if method:
                names.add(method)
        if names:
            surface[type_dir.name] = sorted(names)
    return surface


def predecessor_names() -> set[str]:
    """What openerp implements, as collapse keys.

    Three shapes carry AL methods there: `_al_<name>` free functions (the global builtins), methods
    on the runtime base classes (`Record`, `Page`, `Report`, ...), and the rebuilt `_DotNet*`
    classes. All three are read; nothing is inferred from a name that is not defined."""
    names: set[str] = set()
    if not PREDECESSOR.is_dir():
        return names
    for path in PREDECESSOR.rglob("*.py"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for match in re.finditer(r"^\s*def\s+(_al_)?([a-z_][a-z0-9_]*)\s*\(", text, re.M):
            name = match.group(2)
            if name.startswith("__"):
                continue
            names.add(collapse(name.rstrip("_")))
    return names


def doxygen_xml() -> pathlib.Path:
    """The door, as doxygen parsed it -- regenerated when it is missing or older than the door.

    IT REGENERATES RATHER THAN RETURNING NOTHING. This directory is not build output anybody makes
    on the way here: `make` does not run doxygen, so a fresh checkout or a `make clean` leaves it
    absent, and a counter that reads an absent directory reports 0 of 1 253 implemented and calls
    it a measurement. That is CLAUDE.md's blind gate, and it was live: `test/surface-baseline`
    says 1173 and this script answered 0 (measured 2026-09-07).
    """
    xml = ROOT / "build" / "doc" / "xml"
    stamp = xml / "index.xml"
    newest = max((path.stat().st_mtime for path in (ROOT / "include").rglob("*.h")), default=0.0)
    if not stamp.is_file() or stamp.stat().st_mtime < newest:
        done = subprocess.run(["doxygen", "doc/Doxyfile"], cwd=ROOT, capture_output=True)
        if done.returncode != 0:
            print("al surface: doxygen refused -- " + done.stderr.decode("utf-8", "replace")[-400:],
                  file=sys.stderr)
            raise SystemExit(1)
    if not stamp.is_file():
        print(f"al surface: {stamp} is still missing after running doxygen. ABORT, not a zero.",
              file=sys.stderr)
        raise SystemExit(1)
    return xml


# THE DOOR SPELLS AN AL TYPE THE WAY AL SPELLS IT, and these five are where it cannot.
# `Record` is `Table<Derived>` because AL's `Record` IS the base every table extends, and a CRTP
# base cannot be called `Record` while the concrete tables are the records. `TestPart` is a
# `TestPage` in a subpage's place and AL documents the two apart. `RequestPage` is the report's
# own page. Each is a deviation this file NAMES rather than hides, because the alternative is a
# type whose whole method list silently counts as missing (CLAUDE.md: a mechanical pass cannot
# tell a naming defect from a gap).
DOOR_NAME = {"record": "table", "testpart": "testpage", "requestpage": "page"}

# THE ONE METHOD THE DOOR SPELLS APART FROM AL, and it is named here as well as in
# `src/gen/Door.cpp` because a measure that does not know about the deviation counts it as a gap
# for ever. `Time` is an AL data type AND an AL builtin, and C++ holds one name once per namespace.
DOOR_METHOD = {("system", "time"): "currenttime"}


def door_surface() -> tuple[dict[str, set[str]], set[str]]:
    """What the door declares, per CLASS and as free functions.

    A GLOBAL NAME SET SATURATES AND STOPS MEASURING. Compared against one bag of every name in
    `include/`, the documented surface reads 1 253 of 1 253 -- `Insert` counts for `XmlElement`
    because `Record` has one. Per type it reads what is actually missing. The free functions are
    kept beside it because AL documents a STATIC method on the type (`Text.CopyStr`,
    `Session.CurrentClientType`) and the door writes exactly those as free functions.
    """
    xml = doxygen_xml()
    members: dict[str, set[str]] = {}
    bases: dict[str, set[str]] = {}
    free: set[str] = set()
    for path in xml.glob("*.xml"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for compound in re.finditer(
            r'<compounddef[^>]*kind="(class|struct|namespace|file)".*?</compounddef>', text, re.S
        ):
            block, kind = compound.group(0), compound.group(1)
            named = re.search(r"<compoundname>([^<]+)</compoundname>", block)
            if named is None:
                continue
            short = collapse(named.group(1).split("&lt;")[0].rsplit("::", 1)[-1])
            for base in re.finditer(r"<basecompoundref[^>]*>([^<]+)</basecompoundref>", block):
                spelled = collapse(base.group(1).split("&lt;")[0].rsplit("::", 1)[-1])
                bases.setdefault(short, set()).add(spelled)
            for member in re.finditer(
                r'<memberdef[^>]*kind="(function|variable)"[^>]*>.*?<name>([^<]+)</name>',
                block,
                re.S,
            ):
                if kind in ("class", "struct"):
                    members.setdefault(short, set()).add(collapse(member.group(2)))
                else:
                    free.add(collapse(member.group(2)))
    if not members or not free:
        print("al surface: the door parsed to nothing at all. ABORT, not a zero.", file=sys.stderr)
        raise SystemExit(1)

    def inherited(name: str, seen: set[str] | None = None) -> set[str]:
        seen = seen or set()
        if name in seen:
            return set()
        seen.add(name)
        reachable = set(members.get(name, ()))
        for base in bases.get(name, ()):
            reachable |= inherited(base, seen)
        return reachable

    return {name: inherited(name) for name in members}, free


def missing_per_type(data: dict) -> list[tuple[str, int, list[str]]]:
    """Per AL type: how many methods the documentation names, and which the door does not have."""
    classes, free = door_surface()
    rows = []
    for name, entry in data["types"].items():
        key = DOOR_NAME.get(collapse(name), collapse(name))
        reachable = classes.get(key, set()) | free
        missing = [
            m
            for m in entry["methods"]
            if collapse(m) not in reachable
            and DOOR_METHOD.get((collapse(name), collapse(m)), "") not in reachable
        ]
        rows.append((name, len(entry["methods"]), missing))
    rows.sort(key=lambda row: -len(row[2]))
    return rows


def build() -> dict:
    surface = documented()
    predecessor = predecessor_names()
    return {
        "_doc": "Every method of every AL type, from methods-auto/ -- the documentation is the "
        "specification and it is complete. `predecessor` marks the ones openerp implements, which "
        "is the ORDER to do them in rather than the target: it is 97 % green on the UT subset, so "
        "what it needed is what gets a runtime there.",
        "types": {
            name: {
                "methods": methods,
                "predecessor": sorted(m for m in methods if collapse(m) in predecessor),
            }
            for name, methods in surface.items()
        },
    }


def report(data: dict) -> int:
    rows = missing_per_type(data)
    total = sum(count for _, count, _ in rows)
    gaps = sum(len(missing) for _, _, missing in rows)
    print(f"al surface  {total} documented methods over {len(rows)} types")
    print(f"            {total - gaps} reachable from the door, {gaps} missing")
    print()
    print(f"{'type':<22}{'doc':>6}{'missing':>9}  the missing ones")
    for name, count, missing in rows:
        if missing:
            print(f"{name:<22}{count:>6}{len(missing):>9}  {', '.join(missing)}")
    return total - gaps


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="refresh doc/al-surface.json")
    parser.add_argument("--count", action="store_true", help="print only the implemented count")
    arguments = parser.parse_args()

    if arguments.write:
        if not DOCS.is_dir():
            print(f"al surface: {DOCS} is missing", file=sys.stderr)
            return 1
        SURFACE.parent.mkdir(parents=True, exist_ok=True)
        SURFACE.write_text(json.dumps(build(), indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(f"al surface: wrote {SURFACE.relative_to(ROOT)}")
        return 0

    if not SURFACE.is_file():
        print("al surface: doc/al-surface.json is missing -- run with --write", file=sys.stderr)
        return 1
    data = json.loads(SURFACE.read_text(encoding="utf-8"))
    if arguments.count:
        rows = missing_per_type(data)
        print(sum(count - len(missing) for _, count, missing in rows))
        return 0
    report(data)
    return 0


if __name__ == "__main__":
    sys.exit(main())
