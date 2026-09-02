import sys, pathlib, re
sys.path.insert(0, "/tmp/claude-1000/-home-cosmo-Git-agiru/94c34270-14d1-470d-916f-8c83426f799c/scratchpad")
import door, free

ROOT = pathlib.Path("/home/cosmo/Git/agiru")
door.GENERATED = set()
existing = {}
for d, pre in (("include/type", "type"), ("include/runtime", "runtime"),
               ("include/platform", "platform"), ("include/meta", "meta")):
    for f in (ROOT / d).glob("*.h"):
        existing[f.stem] = f"{pre}/{f.name}"
existing.update({"InStream": "type/Stream.h", "OutStream": "type/Stream.h",
                 "FieldRef": "runtime/RecordRef.h", "RecordRef": "runtime/RecordRef.h"})
# A BARE OBJECT TYPE IS NOT A TYPE HERE. `Codeunit.Run(Codeunit, Record)` names an object by number
# and hands it a record; both are values whose type the CALLER decides, which is what Variant is.
for bare in ("Codeunit", "Record", "Page", "Report", "Query", "XmlPort", "Table", "TestPage",
             "TestRequestPage", "Enum", "Option", "DotNet", "Interface"):
    existing.pop(bare, None)
door.EXISTING = existing
known = set(door.canonical_names().values())

# A FREE FUNCTION MAY CARRY A DOOR TYPE'S NAME, and in C++ the function then hides the type for
# everything after it. Two do, and each has its own answer rather than one rule:
#
#   `Error(Text)` is AL's way of RAISING, and `agiru::Error` is the exception it raises. AL has no
#   `Error` type for a variable to be, so nothing is lost: the generator translates the AL statement
#   into `throw Error(...)`, which is the constructor, and no free function is needed at all.
#
#   `Time()` reads the session clock and `agiru::Time` is the AL type a variable is declared as. The
#   TYPE wins, because every generated table with a Time field names it and no UT test calls the
#   function (board:0040 counts 0 of 2 392).
COLLIDES = {"Error", "Time"}

# AND SOME ARE ALREADY BUILT, with behaviour rather than a refusal. Generating them again would be a
# duplicate symbol -- which the linker said, and which is the right answer: what exists wins.
# WHAT THE DOOR ALREADY DECLARES IS FOUND RATHER THAN LISTED. Three were listed by hand and a
# fourth got through: `CurrentDateTime` is defined in `agiru_net` and the refusing copy in
# `agiru_rt` -- two shared libraries, so the linker said nothing and the refusal won at run time.
# Three gate cases went red with "not implemented yet" for a function that was implemented.
BUILT = set()
for header in sorted((ROOT / "include").rglob("*.h")):
    if header.name == "Builtins.h":
        continue
    for m in re.finditer(r"^(?:\[\[nodiscard\]\]\s*)?[A-Za-z_][\w:<>,& ]*\s+(\w+)\(",
                         header.read_text(errors="replace"), re.M):
        BUILT.add(m.group(1))

alldecls, allbodies, used = [], [], set()
for owner in ("system", "text", "database", "session", "dialog", "file", "secrettext"):
    ms = [m for m in free.free(owner) if m["static"]]
    ms = [m for m in ms if m["method"] not in COLLIDES and m["method"] not in BUILT]
    decls, bodies = free.emit(owner, ms, known)
    for m in ms:
        for _, t, _ in m["params"]:
            used.add(t)
        used.add(m["ret"])
    alldecls.append(f"// ---- {owner} ----")
    alldecls += decls
    allbodies += bodies

needed = {x for t in used for x in re.findall(r"[A-Za-z][A-Za-z0-9]*", door.cpp_type(t))}
needed |= {"Variant", "Integer", "Boolean", "Date", "Time", "DateTime", "Duration", "Decimal",
           "BigInteger", "Guid", "Byte", "Char", "DateFormula", "RecordId"}
inc = "".join(f'#include "{existing[t]}"\n' for t in sorted(needed) if t in existing)

head = '''#pragma once

#include "runtime/Error.h"
''' + inc + '''
#include <string>
#include <string_view>

/// \\file
/// \\brief The AL functions a body calls with NO RECEIVER.
///
/// The platform documents them under a type -- Text.StrSubstNo, System.WorkDate, Database.CalcDate
/// -- and every one carries the same note: "This method can be invoked without specifying the data
/// type name." That note is what selects them, and it is why they are here rather than on the
/// types: a generated body writes the name with nothing in front of it, so ordinary lookup in
/// `agiru` has to find it.
///
/// \\warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature is the one
///          methods-auto states, so a call site compiles and is CHECKED; the body refuses by name
///          rather than returning a plausible wrong answer (board:0035). What the UT milestone
///          leans on hardest is measured: StrSubstNo in 187 of its 2 392 test methods, WorkDate in
///          151, Format in 113 (board:0040).

namespace agiru {

'''
(ROOT / "include/Builtins.h").write_text(head + "\n\n".join(alldecls) + "\n\n} // namespace agiru\n")

# THE SOURCE INCLUDES WHAT ITS BODIES NAME, which is only what the signatures spell -- the header
# is what needs the whole set.
bodytext = "\n".join(allbodies)
inbody = {t for t in needed if t in existing and re.search(r"\b" + t + r"\b", bodytext)}
# The same suppression the header carries, for the same reason: these are AL's parameter orders.
guard = ("// NOLINTBEGIN(bugprone-easily-swappable-parameters,"
         "performance-unnecessary-value-param)")
src = ['#include "Builtins.h"', "", '#include "runtime/Error.h"'] + \
      [f'#include "{existing[t]}"' for t in sorted(inbody)] + \
      ["", "#include <string>", "#include <string_view>", "", "namespace agiru {", "",
       "namespace {", "", "[[noreturn]] void RefuseDoor(std::string_view what) {",
       '  throw Error(std::string(what) + " is declared and not implemented yet (board:0035)");',
       "}", "", "} // namespace", "", guard, ""] + allbodies + \
      ["", guard.replace("NOLINTBEGIN", "NOLINTEND"), "", "} // namespace agiru", ""]
(ROOT / "src/rt/Builtins.cpp").write_text("\n".join(src))
print(len(allbodies), "free functions")
