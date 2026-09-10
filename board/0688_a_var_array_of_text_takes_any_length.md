# 0688 A var array of Text takes any length

**The finding.** `Consolidate.GetAccumulatedErrors(var NumErrors: Integer; var Errors: array[100]
of Text)` is called with `ErrorText: array[100] of Text[250]`. AL accepts it: a parameter declared
`Text` with no length takes a text of any length, and `var` does not change that. The door models
the two as different types -- `AlArray<Text<0>, 0> &` against `AlArray<Text<250>, 100>` -- and the
call does not compile. 21 AL parameters are declared `array[N] of Text`; 32 generated signatures
carry the untyped form.

**Why the obvious answers do not work.** `AlArray<T, N>` already derives from `AlArray<T, 0>`, so
the SIZE mismatch is solved; what is left is the ELEMENT. `Text<N>` derives from `Text<0>`, so an
array of one is layout-compatible with an array of the other -- but walking a derived array through
a base pointer is undefined behaviour, not merely unusual, and a conversion operator would have to
return a reference to a view it does not own. Generating the parameter as a TEMPLATE would put the
body in the header, which the generated-file rule forbids.

**What is done now:** the one source this blocks leaves the slice with this reason written down --
not "it stopped compiling" but "the shape is not expressible yet". The scalar case (`var X: Text`
taking a `Text[250]`) does not arise, because both sides are generated from the same declaration.

**What would settle it:** an owning view -- `AlArray<Text<0>, 0>` made to hold a COPY on the way in
and write it back on the way out, which is what AL's `var` over a differing length means anyway --
or the length dropped from generated array elements entirely, which is a measurement over
`MaxStrLen` before it is a decision.
