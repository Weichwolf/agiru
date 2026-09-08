Type:     measurement
Status:   open
Area:     cli
Source:   a user asking whether the UT are transpiled at all, 2026-09-08
Class:    measurement

# The milestone's denominator is 2 310 over 80 when counted from the TEXT

**THE GOAL SAYS 2 291 `[Test]` PROCEDURES IN 78 CODEUNITS, AND THE TEXT SAYS 2 310 IN 80.**
Counted 2026-09-08 with the goal's own rule -- every `.al` under `~/Git/BCApps/src/Layers/W1/Tests`
whose codeunit declares `Subtype = Test` and whose name ends in ` UT`, `-UT` or `.UT`, its `[Test]`
attributes counted as lines of text:

| | codeunits | `[Test]` |
|---|---|---|
| the goal | 78 | 2 291 |
| the text, 2026-09-08 | **80** | **2 310** |
| generated with every `[Test]` accounted for | 78 of 80 | 2 296 |
| not generated at all (`Microsoft.Integration.Graph`, out of `scope.json`) | 2 | 14 |

The three names that end in `-UT` or `.UT` rather than ` UT` -- `Analysis View Table-UT`,
`Autom. Payment Registration.UT`, `Incoming Doc. To Data Exch.UT` -- are inside both counts. The
difference is not a rule: BCApps `main` moved under the goal, and 2 291 was measured on 2026-09-03.

**AND THE CHECK THE QUESTION ASKED FOR IS DONE: EVERY GENERATED UT CODEUNIT CARRIES EVERY
`[Test]`.** Per codeunit, the `[Test]` lines in the `.al` against the emitted
`std::array<TestMethod, N>` -- 78 of 78 equal, `PriceSourceUT` 115 and 115. What looked like
"almost empty headers" under `apps/tests/` is the 855 objects of a kind with no generator
(reports, queries, XmlPorts), which carry a number, a name and a refusing surface on purpose.

## What is wanted

`agiru run-tests` prints the denominator FROM THE TEXT beside its own count, so the two cannot
drift apart unnoticed -- the goal's own sentence, "counted from the TEXT and never from the
parser", made into a line of output. Until then the milestone is written as 2 310, and the two
codeunits `scope.json` excludes are a scope decision beside board:0619's Dataverse one.
