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


def implemented_here() -> set[str]:
    """What this tree implements, as collapse keys.

    Read from DOXYGEN'S XML rather than from a regular expression over the headers: doxygen has
    already parsed the door and knows what is a declaration and what is a word inside a comment.
    Only `include/` is in that XML, which is the point -- a method that exists in `src/` and
    not in the door cannot be reached by generated code and does not count as implemented."""
    names: set[str] = set()
    xml = ROOT / "build" / "doc" / "xml"
    if not xml.is_dir():
        return names
    for path in xml.glob("*.xml"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for match in re.finditer(
            r'<memberdef[^>]*kind="(function|variable)"[^>]*>.*?<name>([^<]+)</name>', text, re.S
        ):
            names.add(collapse(match.group(2)))
        for match in re.finditer(r"<compoundname>([^<]+)</compoundname>", text):
            names.add(collapse(match.group(1).rsplit("::", 1)[-1]))
    return names


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
    here = implemented_here()
    rows = []
    for name, entry in data["types"].items():
        methods = entry["methods"]
        mine = [m for m in methods if collapse(m) in here]
        rows.append((name, len(methods), len(entry["predecessor"]), len(mine)))

    rows.sort(key=lambda r: -r[2])
    total = sum(r[1] for r in rows)
    prior = sum(r[2] for r in rows)
    mine = sum(r[3] for r in rows)

    print(f"al surface  {total} documented methods over {len(rows)} types")
    print(f"            {prior} implemented by the predecessor, {mine} here")
    print()
    print(f"{'type':<22}{'doc':>6}{'openerp':>9}{'agiru':>7}")
    for name, count, prior_count, my_count in rows[:20]:
        if prior_count == 0:
            continue
        print(f"{name:<22}{count:>6}{prior_count:>9}{my_count:>7}")
    return mine


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
        here = implemented_here()
        print(sum(1 for e in data["types"].values() for m in e["methods"] if collapse(m) in here))
        return 0
    report(data)
    return 0


if __name__ == "__main__":
    sys.exit(main())
