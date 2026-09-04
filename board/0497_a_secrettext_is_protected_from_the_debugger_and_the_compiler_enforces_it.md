Type:     task
Status:   open
Parent:   0035
Area:     net, gen
Source:   developer/devenv-secret-text.md
Verdict:  fehlt
Class:    activation

# A `SecretText` is protected from the debugger, and the compiler enforces part of it

> The `SecretText` data type "is designed to protect sensitive values from being exposed **through
> the AL debugger** when doing regular or snapshot debugging. Its use is recommended for applications
> that need to handle any kind of credentials like API keys, custom licensing tokens, or similar."
>
> **"Any value of type `Text` or `Code` can be ASSIGNED to a `SecretText` value."**
>
> **"The AL compiler GUARANTEES that a hardcoded credential can't be assigned DIRECTLY to a
> destination of type `SecretText`."**
>
> A credential's lifetime is **retrieval, transit and consumption**, and it is exposed for all three
> unless protected by **`[NonDebuggable]`** on the procedure or the variable.
>
> The example uses **`SecretStrSubstNo('Bearer %1', Credential)`** to compose a header without
> unwrapping the secret.

**Two guarantees, and only one of them is a runtime concern.**

**The compiler guarantee is a `static_assert`-shaped rule**: a string literal may not be assigned to a
`SecretText`. It is decidable from the syntax, the AL compiler enforces it, and a transpiler that
allowed it would accept code AL rejects. That is the half agiru can hold completely, and it is worth
holding because the failure mode is a credential committed to a repository.

**The debugger guarantee has no counterpart here yet.** agiru has no AL debugger and no snapshot
debugging, so "protected from the debugger" protects against something that does not exist -- and
that makes it easy to conclude the type is a no-op. **It is not**: `SecretStrSubstNo` exists because
`StrSubstNo` would unwrap the value, so the type's real content is which operations are ALLOWED on it,
and that survives having no debugger.

board:0035 already records the surface: `src/rt/Builtins.cpp:74` -- `Clear(SecretText)` refuses the
door, so the type is declared and its operations are not.

## The type's rule, stated as this item's subject

A `SecretText` may be assigned FROM `Text` and `Code`, and may not be implicitly converted back.
Everything that would produce a plain `Text` from it -- concatenation, `StrSubstNo`, `Format` --
either has a secret-preserving twin or is refused. That is an ordinary C++ strong type with no
`operator Text`, and it is one of the clearest cases in this tree for CLAUDE.md's "every construct the
type system can carry, it carries".

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SecretText` is a TYPE, not a property, so this sweep's declaration pattern does not apply; the count
of `SecretText` variable declarations and of `[NonDebuggable]` belongs to board:0028's builtin census
and board:0190's attribute family. **Stated rather than guessed.**

## The IST-state

`include/type/SecretText.h` is the door's per-type file (board:0051); `src/rt/Builtins.cpp:74` refuses
`Clear(SecretText)`. Whether the type has an unwrapping conversion is exactly the question this item
opens and is not measured here.

## The choice

A strong type with no conversion to `Text`, constructible from `Text` and `Code`, with the
secret-preserving operations as named methods. **The literal prohibition is a `static_assert` in the
generator**: an AL string literal assigned to a `SecretText` destination is a translation error, which
is what the AL compiler does.

`[NonDebuggable]` is carried and has no consumer, and that is recorded rather than treated as done.

## Ordering

With board:0051's per-type door and board:0035's surface. The literal check is independent and cheap.

## Gate, and its negative control

`SecretStrSubstNo('Bearer %1', Credential)` produces a `SecretText`; assigning a string literal to a
`SecretText` fails to transpile; there is no expression that yields a plain `Text` from a
`SecretText`.

**The negative control is the third assertion** -- it is a claim about what does NOT compile, so the
gate is a translation unit that must FAIL, and an implementation with a convenience conversion passes
the first two.
