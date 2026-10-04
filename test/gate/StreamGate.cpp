#include "dotnet/BinaryReader.h"
#include "dotnet/BinaryWriter.h"
#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Code.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
  constexpr agiru::Integer kPrefixLength = 5;
  in.Read(code, kPrefixLength);
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

void WritesRetainStorageAndExactBytes() {
  std::vector<std::uint8_t> initial{'p'};
  constexpr std::size_t kReserved = 64;
  initial.reserve(kReserved);
  Blob blob;
  blob.Set(std::move(initial));
  auto out = blob.CreateOutStream();
  auto in = blob.CreateInStream();
  const std::size_t capacity = blob.Bytes().capacity();
  CHECK_TRUE("empty text preserves reserved storage",
             out.WriteText("") == 0 && blob.Bytes().capacity() == capacity);
  CHECK_TRUE("empty raw write preserves reserved storage",
             out.WriteBytes("") == 0 && blob.Bytes().capacity() == capacity);
  CHECK_TRUE("raw binary bytes have no terminator",
             out.WriteBytes(std::string_view("\0\xff", 2)) == 2);
  CHECK_TRUE("bounded Write adds one zero terminator", out.Write(std::string_view("abcd"), 2) == 3);
  CHECK_TRUE("an empty Write still writes its terminator", out.Write(std::string_view{}) == 1);
  CHECK_TRUE("writes within capacity never replace storage", blob.Bytes().capacity() == capacity);
  CHECK_TEXT("a preexisting input stream sees the exact appended bytes",
             in.ReadBytes(100),
             std::string_view("p\0\xff"
                              "ab\0\0",
                              7));
  CHECK_TRUE("no bytes are lost beyond the stream end", in.EOS());
}

void SmallWritesGrowAmortizedStorage() {
  constexpr std::size_t kWrites = 4096;
  constexpr std::size_t kMaximumGrowths = 13;
  Blob blob;
  auto out = blob.CreateOutStream();
  std::size_t capacity = 0;
  std::size_t growths = 0;
  bool counts = true;
  for (std::size_t index = 0; index < kWrites; ++index) {
    counts = (out.WriteBytes("x") == 1) && counts;
    if (capacity != blob.Bytes().capacity()) {
      capacity = blob.Bytes().capacity();
      ++growths;
    }
  }
  CHECK_TRUE("all small writes return their byte count", counts);
  CHECK_TRUE("small writes allocate logarithmically, not once per write",
             growths <= kMaximumGrowths);
  CHECK_TRUE("storage remains bounded by twice the accumulated bytes", capacity <= kWrites * 2);
  auto in = blob.CreateInStream();
  CHECK_TEXT("amortized append retains every byte",
             in.ReadBytes(static_cast<agiru::Integer>(kWrites)),
             std::string(kWrites, 'x'));
}

void WritesPreserveBorrowedSelfInput() {
  Blob blob;
  auto out = blob.CreateOutStream();
  static_cast<void>(out.WriteBytes("abcd"));
  std::string_view borrowed(reinterpret_cast<const char *>(blob.Bytes().data()), blob.Length());
  CHECK_TRUE("self input survives storage growth", out.WriteBytes(borrowed) == 4);
  borrowed = std::string_view(reinterpret_cast<const char *>(blob.Bytes().data()) + 1, 3);
  CHECK_TRUE("self input also survives terminated writes", out.Write(borrowed, 2) == 3);
  auto in = blob.CreateInStream();
  CHECK_TEXT("self input retains its original bytes",
             in.ReadBytes(100),
             std::string_view("abcdabcdbc\0", 11));
  OutStream unbound;
  bool refused = false;
  try {
    static_cast<void>(unbound.WriteBytes(""));
  } catch (const Error &) { refused = true; }
  CHECK_TRUE("even empty writes require a bound source", refused);
}

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
  const Blob empty;
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

} // namespace

int main() {
  return gate::Run("Stream", [] {
    WhatIsWrittenLandsInTheBlob();
    WriteTextWithNoArgumentWritesALineBreak();
    ReadingWalksTheStreamAndStopsAtItsEnd();
    ReadOfATextTakesTheTextForm();
    ATypedReadOrWriteRefuses();
    WritesRetainStorageAndExactBytes();
    SmallWritesGrowAmortizedStorage();
    WritesPreserveBorrowedSelfInput();
    ABinaryWriterAndReaderRoundTripANote();
  });
}
