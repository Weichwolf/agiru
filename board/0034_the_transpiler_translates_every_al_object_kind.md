Type: root
State: open
Area: al, gen

# The transpiler translates every AL object kind

The UT suite is the PROOF, not the target. The target is a complete Business Central in C++, and
that makes "what the transpiler must be able to do" a question with a measurable answer rather than
a horizon. `~/Git/openerp/scripts/transpiler` is the floor: 18 894 lines over 46 modules that
reached 97.0 % of the UT subset, with a generator per object kind.

Measured over BCApps and over both trees on 2026-09-02:

| AL object | files in BCApps | agiru | openerp |
|---|---|---|---|
| Codeunit | 14 417 | yes | yes |
| **Page** | **6 967** | no | yes (`page_gen`, 1 553 lines) |
| Table | 4 720 | yes | yes |
| **Report** | **2 142** | no | yes (`report_gen`, 995) |
| Enum | 1 470 | yes | yes |
| **PermissionSet** | **1 127** | no | no |
| **Query** | **464** | no | yes (`query_gen`, 465) |
| **XmlPort** | **384** | no | yes (`xmlport_gen`, 550) |
| **Interface** | **209** | no | yes (`interface_gen`) -- board:0027 |
| **Entitlement** | 206 | no | no |
| **Profile** | 59 | no | no |
| **ControlAddIn** | 20 | no | no |

**Three of twelve kinds, and the missing ones are not the tail**: Page is the second largest object
kind in BC and Report the fourth.

## And the body is the bigger half

| | lines |
|---|---|
| `agiru` `src/gen/BodyWriter.cpp` | 445 |
| `openerp` `generator/body_emitter/` | 4 075 |

That is the AL statement and expression translation -- the actual CODE inside a procedure, as
against its signature. A nine-fold gap, and it is the half that decides whether a translated
codeunit DOES anything. The 3 927 lines of `src/al` and `src/gen` together are a fifth of the
predecessor's 18 894, and the predecessor was 97 % on a subset rather than complete.

## What this item is for

It is not a plan; it is the DENOMINATOR. Every other generator item -- board:0027 for interfaces,
board:0030 for pages, board:0033 for extensions -- is a row of the table above, and the count of
kinds translated belongs beside the counts of objects parsed in the run summary. A transpiler that
reads 4 029 codeunits and no page is not 4 029 objects along; it is three kinds of twelve.

## What is true when this closes

- Every AL object kind BCApps declares has a generator, and the run summary counts them by kind.
- A kind with no generator is REPORTED by name and count, not skipped in silence.
- The body emitter translates the statement and expression grammar the BaseApp uses, measured
  against the same corpus rather than against the cases that happen to be written.
