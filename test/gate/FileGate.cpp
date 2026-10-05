#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "dotnet/XmlReader.h"
#include "runtime/ErrorValue.h"
#include "type/File.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace {

constexpr std::size_t kXmlReadCapacity = 1024;
constexpr std::size_t kNativeDeclaredTextLimit = 2048;
constexpr char kTerminatedPayload[] = "abc\0tail";
constexpr std::string_view kTerminatedBytes{kTerminatedPayload, sizeof(kTerminatedPayload) - 1};

/// AL'S `File` IS A FILE, and every one of its methods used to be a stub that threw: 46 UT cases
/// stopped at `File.CreateTempFile()` alone. `file-createtempfile-method.md` says it "creates a
/// temporary file" and opens it, so the name is real, the file is there, and what is written
/// through the handle is on disk once it closes.
void ATempFileIsMadeOpenedAndWritten() {
  agiru::File made;
  made.TextMode(true);
  CHECK_TRUE("a temporary file is created", static_cast<bool>(made.CreateTempFile()));
  const std::string name = made.Name();
  CHECK_TRUE("it has a name", !name.empty());
  CHECK_TRUE("and the file is there", static_cast<bool>(agiru::File::Exists(name)));
  CHECK_TRUE("under the temporary path", static_cast<bool>(agiru::File::IsPathTemporary(name)));

  made.Write(std::string_view("first"));
  made.Write(std::string_view("second"));
  made.Close();

  agiru::File read;
  read.TextMode(true);
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
  CHECK_TRUE("a file that is not there does not open", !static_cast<bool>(missing.Open(name)));
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
             std::string(std::string_view(held)).substr(0, 7),
             "payload");
  opened.Close();
  static_cast<void>(agiru::File::Erase(name));
}

std::string FileWithBytes(std::string_view bytes) {
  agiru::File file;
  static_cast<void>(file.CreateTempFile());
  const auto name = file.Name();
  agiru::OutStream output;
  file.CreateOutStream(output);
  static_cast<void>(output.WriteBytes(bytes));
  file.Close();
  return name;
}

void BinaryTextReadsDoNotSplitLines() {
  constexpr std::string_view xml = "<?xml version=\"1.0\"?>\n<!DOCTYPE rootNode>\n<rootNode/>\n";
  const auto name = FileWithBytes(xml);
  agiru::File file;
  CHECK_TRUE("a fresh File defaults to binary mode", !file.TextMode());
  CHECK_TRUE("the binary XML file opens", file.Open(name));
  CHECK_TRUE("the file pointer starts at zero", file.Pos() == 0);
  agiru::Text<kXmlReadCapacity> text;
  CHECK_TRUE("binary Read reports every byte", file.Read(text) == xml.size());
  CHECK_TEXT("binary Read retains XML declaration, doctype and newlines", text.Value(), xml);
  CHECK_TRUE("binary Read advances to the document end", file.Pos() == xml.size());
  CHECK_TRUE("binary EOF returns zero", file.Read(text) == 0);
  CHECK_TEXT("binary EOF clears the text", text.Value(), "");
  file.Seek(2);
  CHECK_TRUE("the file pointer matches its Seek offset", file.Pos() == 2);
  file.Close();
  static_cast<void>(agiru::File::Erase(name));
}

void SavedXmlDoctypeRemainsReadable() {
  const auto name = FileWithBytes("<?xml version=\"1.0\"?><!DOCTYPE rootNode><rootNode/>");
  agiru::dotnet::XmlDocument document;
  document = document.XmlDocument();
  agiru::dotnet::XmlReaderSettings settings;
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Parse());
  auto reader = agiru::dotnet::XmlReader::Create(name, settings);
  document.Load(reader);
  reader.Close();
  const agiru::dotnet::XmlDocumentType original = document.DocumentType();
  const auto replacement = document.CreateDocumentType(original.Name().Value(),
                                                       original.PublicId().Value(),
                                                       original.SystemId().Value(),
                                                       original.InternalSubset().Value());
  static_cast<void>(document.ReplaceChild(replacement, original));
  document.Save(name);
  agiru::File file;
  static_cast<void>(file.Open(name));
  agiru::Text<kXmlReadCapacity> text;
  CHECK_TRUE("a saved XML document is read completely", file.Read(text) == file.Len());
  CHECK_TRUE("a saved XML document retains its doctype",
             text.Value().find("<!DOCTYPE rootNode>") != std::string_view::npos);
  file.Close();
  static_cast<void>(agiru::File::Erase(name));
}

