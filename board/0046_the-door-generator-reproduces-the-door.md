# 0046 -- `scripts/gen_builtins.py` reproduces `include/Builtins.h`

The door's generator does not produce the door that is committed, and three states of it exist.

| what | free functions |
|---|---|
| `include/Builtins.h` at HEAD, committed as "all 154 of them" (d854066) | 154 |
| `scripts/door.py` + `scripts/free.py` as committed | 151 |
| the copies the last run actually imported, from a session's temp directory | 149 |

**The reference.** None. This is not an AL semantic -- it is a generated artefact whose generator
was never committed in the state that generated it, which no reference can settle.

**What the AL source does.** Nothing; the input is
`methods-auto/` and the free-function pages, and both are unchanged.

**What the predecessor made of it.** `~/Git/openerp/` generates no door at all -- Python needed no
declaration -- so it has nothing to say here.

**What is wrong.** `gen_builtins.py` carried
`sys.path.insert(0, "/tmp/claude-1000/.../scratchpad")` and `ROOT = pathlib.Path("/home/cosmo/...")`,
so it imported its two modules from a temporary directory and ran on exactly one machine. Those
copies had moved on from the committed ones -- optional-parameter positions in `door.py`, an
overload-ambiguity rule in `free.py` -- and they name a `RefuseDoor` that no header declares. So
the committed scripts are behind the header, the temporary ones are ahead of it, and neither
builds it.

Both absolute paths are gone (this round); the scripts now find their own root. What remains is
that running them still does not give the file beside them.

**The choice.** The HEADER is the artefact of record and stays as it is -- it compiles, and the
gate is green over it. The generator is brought up to it, in this order, because the reverse
overwrites a working door with a worse one:

1. Take the two module fixes the temporary copies carry, with a case for each: a call that leaves
   out an optional tail (`PadStr(String, Length)`), and a six-argument `GetUrl` that must not match
   two overloads.
2. Add whatever `RefuseDoor` was going to be, or drop it -- a name emitted into the door and
   declared nowhere is the loud failure the tree asks for, written half way.
3. Then `python3 scripts/gen_builtins.py` leaves `include/Builtins.h` byte-identical, and a gate
   asserts that, so this cannot come back.

**What a second attempt established** (this round). The regeneration does not merely produce
FEWER functions -- it loses named ones. `System.Format(Any, Integer, Integer)` is documented at
`methods-auto/system/system-format-joker-integer-integer-method.md`, stands in the committed header
as `std::string Format(const Variant &, Integer = {}, Integer = {})`, and is ABSENT from what either
script state emits. `Assert.cpp` calls `Format(Left, 0, 2)` and is in the slice, so the loss is
immediate and visible.

Step 2 is done: `RefuseDoor` is declared in the header rather than hidden in an anonymous namespace
in the source. It has to be, because a `var Any` parameter becomes a TEMPLATE -- `Clear(var Any)`
takes a record, a text, a list, anything at all by reference -- and a template's body must be
visible where it is instantiated. That was the error the first attempt died on.

What remains is step 1 and step 3: find why `Format` is dropped, then assert byte-identity in a
gate. **Until then the header is not regenerated**, and the missing optional tails stay missing --
`Message(String)` alone is one error in `Currency`, whose table number is 4, which is what the two
failing tests in `Library - Utility UT` ask for.

**Why it matters beyond the file.** The three counts differ because the two later states DROP
functions the first one had -- a shorter overload rule removing what a call site needs shows up as
`no matching function for call to 'Format'`, which is 20 errors in `Assert.cpp` alone and reads as
a defect in the generated tree.
