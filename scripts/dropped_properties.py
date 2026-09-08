import pathlib, re, sys, collections

# WHICH DOCUMENTED AL PROPERTIES THE GENERATOR NEVER READS, AND WHICH KIND THEY BELONG TO.
#
# THE PASS FOLDS CASE BEFORE IT BELIEVES "ABSENT". AL is case-insensitive and so is
# `Find(properties, name)`, so `Tooltip` and `ToolTip` are one property -- and a pass that compares
# them byte for byte reports 31 gaps that are not there. That is CLAUDE.md's own rule about a
# mechanical sweep, and it cost this one its first reading (board:0623).

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
DOCS = pathlib.Path.home() / "Git/dynamics365smb-devitpro-pb/dev-itpro/developer/properties"
AL = pathlib.Path.home() / "Git/BCApps/src"

# The kinds each property is documented to apply to, read from the page itself rather than listed.
def applies(page):
    text = page.read_text(errors="replace")
    block = re.search(r"## Applies to\n(.*?)(?:\n\[|\n## )", text, re.S)
    if not block:
        return set()
    return {line.strip("- \t").strip() for line in block.group(1).split("\n") if line.strip("- \t")}

def main():
    if not DOCS.is_dir():
        print("dropped: the platform documentation is not where apps.json says", file=sys.stderr)
        return 2
    documented = {}
    for page in DOCS.glob("devenv-*-property.md"):
        name = page.name[len("devenv-"):-len("-property.md")].replace("-", "")
        documented[name] = applies(page)
    if not documented:
        print("dropped: no property page read -- ABORT", file=sys.stderr)
        return 2

    read = set()
    for where in ("src/gen", "src/tc", "src/al"):
        for source in (ROOT / where).glob("*.cpp"):
            read |= {m.lower() for m in re.findall(r'"([A-Za-z][A-Za-z0-9]{2,})"', source.read_text())}
    if not read:
        print("dropped: the generator names no property -- ABORT", file=sys.stderr)
        return 2

    declared = collections.Counter()
    for source in AL.rglob("*.al"):
        for m in re.finditer(r"^\s+([A-Z][A-Za-z0-9]+)\s*=", source.read_text(errors="replace"), re.M):
            declared[m.group(1).lower()] += 1
    if not declared:
        print("dropped: no AL property declaration found -- ABORT", file=sys.stderr)
        return 2

    kinds = collections.Counter()
    rows = []
    for name, count in declared.most_common():
        if name not in documented or name in read:
            continue
        for kind in documented[name] or {"(no kind on the page)"}:
            kinds[kind] += 1
        rows.append((count, name, ", ".join(sorted(documented[name]))))

    print(f"dropped   {len(rows)} documented properties the generator never reads")
    print(f"          over {sum(n for n, _, _ in rows)} declarations in BCApps\n")
    for kind, n in kinds.most_common():
        print(f"  {n:4}  {kind}")
    print()
    for count, name, where in rows[:25]:
        print(f"  {count:6}  {name:28} {where}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