void TextModeReturnsTextLengthAndIgnoresCarriageReturns() {
  const auto name = FileWithBytes("a\rb\r\nc\n");
  agiru::File file;
  file.TextMode(true);
  static_cast<void>(file.Open(name));
  agiru::Text<0> text;
  CHECK_TRUE("text mode excludes carriage returns and LF from its count", file.Read(text) == 2);
  CHECK_TEXT("text mode ignores embedded carriage returns", text.Value(), "ab");
  CHECK_TRUE("text mode consumes the entire first line", file.Pos() == 5);
  CHECK_TRUE("text mode reads the following line", file.Read(text) == 1);
  CHECK_TEXT("the second line retains its text", text.Value(), "c");
  file.Close();
  static_cast<void>(agiru::File::Erase(name));
}

void BinaryTextUsesTheDeclaredCapacityAndConsumesItsTerminator() {
  const auto name = FileWithBytes(kTerminatedBytes);
  agiru::File file;
  file.TextMode(false);
  static_cast<void>(file.Open(name));
  agiru::Text<3> text;
  agiru::Integer read = -1;
  std::string errorMessage;
  try {
    read = file.Read(text);
  } catch (const agiru::Error &error) { errorMessage = error.what(); }
  CHECK_SILENT("a binary value with a valid terminator does not raise", errorMessage);
  CHECK_TRUE("binary capacity allows one trailing zero byte", read == 4);
  CHECK_TEXT("the zero terminator is not part of the text", text.Value(), "abc");
  CHECK_TRUE("binary capacity advances past its zero terminator", file.Pos() == 4);
  bool refused = false;
  try {
    static_cast<void>(file.Read(text));
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).find("invalid stream data") != std::string_view::npos;
  }
  CHECK_TRUE("a nonzero byte beyond declared capacity refuses", refused);
  CHECK_TRUE("the refused binary read retains its consumed position", file.Pos() == 8);
  file.Seek(0);
  agiru::Text<kTerminatedBytes.size()> wider;
  CHECK_TRUE("binary conversion consumes bytes beyond an embedded zero", file.Read(wider) == 8);
  CHECK_TEXT("binary conversion stops its text at the first zero", wider.Value(), "abc");
  file.Close();
  static_cast<void>(agiru::File::Erase(name));
}

void DeclaredCapacityAndClosedFileRefusalsRemainVisible() {
  const auto name = FileWithBytes("a");
  agiru::File file;
  static_cast<void>(file.Open(name));
  agiru::Text<kNativeDeclaredTextLimit> supported;
  CHECK_TRUE("the largest supported declared text can read a byte", file.Read(supported) == 1);
  agiru::Text<kNativeDeclaredTextLimit + 1> tooWide;
  bool refused = false;
  try {
    static_cast<void>(file.Read(tooWide));
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).find("declared text length") != std::string_view::npos;
  }
  CHECK_TRUE("an unsupported declared text length refuses even at EOF", refused);
  file.Close();
  refused = false;
  try {
    static_cast<void>(file.Read(supported));
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).find("not open") != std::string_view::npos;
  }
  CHECK_TRUE("reading a closed file refuses", refused);
  static_cast<void>(agiru::File::Erase(name));
}

} // namespace

int main() {
  return gate::Run("File", [] {
    ATempFileIsMadeOpenedAndWritten();
    AStreamOverAFileReadsIt();
    BinaryTextReadsDoNotSplitLines();
    SavedXmlDoctypeRemainsReadable();
    TextModeReturnsTextLengthAndIgnoresCarriageReturns();
    BinaryTextUsesTheDeclaredCapacityAndConsumesItsTerminator();
    DeclaredCapacityAndClosedFileRefusalsRemainVisible();
  });
}
