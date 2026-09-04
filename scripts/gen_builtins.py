import sys, pathlib, re

# THE SCRIPT FINDS ITS OWN NEIGHBOURS AND ITS OWN ROOT. Both were absolute paths -- one into a
# session's temporary directory, one into a checkout by name -- so this ran on exactly one machine
# and nowhere else, which is the same defect as a hardcoded connection string.
HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import door, free

ROOT = HERE.parent


def settle(path, text):
    """Write only when the bytes differ.

    An unconditional write changes the mtime, and clang's precompiled header compares MTIME rather
    than content: a rewrite that changes nothing still kills the PCH for the whole door and every
    ccache entry behind it, and a `make tree` running beside it loses its entire census to
    "file has been modified since the precompiled header was built".
    """
    if path.exists() and path.read_text() == text:
        return
    path.write_text(text)

door.GENERATED = set()
existing = {}
for d, pre in (("include/type", "type"), ("include/runtime", "runtime"),
               ("include/platform", "platform"), ("include/meta", "meta")):
    for f in (ROOT / d).glob("*.h"):
        # THE FIRST DIRECTORY WINS, and `type/` is first on purpose: CLAUDE.md puts one door header
        # per AL TYPE there, so `Integer` is `type/Integer.h`. Letting a later directory overwrite
        # gave `platform/Integer.h`, which declares something else of that name.
        existing.setdefault(f.stem, f"{pre}/{f.name}")
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
# A DUPLICATE SYMBOL NEEDS THE NAME AND THE SIGNATURE, so the key is the name and the ARITY.
# `runtime/Record.h` declares `Format(const T &)` -- one parameter, a template -- and by name alone
# that removed `System.Format(Any, Integer, Integer)` from the door, which `Assert.cpp` calls as
# `Format(Left, 0, 2)` and which is in the slice. Two functions of one name and different arities
# are two functions, which is what AL says too.
def arity(text, at):
    depth, count, seen = 0, 0, False
    for c in text[at:]:
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return count + (1 if seen else 0)
        elif c == "," and depth == 1:
            count += 1
        elif depth == 1 and not c.isspace():
            seen = True
    return count


# AND A TEMPLATE IS NOT A FUNCTION OF THAT ARITY FOR EVERY TYPE. `runtime/Record.h` declares
# `Format(const T &)` under a constraint that excludes everything a Variant can hold, so it does not
# cover `Format(9)` -- and suppressing the door's defaults because "a one-argument Format exists"
# took `Format(x)` away from every caller. CONCRETE holds only what is not a template.
BUILT = set()
CONCRETE = set()
for header in sorted((ROOT / "include").rglob("*.h")):
    if header.name == "Builtins.h":
        continue
    text = header.read_text(errors="replace")
    # A RETURN TYPE MAY START WITH `::`. `::agiru::Integer StrPos(...)` is how the door spells a
    # qualified one, and a pattern anchored on a letter skipped every such declaration -- so the
    # generator emitted its own `StrPos` beside the written one and the linker found both.
    for m in re.finditer(r"^(?:\[\[nodiscard\]\]\s*)?(?:::)?[A-Za-z_][\w:<>,& ]*\s+(\w+)\(",
                         text, re.M):
        found = (m.group(1), arity(text, m.end() - 1))
        BUILT.add(found)
        # THE MARKER SITS ON THE SAME LINE OR TWO ABOVE. `template <typename E> std::string
        # Format(const Option<E> &)` carries it inline; `template <typename T>\n requires(...)\n
        # std::string Format(const T &)` carries it two lines up. Reading only the lines before
        # missed the first and put a template in CONCRETE.
        above = text.rfind("\n", 0, text.rfind("\n", 0, max(0, m.start() - 1)) + 1)
        if "template" not in text[max(0, above):m.end()]:
            CONCRETE.add(found)

