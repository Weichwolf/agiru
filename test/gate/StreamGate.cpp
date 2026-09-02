#include "Check.h"
#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"

#include <string>

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

  std::string read;
  CHECK_TRUE("a bounded read takes that many", in.ReadText(read, 3) == 3);
  CHECK_TEXT("from the front", read, "abc");
  CHECK_TRUE("and the position moves", in.Position() == 4);

  CHECK_TRUE("the rest comes out in one read", in.ReadText(read) == 3);
  CHECK_TEXT("and it is the rest", read, "def");
  CHECK_TRUE("now it is at the end", in.EOS());
  CHECK_TRUE("and a further read takes nothing", in.ReadText(read) == 0);

  in.ResetPosition();
  CHECK_TRUE("resetting starts again", in.Position() == 1 && !in.EOS());
  CHECK_TRUE("a read longer than the stream takes what is there", in.ReadText(read, 100) == 6);
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

int main() {
  return gate::Run("Stream", [] {
    WhatIsWrittenLandsInTheBlob();
    WriteTextWithNoArgumentWritesALineBreak();
    ReadingWalksTheStreamAndStopsAtItsEnd();
    ATypedReadOrWriteRefuses();
  });
}
