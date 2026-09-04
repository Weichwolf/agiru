Type: root
State: open
Area: rt, gen

# AL's `Text` is a TYPE WITH METHODS, and the door hands back a `std::string`

`LowerCase(Guid).Replace("{", "").Replace("}", "")` is one AL line from
`Import Consolidation from API`, and neither half of it compiles:
`LowerCase` returns a `std::string`, which has no `Replace`, and its argument is
a `Guid`, which has no conversion to text.

AL has ONE text type and it carries 21 methods -- `methods-auto/text/` gives
`Replace`, `Split`, `Trim`, `Substring`, `Contains`, `StartsWith`, `IndexOf`,
`PadLeft` and the rest -- and every expression of type `Text` has all of them,
whether it came from a variable, a field or a call. This tree has TWO spellings:
`StringValue` (which carries the methods, and which `Code<n>` and `Text<n>`
derive from) and `std::string` (which the door returns and a generated procedure
declares). The methods exist on one and the calls land on the other.

## Measured over `Layers/W1`, 2026-09-04

| | |
|---|---:|
| door builtins returning `std::string` | 17 |
| AL procedures declared to return `Text` | 2 838 |
| AL variables declared bare `Text` | 8 039 |
| a Text METHOD called on the result of an expression -- `X(...).Replace(...)` | 117 |
| of those, on a builtin's result | 67 |

117 is the count that does not compile today, and it is a floor: it counts
`).Method(` only, so a method on a bare procedure result reached through a
variable is not in it.

## Step one is done: the BUILTINS return a text

2026-09-04: the eleven text-returning builtins in the hand-written door --
`LowerCase`, `UpperCase`, `ConvertStr`, `CopyStr`, `DelChr`, `DelStr`, `IncStr`,
`InsStr`, `PadStr`, `SelectStr`, `CompanyName` -- return `Text<0>` instead of
`std::string`. The whole tree still builds and every gate stays green, because
`Text<0>` derives from `StringValue` and `StringValue` reads as a
`std::string_view`, which is what every consumer wanted.

**IT IS NOT ENOUGH, AND THE NEXT STEP IS THE HEADER ORDER.** `LowerCase(X).Replace
("{", "").Replace("}", "")` still fails on the SECOND `Replace`, because
`StringValue`'s own text methods -- `Replace`, `Substring`, `Trim`, `TrimStart`,
`TrimEnd`, `PadLeft`, `PadRight`, `Remove` and the rest, fifteen of them -- return
`std::string` too. They cannot return `Text<0>` where they stand: that type is
declared in `Text.h`, which includes `StringValue.h`, and a member defined inside
the class body needs the return type COMPLETE.

So the step is a header move rather than a signature change: `Text<0>` is the
unbounded string VALUE and belongs beside `StringValue`, leaving `Text.h` with
the sized `Text<N>` alone. Then the fifteen methods return it and a chain of AL
text methods reads as AL wrote it.

## The choice, and why it is not obviously the small one

**`Text<0>` IS THE AL TYPE AND `std::string` IS NOT.** The door already declares
`Text<n>`, deriving from `StringValue`, carrying every documented method, and
`Text<0>` is AL's unbounded `Text`. So the shape is right and the change is
mechanical: a builtin declared to return `Text` returns `Text<0>`, and a
generated procedure declared `: Text` returns `Text<0>` rather than
`std::string`.

What makes it a root rather than a patch is the blast radius:

- **17 door signatures** change, and every call site that assigns the result to
  a `std::string` or hands it to a `std::string_view` parameter must still work
  -- `StringValue` converts to `std::string_view`, so that half is free.
- **2 838 generated procedures** change their return type. Every `return "text";`
  in a generated body must construct one, which `Text<0>` does from a view.
- **`StringValue.h` CANNOT NAME `Text<0>`** -- `Text.h` includes it, not the
  other way round -- so `Split`'s `List<std::string>` return cannot become
  `List<Text<0>>` without moving the declaration. The `List<T>` converting
  constructor added on 2026-09-04 is the seam that holds until it does.

**AND THE ARGUMENT SIDE IS THE SAME QUESTION.** `LowerCase(Guid)` compiles in AL
because a Guid reads as text where text is wanted, and BC's own tests rely on it
(`CopyStr(CreateGuid(), 1, 20)`). C++ must not give `Guid` a conversion operator:
a Guid is 16 bytes and its text is 38, and one sits in every record as
`SystemId`, so a conversion would put the text's storage on every row in memory.
The `+` operators carry it where text is joined (done); an ARGUMENT needs the
parameter to accept it, which is the same door-wide decision as the return.

## The gate

A generated procedure returning `Text`, called and then asked for `.Replace`,
`.Split` and `.Substring` -- and a builtin's result asked the same. The negative
control is a `Code[20]` return: it must NOT become `Text<0>`, because the length
is part of the declaration and `Code` normalises.
