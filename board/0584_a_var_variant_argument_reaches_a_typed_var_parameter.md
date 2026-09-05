Type:     task
Status:   open
Parent:   0035
Area:     gen, rt
Source:   developer/devenv-variant-data-type.md
Verdict:  fehlt
Class:    compile root

# A `var Variant` argument reaches a `var Date` parameter, and the value comes back

`TypeHelper.Evaluate(var Variable: Variant; ...)` hands its `Variable` on to
`TryEvaluateDate(String, Format, CultureName, Variable)` whose parameter is `var Value: Date`. AL
compiles it: the platform unwraps the Variant into the typed slot for the call and writes the
result back into the Variant afterwards. C++ refuses it -- `no viable conversion from 'Variant' to
'Date'` binding a `Date &` -- and `TypeHelper.cpp` is the first codeunit in the census that
`CountryRegionUT` needs and cannot link (loop16, 2026-09-05).

## Population, measured 2026-09-05 over `~/Git/BCApps/src`

Two call sites in `TypeHelper.Codeunit.al:66,68`; the shape is "a Variant passed by reference to a
typed var parameter" and the transpiler does not see the callee's parameter type at the call site.

## The choice

A proxy at the call site is the only faithful shape: `detail::VarSlot<Date> slot(variant)` that
holds a `Date` copied out of the Variant, binds as `Date &`, and writes back into the Variant in
its destructor. The generator emits it only where the callee is RESOLVED and its parameter is a
typed `var` while the argument is declared `Variant` -- both known from the declarations. Nothing
is guessed; where the callee is unresolved the call stays a compile error, which is the loud
answer.

## Gate

A codeunit passing `var V: Variant` into `var D: Date` compiles and the Variant holds the Date the
callee wrote. Negative control: a non-var Variant argument is not wrapped.
