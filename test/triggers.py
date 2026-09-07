"""The trigger census: which of the documented triggers the tree names at all.

`triggers-auto/` is one page per trigger per object kind, 151 of them. A trigger the runtime or
the generator never names is one nothing fires, declares or refuses -- the silent kind of hole.
The count of NAMED triggers is the baseline in `test/trigger-baseline`, and it may only rise.
"""
import pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = pathlib.Path.home() / "Git/dynamics365smb-devitpro-pb/dev-itpro/developer/triggers-auto"


def census():
    rows = []
    for md in sorted(DOCS.rglob("*.md")):
        m = re.match(r"devenv-(.+)-([a-z]+)-trigger\.md", md.name)
        if not m:
            continue
        head = re.search(r"^# (\w+)", md.read_text(errors="replace"), re.M)
        name = head.group(1) if head else m.group(1)
        hits = subprocess.run(["grep", "-rlw", name, "src/rt", "src/gen", "include"],
                              capture_output=True, text=True, cwd=ROOT).stdout.split()
        rows.append((m.group(2), name, len(hits) > 0))
    return rows


def main():
    rows = census()
    if not rows:
        print("trigger census: no pages found under", DOCS, "-- an empty census is an abort")
        return 2
    named = sum(1 for _, _, n in rows if n)
    baseline_path = ROOT / "test/trigger-baseline"
    baseline = int(baseline_path.read_text().split()[0]) if baseline_path.exists() else 0
    kinds = {}
    for kind, name, n in rows:
        kinds.setdefault(kind, []).append((name, n))
    if "--table" in sys.argv:
        for kind in sorted(kinds, key=lambda k: -len(kinds[k])):
            absent = [x for x, n in kinds[kind] if not n]
            print(f"{kind:28} {len(kinds[kind]) - len(absent):2} named / {len(absent):2} absent"
                  + ("   " + ", ".join(absent) if absent else ""))
    print(f"triggers named {named} of {len(rows)} (baseline {baseline})")
    if named < baseline:
        print("the trigger baseline may only rise")
        return 1
    if named > baseline and "--record" in sys.argv:
        baseline_path.write_text(f"{named} of {len(rows)}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
