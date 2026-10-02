#include "dotnet/BinaryReader.h"
#include "dotnet/BinaryWriter.h"
#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Code.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"

#include <string>
#include <string_view>

using agiru::Blob;
using agiru::Error;
using agiru::InStream;
using agiru::OutStream;

namespace {

/// A STREAM WRITES INTO THE BLOB IT WAS GIVEN, and does not own a copy of it. That is what makes
/// `Rec.Blob.CreateOutStream(Out); Out.WriteText(x)` leave the value in the record -- and a stream
/// that copied would leave the caller's BLOB empty with every test of it green for the wrong
/// reason.
void WhatIsWrittenLandsInTheBlob() {
  Blob blob;
  CHECK_TRUE("a fresh BLOB has no value", !blob.HasValue());

  OutStream out = blob.CreateOutStream();
  CHECK_TRUE("WriteText answers how much it wrote", out.WriteText("hello") == 5);
  CHECK_TRUE("and the BLOB now has one", blob.HasValue());
  CHECK_TRUE("of that length", blob.Length() == 5);

  (void)out.WriteText(" world");
  CHECK_TRUE("a second write appends rather than replacing", blob.Length() == 11);
}

/// `WriteText()` WITH NO ARGUMENT WRITES A LINE BREAK, and the page says so outright: "if you do
/// not specify this, a carriage return and a line feed are written". An empty write would leave
/// every generated file on one line, and nothing would raise.
void WriteTextWithNoArgumentWritesALineBreak() {
  Blob blob;
  OutStream out = blob.CreateOutStream();
  CHECK_TRUE("it writes two characters", out.WriteText() == 2);
  CHECK_TRUE("and they are in the BLOB", blob.Length() == 2);
  CHECK_TRUE("carriage return first", blob.Bytes().at(0) == '\r');
  CHECK_TRUE("then line feed", blob.Bytes().at(1) == '\n');

  // THE NEGATIVE CONTROL: writing the empty string is NOT the same thing.
  Blob other;
  OutStream second = other.CreateOutStream();
  CHECK_TRUE("writing an empty text writes nothing", second.WriteText("") == 0);
  CHECK_TRUE("and leaves the BLOB empty", !other.HasValue());
}

void ReadingWalksTheStreamAndStopsAtItsEnd() {
  Blob blob;
  OutStream out = blob.CreateOutStream();
  (void)out.WriteText("abcdef");

  InStream in = blob.CreateInStream();
  CHECK_TRUE("the length is the whole BLOB", in.Length() == 6);
  CHECK_TRUE("and it starts at position one, as AL counts", in.Position() == 1);
  CHECK_TRUE("nothing has been read yet, so it is not at the end", !in.EOS());

  agiru::Text<0> read;
  CHECK_TRUE("a bounded read takes that many", in.ReadText(read, 3) == 3);
  CHECK_TEXT("from the front", read.Value(), "abc");
  CHECK_TRUE("and the position moves", in.Position() == 4);

  CHECK_TRUE("the rest comes out in one read", in.ReadText(read) == 3);
  CHECK_TEXT("and it is the rest", read.Value(), "def");
  CHECK_TRUE("now it is at the end", in.EOS());
  CHECK_TRUE("and a further read takes nothing", in.ReadText(read) == 0);

  in.ResetPosition();
  CHECK_TRUE("resetting starts again", in.Position() == 1 && !in.EOS());
  CHECK_TRUE("a read longer than the stream takes what is there", in.ReadText(read, 100) == 6);
}

/// `InStream.Read(var Text)` IS THE TEXT FORM, not a typed read: AL's `Read` on a `Text` variable
/// takes the bytes up to the terminator, the way `My Notifications` reads back the filter it wrote
/// with `WriteText`. The overload used to demand `std::assignable_from`, which `Text` fails on the
/// common-reference clause, and so a `Text` went to the typed refusal (17 cases, 2026-09-10).
void ReadOfATextTakesTheTextForm() {
  Blob blob;
  OutStream out = blob.CreateOutStream();
  (void)out.WriteText("WHERE(Field1=1(*))");
  InStream in = blob.CreateInStream();
  agiru::Text<0> filters;
  in.Read(filters);
  CHECK_TEXT("the text comes back whole", filters.Value(), "WHERE(Field1=1(*))");
  in.ResetPosition();
  agiru::Code<10> code;
  in.Read(code, 5);
  CHECK_TEXT("and a Code reads the same way, bounded", code.Value(), "WHERE");
}

/// A TYPED READ OR WRITE REFUSES rather than inventing a binary layout: the platform has its own,
/// and a BLOB written with a made-up one reads back wrong wherever BC reads it.
void ATypedReadOrWriteRefuses() {
  Blob blob;
  OutStream out = blob.CreateOutStream();
  std::string said;
  try {
    out.Write(agiru::Integer{1});
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a typed write refuses", !said.empty());
  CHECK_TRUE("saying it is the binary layout that is missing",
             said.find("binary layout") != std::string::npos);
  CHECK_TRUE("and the BLOB is untouched", !blob.HasValue());
}

} // namespace

/// A NOTE ON A RECORD LINK IS A .NET STRING: `BinaryWriter.Write(string)` puts a 7-bit length
/// prefix before the UTF-8 bytes and `BinaryReader.ReadString` takes it off again, which is how
/// `Record Link Management` writes and reads a note (board:0645). The constructor is spelled the
/// way AL spells it, `X := X.X(stream)`, and an empty stream answers `Position = Length` before any
/// read, which is the module's own emptiness test.
void ABinaryWriterAndReaderRoundTripANote() {
  Blob blob;
  OutStream out;
  blob.CreateOutStream(out);
  agiru::dotnet::BinaryWriter BinWriter{};
  BinWriter = BinWriter.BinaryWriter(out);
  BinWriter.Write(std::string_view("h\xc3\xa4llo"));
  CHECK_TRUE("the prefix is one byte for a short string", blob.Length() == 7);
  InStream in;
  blob.CreateInStream(in);
  agiru::dotnet::BinaryReader BinReader{};
  BinReader = BinReader.BinaryReader(in);
  CHECK_TRUE("Position is zero-based", BinReader.BaseStream().Position() == 0);
  CHECK_TRUE("and Length is the blob's", BinReader.BaseStream().Length() == 7);
  CHECK_TEXT(
      "what was written comes back", std::string(BinReader.ReadString().Value()), "h\xc3\xa4llo");
  Blob empty;
  InStream none;
  empty.CreateInStream(none);
  BinReader = BinReader.BinaryReader(none);
  CHECK_TRUE("an empty stream is at its end before any read",
             BinReader.BaseStream().Position() == BinReader.BaseStream().Length());
  bool threw = false;
  try {
    static_cast<void>(BinReader.ReadString());
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("and reading it is refused", threw);
  agiru::dotnet::BinaryWriter unbound{};
  threw = false;
  try {
    unbound.Write(std::string_view("x"));
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("a writer never bound refuses", threw);
}

int main() {
  return gate::Run("Stream", [] {
    WhatIsWrittenLandsInTheBlob();
    WriteTextWithNoArgumentWritesALineBreak();
    ReadingWalksTheStreamAndStopsAtItsEnd();
    ReadOfATextTakesTheTextForm();
    ATypedReadOrWriteRefuses();
    ABinaryWriterAndReaderRoundTripANote();
  });
}
