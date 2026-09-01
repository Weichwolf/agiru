Type: bug
State: open
Area: net
Tags: performance

# The decimal divider stops walking bit by bit

`U192::DividedBy` is a bitwise long division: 192 rounds per division, chosen because it is
obviously correct. `Decimal::operator/=` calls it once and `Round` calls it once per rounding --
and rounding happens on every posted amount.

## Reference

**Correct before fast, and that order is not regret.** The first version of this file carried three
findings that were structural rather than cosmetic, and one of them -- a bare `32` in a hand-picked
bound -- exposed a silently lost carry in the wide multiply. A clever divider written first would
have hidden it.

**The target decides the urgency** (board:0006): Cortex-A53 at 1 GHz, in-order. 192 iterations with
a compare and a conditional subtract each is on the order of a thousand cycles per division. A
posting run rounds thousands of amounts. This is not a micro-optimisation, it is whether the machine
finishes.

**The measurement that has to come first:** how often division is actually on the path. If `Round`
dominates, the cheaper fix is not a faster divider but a `Round` that does not divide at all --
scaling by a power of ten is a shift of the scale, not a division, whenever the precision is a power
of ten, and 0.01 is the overwhelmingly common case in BC.

**The choice, pending that measurement:** the fast path for a power-of-ten precision first, Knuth
algorithm D for the general divider second. In that order, because the first is smaller and probably
removes most of the traffic.

## What will be true

- [ ] Division and rounding are measured on the target, with the population quoted.
- [ ] `Round` with a power-of-ten precision performs no long division.
- [ ] The general divider is Knuth D over 64-bit limbs.
- [ ] Proof: the 33 existing `Decimal` gate checks stay green, unchanged, through both rewrites --
      they were written before the speed work, which is what makes them a proof rather than a
      description.
- [ ] **Negative control**: an off-by-one in the fast path must make an existing case go red. If no
      case falls, the fast path is untested rather than correct.
