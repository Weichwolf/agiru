# System declaration regression inputs

Four original System.app 28.0.53152.0 / runtime 17.0 declarations, normalized to LF.
Package SHA256: `5b72ba127cb2221722f02544bfb3f3bd5a50402bf92bfab6d7e584e049b9681d`.
Original Microsoft notices and MIT permission text are retained in `License.txt`.

`make gate GATE=PlatformSourceGate JOBS=2` compares every declared field, option
position, key and represented property with the handwritten/runtime declarations
and compiler aliases. Negative controls mutate identity, field count/number/type/
length, key order and option population. Reflection includes all implicit system
fields and maximum-length text roundtrips.

Set `AGIRU_SYSTEM_SYMBOLS` when running `make test` to verify the package and audit
its actual original sources through `test/platform-source.sh`. Missing or invalid
input fails, never falls back. Frozen integration supplies its immutable package
automatically. The C++ gate accepts an optional package-root argument, reads it
before testing, and does not access mutable process environment state.

These are declaration checks, not System-loader activation, SQL migration,
Scope/fieldgroup dispatch, privacy workflows or live provider acceptance. A
`Cloud` declaration is extension availability, not a product integration mandate.
