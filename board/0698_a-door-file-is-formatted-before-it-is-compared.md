# 0698 A door file is formatted before it is compared

**The finding, and it cost half an hour here.** `scripts/gen_builtins.py` LOOKS non-idempotent when
it is run by hand: generate, and `include/Builtins.h` loses 349 lines; generate again, and they
come back. The gate `test/door-reproduces.sh` passes throughout, which reads as the gate being
blind -- and it is not. The generator writes UNFORMATTED output; the gate formats it before
comparing, which is why its own two states agree and a hand run's do not. A hand run leaves the
tree in the unformatted phase, and the NEXT hand run reads a differently-wrapped header when it
scrapes `(name, arity)` out of the door -- a declaration whose parameters clang-format folded onto
one line is scraped where the wrapped one was missed. So the phase difference is the SCRAPE reading
its own previous output's line breaks.

**The rule that follows:** `python3 scripts/gen_builtins.py` is never run alone. It is run the way
the gate runs it -- generate, then `clang-format -i` both files -- or its output is not comparable
with anything, including itself.

**What was nearly filed instead:** an item claiming the gate was blind and had to run the generator
twice. It would have been a false finding with a measurement attached, which is worse than none;
the measurement was real and the conclusion was not. Recorded here because the shape recurs: a
number that reproduces is not yet a cause.

**Where it should be enforced:** the generator should format its own output before writing it, so
that a hand run and the gate produce the same bytes. Two lines in the script; not done here because
the round it interrupted was a builtin batch and this is a separate change with its own gate.
