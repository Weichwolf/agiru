Type:     task
Status:   open
Parent:   0190
Area:     gen
Source:   developer/attributes/devenv-nondebuggable-attribute.md
Verdict:  fehlt
Class:    activation

# A `[NonDebuggable]` symbol keeps its value out of the debugger, and agiru has no debugger

`[NonDebuggable]` marks a method or variable whose values must not be visible when debugging --
the attribute form of what `SecretText` does with a type (`devenv-secret-text.md`: "designed to
protect sensitive values from being exposed through the AL debugger when doing regular or snapshot
debugging").

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**1 505 `[NonDebuggable` declarations** -- a large population for an attribute with no runtime
behaviour, and that is the finding: it is a promise about tooling, not about execution.

## The IST-state

The attribute parses into the raw list and is dropped.

## The choice

**Carried as `constexpr` metadata and acted on by nothing, because agiru has no AL debugger.** The
generated tree is C++ and is debugged with a C++ debugger, which knows nothing about AL's
visibility rules and cannot be asked to honour them.

**Why carry it rather than refuse it.** board:0190's rule is act-or-refuse, and refusing 1 505
declarations would stop the translation of a large part of the System Application over an attribute
that changes no answer. Recording it as known-and-inert is the honest third state, and the metadata
is what a future AL-level debugger or a value dump would read.

**And it is a WARNING worth carrying to a reader**: a `[NonDebuggable]` value in agiru IS visible in
gdb, in a core dump and in a stack trace. That is a real difference from BC and it belongs beside
the declaration rather than being discovered by someone who assumed the guarantee travelled.

## Ordering

Low, and it will stay low: nothing depends on it and no gate can distinguish carried from acted-on
without a debugger to ask.

## Gate, and its negative control

The attribute appears in the generated metadata for an annotated method and not for an unannotated
one. **The negative control is the unannotated method** -- a generator that emits the marker
unconditionally makes the whole 1 505 meaningless.
