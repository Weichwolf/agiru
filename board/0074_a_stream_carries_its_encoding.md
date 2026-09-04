Type: root
State: open
Area: net, rt

# A stream carries the encoding it was created with, and a BLOB goes to and from a file

`Blob.CreateInStream` and `Blob.CreateOutStream` both take an optional `TextEncoding`, and the page
states the default outright: **"The default encoding is MSDos."** `include/type/Blob.h` declares
neither the parameter nor any notion of encoding, and `Import`/`Export` are absent entirely -- the
header says why, and says it honestly:

> THE STREAMS ARE NOT HERE YET. `CreateInStream`, `CreateOutStream`, `Import` and `Export` each take
> or return an AL type the runtime does not have -- InStream, OutStream and a file.

Two of the four have since arrived: `CreateInStream` and `CreateOutStream` return real streams. The
ENCODING did not arrive with them.

## Why the default matters more than it looks

`devenv-file-handling-and-text-encoding.md` exists for this: MSDos, Windows, UTF8 and UTF16 are four
different byte sequences for the same text, and BC's own guidance is to name the encoding
explicitly *"so that all the language-specific characters are represented correctly"*. A stream that
silently writes UTF-8 where BC writes MSDos produces a file that round-trips inside agiru and is
wrong everywhere else -- payment files, data exchange, e-documents. The Data Exchange framework is
the biggest single user of it in the milestone's own codeunits (board:0065).

**And MSDos is not a plausible default to guess at.** It is code page 437/850, not Latin-1 and not
UTF-8, so "we did not implement encodings" means every non-ASCII byte is already wrong rather than
merely untranslated.

## The surface, as the sweep has reached it

| page | state |
|---|---|
| `blob-createinstream-method.md`, `blob-createoutstream-method.md` | stream yes, `TextEncoding` parameter absent |
| `blob-import-method.md`, `blob-export-method.md` | absent -- on-premises only from runtime 9.0, which agiru is |
| `textencoding/textencoding-option.md`, `instream/`, `outstream/`, `file/` | read as the sweep reaches them (board:0071); `File` alone carries 59 door refusals |

## The choice

- **The encoding is a property of the STREAM and is fixed when it is created**, which is what the AL
  signature says: it is a parameter of `CreateInStream`, not of `Read`. So `InStream` and `OutStream`
  gain one member, defaulted to MSDos through board:0072's mechanism.
- **The conversion is a table, not a library.** MSDos code pages are 256-entry maps; UTF-8 and UTF-16
  the standard library can do. That is `constexpr` `.rodata` and no dependency, the same answer
  board:0041 reached for case conversion.
- **`Import`/`Export` are the FILE surface** and arrive with `File`, not before it: a BLOB that can
  be written to a path needs the path rules the `File` pages state.
- **An unknown encoding REFUSES**, naming it. A stream that silently falls back to UTF-8 is the
  `_NilValue` shape board:0035 exists to keep out of this tree.

## AND THE FOUR METHODS DISAGREE ABOUT ZERO BYTES AND LINE ENDINGS, ON PURPOSE

`devenv-write-read-methods-line-break-behavior.md` exists for one paragraph, and it is a gate
waiting to be written (read 2026-09-04, board:0071):

| method | where it stops, or what it adds |
|---|---|
| `Write` | **adds a 0 byte at the end of the stream** |
| `WriteText` | does not |
| `Read` | reads until a **0 byte** or the string's length |
| `ReadText` | reads until a zero byte, **an end-of-line**, the given byte count, or the maximum length |

Its worked example is the gate case itself: `WriteText('A')`, `WriteText()`, `WriteText('B')`,
`WriteText()`, `WriteText('C')` into a BLOB, then three `ReadText` calls give `A`, `B`, `C` -- and
one `Read` over the same bytes gives all three at once, because there is no zero byte in them. Write
the same content with `Write` instead and the zero bytes it adds make `Read` stop between them.

`include/type/Stream.h` already implements the `WriteText()` half correctly and says why ("an empty
write would leave every generated file on one line"); the zero-byte half belongs to the typed
`Write`, which refuses.

## Gate

A round trip through each documented encoding over a string with a character outside ASCII: the
bytes are the code page's bytes, and reading them back gives the string. The default with no
argument is MSDos and the gate asserts the BYTES, not the round trip -- a round trip through one
wrong encoding twice is green.

**Negative control**: write MSDos and read UTF-8 and require the case to go red.

## MS-DOS ENCODING DEPENDS ON THE SERVER'S SYSTEM LOCALE, WHICH THIS TREE CANNOT HAVE

`devenv-file-handling-and-text-encoding.md` (read 2026-09-04, board:0071):

> **Internally, Business Central uses Unicode encoding.** For exporting and importing data with an
> XMLport, it supports MS-DOS, UTF-8, UTF-16, and Windows encoding formats.
> ... When the property is set to MS-DOS, **text is encoded by using the system locale language of
> the computer that is running the Business Central Server instance.** So, if you use MS-DOS
> encoding, you should set the system locale language of the server instance computer to match the
> language of the data.

**That is the one encoding agiru cannot reproduce faithfully and must not pretend to.** CLAUDE.md's
target says "no named machine decides an argument"; MS-DOS encoding makes the SERVER'S LOCALE decide
what bytes an export contains, so the same XmlPort over the same data produces different files on
two machines -- which is the determinism rule, broken by the specification itself.

The honest shape: MS-DOS resolves to an EXPLICIT code page, declared in configuration and defaulting
to CP850, rather than to whatever the host's locale is. It is a named deviation, it is
deterministic, and it is right for the overwhelming majority of the data -- and it is stated at the
declaration so that a reader does not go looking for a locale that is not there.

The direction of conversion is documented and settles what the streams owe: **import reads in the
declared encoding and converts TO Unicode; export converts FROM Unicode and writes in the declared
one.** So the internal representation is Unicode always, which is what `agiru::Text` already is, and
the encoding lives only at the two boundaries.

## `Write` TERMINATES AND `WriteText` DOES NOT, AND THE PAGE GIVES THE GATE

`devenv-write-read-methods-line-break-behavior.md` (read 2026-09-04, board:0071) states the four
rules and then proves them with a worked example:

| method | behaviour |
|---|---|
| `Write` | **adds a 0 byte at the end of the stream** |
| `WriteText` | **does not** |
| `Read` | reads until a **0 byte** or the specified length of the string |
| `ReadText` | reads until a **zero byte, an end-of-line**, the specified number of bytes, or the maximum length |

and `WriteText` with NO argument writes a line break. The example is the gate, verbatim:

```al
MyOutStream.WriteText('A'); MyOutStream.WriteText; MyOutStream.WriteText('B');
MyOutStream.WriteText;      MyOutStream.WriteText('C');
// three ReadText calls read  A, then B, then C
// one Read over the same bytes reads  A<line break>B<line break>C
```

**So the same bytes read back as three values or as one, decided by WHICH read method is called** --
and a runtime that treats `Read` and `ReadText` as the same thing gives one answer to both. The
second half of the page does the same with an explicit CR LF written through `Write`, which is the
case that distinguishes the zero terminator from the line ending.

These are behaviours no signature carries and the `methods-auto` sweep could not have found: both
methods are declared, both have verdicts, and the DIFFERENCE between them is on this page alone.
