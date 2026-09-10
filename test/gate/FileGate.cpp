#include "runtime/Error.h"
#include "type/File.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "Check.h"

#include <filesystem>
#include <string>

namespace {

/// AL'S `File` IS A FILE, and every one of its methods used to be a stub that threw: 46 UT cases
/// stopped at `File.CreateTempFile()` alone. `file-createtempfile-method.md` says it "creates a
/// temporary file" and opens it, so the name is real, the file is there, and what is written
/// through the handle is on disk once it closes.
void ATempFileIsMadeOpenedAndWritten() {
  agiru::File made;
  CHECK_TRUE("a temporary file is created", static_cast<bool>(made.CreateTempFile()));
  const std::string name = made.Name();
  CHECK_TRUE("it has a name", !name.empty());
  CHECK_TRUE("and the file is there", static_cast<bool>(agiru::File::Exists(name)));
  CHECK_TRUE("under the temporary path", static_cast<bool>(agiru::File::IsPathTemporary(name)));

  made.Write(std::string_view("first"));
  made.Write(std::string_view("second"));
  made.Close();

  agiru::File read;
  CHECK_TRUE("it opens again", static_cast<bool>(read.Open(name)));
  agiru::Text<0> line;
  CHECK_TRUE("the first line comes back", read.Read(line) > 0);
  CHECK_TEXT("as it was written", std::string(std::string_view(line)), "first");
  CHECK_TRUE("and the second", read.Read(line) > 0);
  CHECK_TEXT("in order", std::string(std::string_view(line)), "second");
  CHECK_TRUE("and then nothing", read.Read(line) == 0);
  read.Close();

  CHECK_TRUE("Erase removes it", static_cast<bool>(agiru::File::Erase(name)));
  CHECK_TRUE("and it is gone", !static_cast<bool>(agiru::File::Exists(name)));

  // THE NEGATIVE CONTROL: opening what is not there answers false rather than raising, and
  // erasing it twice answers false the second time.
  agiru::File missing;
  CHECK_TRUE("a file that is not there does not open",
             !static_cast<bool>(missing.Open(name)));
  CHECK_TRUE("nor erase", !static_cast<bool>(agiru::File::Erase(name)));
}

/// A STREAM OVER AN OPEN FILE READS WHAT THE FILE HOLDS, which is how the BaseApp imports:
/// `File.Open(Name); File.CreateInStream(In); In.ReadText(...)`.
void AStreamOverAFileReadsIt() {
  agiru::File written;
  CHECK_TRUE("a file is made", static_cast<bool>(written.CreateTempFile()));
  const std::string name = written.Name();
  written.Write(std::string_view("payload"));
  written.Close();

  agiru::File opened;
  CHECK_TRUE("and opened", static_cast<bool>(opened.Open(name)));
  agiru::InStream reading;
  opened.CreateInStream(reading);
  agiru::Text<0> held;
  reading.ReadText(held);
  CHECK_TEXT("the stream reads what was written",
             std::string(std::string_view(held)).substr(0, 7), "payload");
  opened.Close();
  static_cast<void>(agiru::File::Erase(name));
}

} // namespace

int main() {
  return gate::Run("File", [] {
    ATempFileIsMadeOpenedAndWritten();
    AStreamOverAFileReadsIt();
  });
}
