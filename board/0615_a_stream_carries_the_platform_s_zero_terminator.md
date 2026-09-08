Type:     task
Status:   open
Area:     net
Source:   the UT ranking after the two unlinked sources compiled, 2026-09-08
Class:    activation

# A stream's `Write` and `Read` carry the platform's zero terminator

**117 UT FAILURES ARE ONE REFUSAL**, and the reason written beside it is only half true:

```
a typed Write puts the platform's own binary layout into the stream, and this runtime does not
have it. Only the text forms are here
```

**FOR A NUMBER THAT IS RIGHT AND FOR A TEXT IT IS NOT.**
`devenv-write-read-methods-line-break-behavior.md` states the layout outright, and states it as the
one difference between the two pairs:

- `Write` adds a 0 byte at the end of the stream; `WriteText` does not.
- `Read` reads until a 0 byte or the specified length.
- `ReadText` reads until a zero byte, an end-of-line, the specified number of bytes, or the
  maximum length.

The page then works an example through both, and the example is the gate case: `A<CR><LF>B` written
with `Write` reads back as ONE value through `Read` and as TWO through `ReadText`.

## The population says it is the text form that is called

**111 `.Write(` CALL SITES OVER `apps/`, and the ones that are not `WriteText` write a TEXT** --
`OutStream.Write(FilterView)`, `Write(TextValue)`, `Write(BodyText)`, `Write(ManifestText)`,
`Write(Filter)`, `Write(TextInput)` (measured 2026-09-08). Not one writes a Decimal or a Date. So
the refusal that costs 117 tests is refusing the case the documentation SPELLS OUT, and keeping the
case it cannot know.

## What is taken

**THE TEXT OVERLOADS ARE WRITTEN AND THE REST STILL REFUSES**, split by a constraint rather than by
a runtime check: `Write(T)` where `T` reads as a `std::string_view` writes the bytes and a zero
byte; every other `T` refuses by the same message it does today. `Read(var T)` where `T` assigns
from a `std::string_view` reads to the next zero byte and consumes it.

**`Write` RETURNS THE TERMINATOR IN ITS COUNT.** The page calls `Written` "the number of bytes that
were written" and says the zero byte is written, so it is one of them. No call site over `apps/`
reads the return value (measured 2026-09-08), so the number is derived from the page rather than
from a test.

## What is NOT taken in this item

**`ReadText` DOES NOT STOP AT A LINE BREAK YET, and that is a second activation.** It reads the
whole rest of the stream today. Making it stop is documented and is what the example demands -- and
it can only be measured apart from this item, because a test that writes several lines with
`WriteText` and reads them back whole passes today and would stop passing.

## What proves it

The example from the page as a gate case, both ways round, and the UT count over the 78 codeunits.
The negative control is the numeric form: `OutStream.Write(SomeDecimal)` must still refuse, because
that layout is still unknown.
