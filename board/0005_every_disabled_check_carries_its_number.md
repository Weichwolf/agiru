Type: bug
State: open
Area: build
Tags: gate, measured

# Every disabled check carries its measured finding count, not only its reason

`.clang-tidy` switches four checks off and says WHY for each. What none of them carries is a NUMBER
-- and the file says so itself: "No finding counts stand beside the exceptions, because there was
nothing to count yet."

That is right today and becomes wrong the moment `src/` has volume. A reason without a number is a
claim about the size of a problem nobody has measured.

## Reference

**outshine** measures every exception over two representative translation units before choosing it,
and writes the number in the line above: `modernize-use-trailing-return-type` (897),
`readability-uppercase-literal-suffix` (194). The value is not in the number itself but in the fact
that it makes an exception UNCOMFORTABLE: whoever switches off 3 667 findings can see that they do.

**The counter-example in the same tree:** there `modernize-avoid-c-style-cast` (91) and
`modernize-avoid-c-arrays` (281) stood acknowledged as right and switched off anyway, with "stage
two" as the reason. A stage without a date is a promise; only the number was the schedule.

**The choice:** the same mechanism, but the measurement comes AFTER the volume. Two representative
units from different tiers -- one from `src/gen` (emitter, heavy on string work) and one from
`src/rt` (semantics, heavy on branching).

**What is already measured** (2026-09-01, three units: `Main.cpp`, `Decimal.cpp`, `DecimalGate.cpp`):
with all seven categories on and the four exceptions off, **41 findings** came up across two rounds
-- 8 in the client, 33 in the decimal work -- all genuine, all repaired, none of them taste. Two of
them were answered with better structure rather than an exception, and one of them
(`readability-magic-numbers` chasing a bare `32`) exposed a silent lost carry in the wide multiply.
That is the sample which supports the choice of categories. It is too small to support an exception.

## What will be true

- [ ] Every check switched off in `.clang-tidy` carries its measured finding count above it, with
      the population it was measured on.
- [ ] An exception whose number is small is no longer an exception but a repair.
- [ ] The four current exceptions are examined one by one: does each stay taste when measured
      against real volume?
- [ ] Proof: the measurement is quoted with the commit rather than re-derived -- a number written
      into a file has to be right before it goes in.
