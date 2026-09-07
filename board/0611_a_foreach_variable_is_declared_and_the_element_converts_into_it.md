Type:     task
Status:   open
Area:     gen, net
Source:   the vtable of `IDAutomation1DProvider`, which stops `agiru run-tests`, 2026-09-07
Class:    activation

# A `foreach` variable is declared, and the element converts into it

**AL DECLARES THE LOOP VARIABLE AND THE GENERATOR THROWS THE DECLARATION AWAY.**

```al
var
    CurrentBarcodeSymbology: Enum "Barcode Symbology";
begin
    foreach CurrentBarcodeSymbology in Enum::"Barcode Symbology".Ordinals() do
        if GetBarcodeFontEncoder(CurrentBarcodeSymbology, DummyEncoder) then
            Result.Add(CurrentBarcodeSymbology);
```

```cpp
for ([[maybe_unused]] auto &CurrentBarcodeSymbology :
     ::agiru::Enum<...BarcodeSymbology_Enum>::Ordinals()) {
```

`Ordinals()` answers a `List of [Integer]`, so `auto &` deduces `Integer` -- and `Result` is a
`List of [Enum "Barcode Symbology"]`, so `Result.Add(CurrentBarcodeSymbology)` does not compile.
**AL converts the ordinal into the declared enum**, and it can because an AL enum IS its ordinal.

## What is missing, and it is two things

1. **The generator emits `auto &` where AL wrote a TYPE.** Everywhere else the element type and the
   declared type agree, so `auto &` has been right by accident. The declaration is in the AST and
   the emission has to use it.
2. **`Enum<E>` has no conversion from an `Integer`.** It has `FromInteger` -- `enum-data-type.md`
   names it -- and every conversion it does have is from another ENUM. The loop above needs the
   implicit one.

## Why it is not done in the round that found it

**AN IMPLICIT `Enum(Integer)` IS A CATCH-ALL, and this session has now paid for two.** board:0608's
`operator+(Text<0>, Guid)` swallowed every text join the door had not declared, and the Guid
constructor swallowed `PriceSourceList.Add(Type, Code[20])`. A `Enum<E>` constructible from any
integer makes every `Enum<E>` parameter reachable by every integer expression in the tree, and the
failure mode is the one all three share: it compiles, it runs, it returns a value.

**AND THE ONE MEASUREMENT THAT WAS CHEAP TO TAKE SETTLES HALF OF IT: `::agiru::Integer` IS
`std::int32_t`.** It is a type ALIAS and not a class (`include/type/Integer.h`), so a constructor
`Enum(Integer)` is a constructor from `int` -- every integer literal and every integer expression
in the tree converts to every enum, in ONE user-defined conversion. Written and taken back the same
round, before the build: the `\warning` that would have defended it ("it takes `Integer` and not
`int`") is simply false, and the catch-all is the whole one.

**So the implicit road is closed and the remaining question is which of the other two:** an
`explicit Enum(Integer)` with the generator direct-initialising the loop variable from its DECLARED
type, or a `foreach` emission that converts only where the declared type and the element type
disagree. Both need the generator to know the declared type, which the scope does not answer today.

**The population that wants it is ONE call site today** -- `IDAutomation1DProvider` -- and its cost
is one vtable of the eight that stop `agiru run-tests`. That is the whole argument for filing it
rather than guessing at it.

## What proves it

`ldd -r build/agiru` names no undefined vtable for
`agiru::System::Text::IDAutomation1DProvider_Codeunit`, and the source is in `test/slice`. The
negative control is the enum's own ordinals: `Enum::"Barcode Symbology".Ordinals()` must yield the
DECLARED numbers and not `0..n-1`, which `include/type/Enum.h` already asserts and 103 of the
BaseApp's 576 enumerations would otherwise break.