# WHAT IS WRITTEN RATHER THAN GENERATED LIVES IN `BuiltinsWritten.h`, and the `BUILT` scrape above
# excludes it by construction -- it reads every door header but this script's own output. Twelve
# functions were written INTO the generated pair and re-running this turned each back into a
# refusal, silently; moving them out is the whole fix, and it needs no rule of its own.
alldecls, allbodies, used = [], [], set()
for owner in ("system", "text", "database", "session", "dialog", "file", "secrettext"):
    ms = [m for m in free.free(owner) if m["static"]]
    ms = [m for m in ms
          if m["method"] not in COLLIDES and (m["method"], len(m["params"])) not in BUILT]
    decls, bodies = free.emit(owner, ms, known)
    for m in ms:
        for _, t, _ in m["params"]:
            used.add(t)
        used.add(m["ret"])
    # NO SECTION MARKER. `make` strips every `//` from `include/` and `src/`, so a marker written
    # here is deleted on the next build and written again on the one after -- which makes the
    # generator look as though it does not reproduce its own output.
    alldecls += decls
    allbodies += bodies

needed = {x for t in used for x in re.findall(r"[A-Za-z][A-Za-z0-9]*", door.cpp_type(t))}
needed |= {"Variant", "Integer", "Boolean", "Date", "Time", "DateTime", "Duration", "Decimal",
           "BigInteger", "Guid", "Byte", "Char", "DateFormula", "RecordId"}
inc = "".join(f'#include "{existing[t]}"\n' for t in sorted(needed) if t in existing)

head = '''#pragma once

#include "BuiltinsWritten.h"
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
# NO CLOSING COMMENT. `test/strip-comments.py` deletes every `//` in the door and only Doxygen
# survives there, so `} // namespace agiru` is written by this script and removed by the next
# `make` -- forever. The door's precompiled header and every ccache entry behind it fall over on
# each of those rewrites, and a `make tree` running beside one loses its whole census.
# `RefuseDoor` IS DECLARED IN THE HEADER, because a body can land there. An AL `var Any` parameter
# becomes a TEMPLATE -- `Clear(var Any)` takes a record, a text, a list, anything at all by
# reference -- and a template's body has to be visible where it is instantiated, so the refusal has
# to be reachable from the header rather than hidden in an anonymous namespace in the source.
refusal = ("[[noreturn]] void RefuseDoor(std::string_view what);\n\n")
settle(ROOT / "include/Builtins.h", head + refusal + "\n\n".join(alldecls) + "\n}\n")

# THE SOURCE INCLUDES WHAT ITS BODIES NAME, which is only what the signatures spell -- the header
# is what needs the whole set.
bodytext = "\n".join(allbodies)
# WHAT THE BODIES NAME, AND NOT WHAT THE SIGNATURES DECLARE. The two differ: a refusal names its
# parameter types and nothing else, so restricting the search to `needed` left `Date` -- which a
# body mentions and no signature here does -- without a header, and kept `Integer.h` after the
# function that used it moved to `BuiltinsWritten.cpp`.
# AND NOT WHAT A STRING LITERAL NAMES. Every refusal carries its AL signature as text --
# `RefuseDoor("Session.CurrentClientType()")` -- so searching the raw source found `Session` and
# `Any` and asked for headers no code uses.
code = re.sub(r'"(?:[^"\\]|\\.)*"', '""', bodytext)
inbody = {t for t in existing if re.search(r"\b" + t + r"\b", code)}
# The same suppression the header carries, for the same reason: these are AL's parameter orders.
guard = ("// NOLINTBEGIN(bugprone-easily-swappable-parameters,"
         "performance-unnecessary-value-param)")
src = ['#include "Builtins.h"', "", '#include "runtime/Error.h"'] + \
      [f'#include "{existing[t]}"' for t in sorted(inbody)] + \
      ["", "#include <string>", "#include <string_view>", "", "namespace agiru {", "",
       "[[noreturn]] void RefuseDoor(std::string_view what) {",
       '  throw Error(std::string(what) + " is declared and not implemented yet (board:0035)");',
       "}", "", guard, ""] + allbodies + \
      ["", guard.replace("NOLINTBEGIN", "NOLINTEND"), "", "}", ""]
settle(ROOT / "src/rt/Builtins.cpp", "\n".join(src))
print(len(allbodies), "free functions")
