Type:     task
Status:   open
Parent:   0026
Area:     al, gen
Source:   developer/devenv-xml-comments.md
Verdict:  fehlt
Class:    activation

# The AL source carries a million comment lines, and the generator drops them

AL has documentation comments -- `///` followed by XML, immediately before the type or member they
annotate -- with a documented tag set:

| top-level | formatting |
|---|---|
| `<summary>`, `<param name="">`, `<returns>`, `<example>`, `<remarks>` | `<paramref name="">`, `<para>`, `<b>`, `<i>`, `<c>`, `<code>`, `<list type="bullet\|number\|table">` |

**They are Doxygen's tags under different names**, and the correspondence is close enough to be
mechanical: `<summary>` is `\brief`, `<param name="x">` is `\param x`, `<returns>` is `\return`,
`<remarks>` is a trailing paragraph, `<code>` is a fenced block.

## The size of what is thrown away

Measured 2026-09-04 over `~/Git/BCApps/src`:

| | count |
|---|---:|
| `///` documentation lines | **542 639**, across 5 768 files |
| `<param` tags | **136 416** |
| `<summary>` tags | **111 885** |
| `<returns>` tags | 11 254 |
| plain `//` comment lines | **719 565** |

**1.26 million comment lines**, of which 542 639 are structured documentation on 111 885 documented
members.

## CLAUDE.md answers this and the answer is uncomfortable

> "**A GENERATED FILE CARRIES NO COMMENTS**, only a two-line provenance header naming the `.al` it came
> from."

So the plain `//` comments go -- 719 565 of them -- and that is right for the same reason `make`
deletes comments in `src/`: they are the same statement in two languages and they drift.

**But the SAME document argues the other way about the `///` half:**

> "**THE STRONGER REASON IS THE READER.** Nobody will write an agiru module by hand: it will be
> written by a model, and AL is in its training data while agiru never will be. So a reader who knows
> AL and has never seen agiru must open one file and know how to write the next."
>
> "`include/` is the public interface and every public name carries Doxygen."

**A generated header IS an interface a reader opens**, and 111 885 `<summary>` tags are exactly the
knowledge that reader lacks. The rule that keeps `src/` clean, applied to `apps/`, discards the one
kind of comment CLAUDE.md elsewhere requires.

**This item does not resolve that. It states it with the number and names what would settle it**,
because the rule is CLAUDE.md's and this board does not overturn CLAUDE.md by argument.

## The three positions, and what each costs

| | what happens to the 542 639 | cost |
|---|---|---|
| **A -- drop everything** | discarded with the `//` comments | the current rule, no work, and a generated header that says nothing about what a procedure is for |
| **B -- translate `///` to Doxygen** | `<summary>` becomes `\brief` and so on | a tag translator in the generator; the generated tree grows by roughly 540 000 lines; `make`'s comment stripper must not touch `apps/`, which it already does not |
| **C -- keep them as a side artefact** | emitted to a separate file per object, not into the header | the door's parse cost is untouched, and nothing reads them |

**The measurement that decides it is not taken here and is named**: what a `///` block costs in the
door's PARSE TIME. CLAUDE.md measures that cost for `<memory>` at 1.2 s of 3.4 s over 7 885
translation units, so a comment block per member is exactly the kind of thing that budget exists for.
**A comment is not a declaration and a preprocessor drops it early** -- so the cost is plausibly near
zero and plausibly not, and one `make tree` with and without settles it.

**Position B is the one this item recommends**, on the reader argument alone, CONDITIONAL on that
measurement. Position A is what happens if nobody chooses.

## One fact that constrains all three

> "If you have the `allowDownloadingSource` setting in `app.json` set to `false` and you then download
> an app package, the app package won't contain any XML comments."

So BC itself treats the comments as SOURCE rather than as metadata, and an app may ship without them.
**agiru reads `~/Git/BCApps` directly rather than an app package** (CLAUDE.md), so the comments are
always present in the input -- but the fact says what they are: part of the source text, not part of
the object's definition.

## The IST-state

- **`src/al/Lexer.cpp` handles `//` and `///` as comments** and the AST keeps none of them --
  `al::ProcedureDecl` and `al::FieldDecl` have no documentation member.
- **`test/strip-comments.py` runs over `src/` and not over `apps/`**, so nothing would delete a
  generated comment; the rule is enforced by the generator not writing one.
- **`make lint`'s doc baseline counts undocumented public names in `include/` only** -- `apps/` is out
  of `make lint` entirely, so a documented generated header would not be measured either way.

## The choice

**If B: one pass in the lexer that attaches a `///` block to the next declaration, and one translator
from the AL tag set to Doxygen.**

```cpp
struct DocComment { std::string brief, returns; std::vector<std::pair<std::string, std::string>> params; };
```

**Why attach in the LEXER and not the parser:** the block is defined positionally -- *"must
immediately precede a user-defined type that it annotates"* -- so the association is lexical and the
parser would have to reconstruct it from token positions.

**Why a translator and not a passthrough:** `<summary>` in a C++ header is not Doxygen, and a
generated file that carried raw XML would be documented for no tool. The tag sets correspond, so the
translation is a table of eleven entries.

**What does NOT survive**: `<example>` and `<code>` blocks contain AL, which in a C++ header would be
a code sample in the wrong language. They are dropped even under B, and that is a decision rather than
an omission.

## Ordering

**After the measurement**, which is one `make tree` pair and belongs to whoever next runs the tree.
Nothing else waits on this.

## Gate, and its negative control

Under position B:

1. an AL procedure with `<summary>` and two `<param>` tags emits a Doxygen block with `\brief` and two
   `\param`
2. a procedure with no `///` block emits no comment
3. an `<example>` block emits nothing
4. `make` still deletes every comment in `src/` and none in `apps/`

**The negative control is case 2.** Emit an empty `\brief` for an undocumented member -- which a
template-driven generator does by default -- and cases 1, 3 and 4 stay green while the tree grows a
useless line per member. With 111 885 documented members against a much larger number of members in
all, the difference is most of the output.

**Case 4 is the guard against the obvious accident**: pointing `test/strip-comments.py` at the tree
would delete what B just wrote, and it would look like B failing rather than like the stripper
succeeding.

## Class

`activation`, and unusually it activates nothing at run time -- no behaviour changes under any of the
three positions. **What changes is what a reader of `apps/` can see**, and CLAUDE.md says that reader
is a model with AL in its training data and no knowledge of agiru. That is the whole argument, and it
is why the item exists rather than being settled by the no-comments rule alone.
