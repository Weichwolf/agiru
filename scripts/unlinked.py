#!/usr/bin/env python3
"""Emit a definition for every AL procedure the slice calls and nothing defines.

WHY THIS EXISTS. `test/slice` is the part of the generated tree that compiles, and it is not the
whole tree: a body in it calls procedures whose own source does not compile yet. Those calls leave
UNDEFINED FUNCTION symbols, which the dynamic loader binds LAZILY -- they cost nothing until a test
walks into one, and then the loader writes `symbol lookup error` to stderr and calls `_exit`.

THAT IS NOT A FAILURE THIS RUNTIME CAN COUNT. It is not a C++ exception, nothing catches it, and
`agiru run-tests` ends at the FIRST test that reaches one -- so the milestone would be gated behind
linking the whole tree rather than behind the tests (board:0612).

So this writes a definition for each of them that RAISES, naming the procedure it stands for. The
test that reached it fails the way every other refusal in this tree fails, and the next test runs.

THE TRICK THAT MAKES IT MECHANICAL: a body that never returns needs no signature. Each stub is
declared `void f()` with an `asm` label carrying the MANGLED name, so the linker sees the symbol the
caller wants; the body throws, so no return value is ever produced and no calling convention has to
be reproduced. The `asm` label is a GCC and Clang extension and it is architecture-independent,
which hand-written assembly would not be.

    python3 scripts/unlinked.py <out.cpp> <library.so> [<library-that-defines.so> ...]
"""

import pathlib
import subprocess
import sys


def fail(message):
    print(f"unlinked: {message}", file=sys.stderr)
    raise SystemExit(1)


def symbols(path, which):
    done = subprocess.run(["nm", "--dynamic", which, "--format=posix", str(path)],
                          capture_output=True, check=False)
    if done.returncode != 0:
        fail(f"nm refused {path}: {done.stderr.decode('utf-8', 'replace').strip()}")
    found = set()
    for line in done.stdout.decode("utf-8", "replace").splitlines():
        parts = line.split()
        if len(parts) >= 2:
            found.add(parts[0])
    return found


def demangled(names):
    if not names:
        return {}
    done = subprocess.run(["c++filt"], input="\n".join(names), capture_output=True, text=True,
                          check=False)
    if done.returncode != 0:
        fail("c++filt refused the symbol list")
    return dict(zip(names, done.stdout.splitlines()))


def main():
    if len(sys.argv) < 3:
        fail("usage: unlinked.py <out.cpp> <library.so> [<defining.so> ...]")
    out = pathlib.Path(sys.argv[1])
    slice_so = pathlib.Path(sys.argv[2])
    if not slice_so.is_file():
        fail(f"{slice_so} does not exist")

    wanted = {name for name in symbols(slice_so, "--undefined-only") if "5agiru" in name}
    for other in sys.argv[3:]:
        if pathlib.Path(other).is_file():
            wanted -= symbols(pathlib.Path(other), "--defined-only")

    names = sorted(wanted)
    readable = demangled(names)
    # A DATA SYMBOL CANNOT BE STOOD IN FOR. A table's or a page's definition -- `kSalesHeaderTable`,
    # `kCurrenciesPage` -- is read as an OBJECT, and a function body under that name is read as
    # the object's bytes: `Get` walked the primary key of a function and 32 codeunits died of
    # SIGSEGV on 2026-09-09 (board:0634). The definition has its own unit, `<Object>.def.cpp`,
    # which compiles whenever the header does, so an undefined one is a slice that lacks the unit.
    data = [name for name in names if "(" not in readable.get(name, name)]
    if data:
        for name in data:
            print(f"unlinked: {readable.get(name, name)} is DATA and the slice does not define it",
                  file=sys.stderr)
        fail(f"{len(data)} data symbol(s) undefined -- add their .def.cpp units to test/slice")
    text = ["// Generated from the slice's undefined symbols. Do not edit.",
            "// One definition per AL procedure the slice calls and no linked source defines.",
            "",
            '#include "runtime/Error.h"',
            "",
            "#include <string>",
            "",
            "namespace {",
            "",
            "[[noreturn]] void Unlinked(const char *procedure) {",
            '  throw ::agiru::Error(std::string("the AL procedure ") + procedure +',
            '                       " is declared and its source is not in the slice '
            '(board:0612)");',
            "}",
            "",
            "}",
            ""]
    for at, name in enumerate(names):
        spelled = readable.get(name, name).replace("\\", "\\\\").replace('"', '\\"')
        text.append(f'extern "C" void agiru_unlinked_{at}() asm("{name}");')
        text.append(f'extern "C" void agiru_unlinked_{at}() {{ Unlinked("{spelled}"); }}')
    text.append("")
    body = "\n".join(text)
    if not out.exists() or out.read_text() != body:
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(body)
    print(f"unlinked: {len(names)} AL procedure(s) stand in for a source the slice does not carry")


if __name__ == "__main__":
    main()
