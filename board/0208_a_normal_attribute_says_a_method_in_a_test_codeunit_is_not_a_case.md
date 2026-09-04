Type:     task
Status:   open
Parent:   0190
Area:     gen
Source:   developer/attributes/devenv-normal-attribute.md
Verdict:  fehlt
Class:    activation

# A `[Normal]` attribute says a method inside a test codeunit is NOT a case

`[Normal]` -- "Specifies that the method is a Normal method", and the page adds the restriction that
matters: "**The Normal attribute can only be set inside codeunits.**"

Inside a `Subtype = Test` codeunit there are three kinds of method
(`devenv-test-codeunits-and-test-methods.md`): **test, handler, and normal**. `[Normal]` names the
third explicitly, and `devenv-tryfunction-attribute.md` gives it a consequence:
"**In test and upgrade codeunits, [TryFunction] only applies to normal methods**". So `[Normal]` is
what makes `[TryFunction]` legal there.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`[Normal` is rare enough that the count is dominated by the word appearing elsewhere; the population
is re-measured with an anchored pattern when this item is worked. Saying so is cheaper than carrying
a number that is wrong.

## The IST-state

The attribute parses into the raw list and is dropped. `TestsOf` (`src/gen/CodeunitWriter.cpp:80`)
collects procedures by `[Test]`, so a `[Normal]` method is already not a case -- **the current
behaviour is right by accident**, because the classification is positive rather than negative.

## The choice

Two `static_assert`s and no runtime behaviour:

- `[Normal]` outside a codeunit is a translation error naming the object.
- `[TryFunction]` on a method in a `Subtype = Test` or upgrade codeunit that is NOT `[Normal]` is a
  translation error -- which is board:0226's restriction, expressed where the attribute that lifts
  it lives.

**Nothing is emitted.** A normal method is an ordinary method, and the attribute's whole job is to
say so to the compiler.

## Ordering

After 0223 (`[Test]`) and 0226 (`[TryFunction]`), because it exists to constrain them.

## Gate, and its negative control

A `[TryFunction]` on a `[Normal]` method inside a test codeunit translates; the same without
`[Normal]` must FAIL the build.

**The negative control is the second case.** Today both translate, so the gate goes red before the
change and green after -- which is the only way to see the check exists.
