# 0686 An object name declared by two apps is translated once

**The finding.** Chain 101 failed at the LINK, not the compile: `Code Coverage Detailed` is
xmlport **9991** in the BaseApp and xmlport **130471** in the Test Runner, both in namespace
`System.TestTools.CodeCoverage`, so the two generated classes are one symbol and `ld.lld` reported
six duplicates. `Permission Set Buffer` is the same shape between `system` and `base`.

**Why BC allows it and this tree cannot.** In BC the two apps are never installed together, so the
names never meet. Here every app is linked into ONE image, which is the whole point of the app
boundary being a BUILD boundary (CLAUDE.md): the linker enforces direction, and it also sees every
symbol at once.

**The choice.** The first app to declare a `<namespace>/<kind>/<name>` writes it; a second one is
SKIPPED and the collision is counted and printed --
`declared 2 object name(s) are declared by two apps and translated once`, with both apps named.
The check sits at the one place every generated file is written, so it holds for every kind
without a per-kind list, and the identity it compares is the file path BELOW the app directory,
which is exactly the C++ symbol's identity: namespace, kind and name.

**What it does NOT do, and why that is the honest limit.** The reference index is keyed by name
alone, so a call to `Code Coverage Detailed` from either app binds to the one that was kept. Both
of this pair export code coverage, so the difference is invisible here; where it stops being
invisible the answer is per-app resolution, which is a bigger item than this one and belongs with
`apps.json`'s dependency graph.

**Measured.** Chain 102, A/B against chain 101 -- which never reached a milestone, so the base is
chain 97's 1 590.

**The slice follows the decision.** `test/slice` named both copies, and the second no longer
exists, so the build died on a missing file. Two lines are gone from it -- which is the one case
where the slice may shrink: not "it stopped compiling" but "the transpiler no longer writes it,
because the name is translated once". The kept copy is still in the slice and still compiles.
