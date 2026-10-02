# System declaration regression inputs

Five original System.app 28.0.53152.0 / runtime 17.0 declarations, normalized to LF.
Package SHA256: `5b72ba127cb2221722f02544bfb3f3bd5a50402bf92bfab6d7e584e049b9681d`.
Original Microsoft notices and MIT permission text are retained in `License.txt`.

`make gate GATE=PlatformSourceGate JOBS=2` compares every declared field, option
position, key and represented property with the handwritten/runtime declarations
and compiler aliases. Negative controls mutate identity, field count/number/type/
length, key order and option population. `Field` checks all 24 fields, compact
native Type codes, classification order and external-name identity/length; wrong
codes and missing package fields fail. Reflection includes all implicit system
fields and maximum-length text roundtrips.

Set `AGIRU_SYSTEM_SYMBOLS` when running `make test` to verify the package and audit
its actual original sources through `test/platform-source.sh`. Missing or invalid
input fails, never falls back. Frozen integration supplies its immutable package
automatically. The C++ gate accepts an optional package-root argument, reads it
before testing, and does not access mutable process environment state.

`Field.Table.al` is also byte-identical in the verified System.app 29.0.55365.0
package from platform 29.0.54011.55407 (SHA256
`f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`).
Other families may differ between releases: 29 adds inherent permissions to the
privacy tables. Report such mismatches; never replace the demo pin or weaken checks.

These are declaration checks, not System-loader activation, SQL migration,
Scope/fieldgroup dispatch, privacy workflows or live provider acceptance. A
`Cloud` declaration is extension availability, not a product integration mandate.
