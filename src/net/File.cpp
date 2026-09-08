#include "type/File.h"

#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/TextEncoding.h"
#include "type/Variant.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace agiru {

namespace {

std::vector<std::uint8_t> Slurp(const std::string &name) {
  std::ifstream in(name, std::ios::binary);
  if (!in) { throw Error("the file " + name + " cannot be opened for reading"); }
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void Spill(const std::string &name, const std::vector<std::uint8_t> &bytes) {
  std::ofstream out(name, std::ios::binary | std::ios::trunc);
  if (!out) { throw Error("the file " + name + " cannot be opened for writing"); }
  out.write(reinterpret_cast<const char *>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
  if (!out) { throw Error("the file " + name + " could not be written"); }
}

std::filesystem::path TempDirectory() {
  std::error_code failed;
  const std::filesystem::path where = std::filesystem::temp_directory_path(failed);
  if (failed) { throw Error("this host has no temporary directory: " + failed.message()); }
  return where;
}

}

void File::Bind(std::string_view name, bool truncate) {
  name_ = std::string(name);
  position_ = 0;
  open_ = true;
  if (truncate) {
    held_.Set({});
    Spill(name_, {});
    return;
  }
  held_.Set(Slurp(name_));
}

Boolean File::Open(std::string_view Name, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  Bind(Name, false);
  return true;
}

Boolean File::Create(std::string_view Name, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  Bind(Name, true);
  return true;
}

Boolean File::CreateTempFile() {
  return CreateTempFile(TextEncoding{});
}

Boolean File::CreateTempFile(const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  std::filesystem::path where = TempDirectory();
  static thread_local std::uint64_t counted = 0;
  ++counted;
  where /= "agiru-" + std::to_string(counted) + ".tmp";
  Bind(where.string(), true);
  return true;
}

void File::Close() {
  if (!open_) { throw Error("this File variable has nothing open"); }
  if (writeMode_) { Spill(name_, held_.Bytes()); }
  open_ = false;
}

std::string File::Name() {
  return name_;
}

Integer File::Len() {
  if (!open_) { throw Error("this File variable has nothing open"); }
  return static_cast<Integer>(held_.Length());
}

Integer File::Pos() const {
  if (!open_) { throw Error("this File variable has nothing open"); }
  return static_cast<Integer>(position_) + 1;
}

void File::Seek(Integer Position) {
  if (!open_) { throw Error("this File variable has nothing open"); }
  if (Position < 0 || static_cast<std::size_t>(Position) > held_.Length()) {
    throw Error("the position " + std::to_string(Position) + " is outside the file, which holds " +
                std::to_string(held_.Length()) + " byte(s)");
  }
  position_ = static_cast<std::size_t>(Position);
}

Boolean File::TextMode() const {
  return textMode_;
}

Boolean File::TextMode(Boolean Mode) {
  const Boolean was = textMode_;
  textMode_ = Mode;
  return was;
}

Boolean File::WriteMode() const {
  return writeMode_;
}

Boolean File::WriteMode(Boolean Mode) {
  const Boolean was = writeMode_;
  writeMode_ = Mode;
  return was;
}

void File::CreateInStream(InStream &InStream) {
  if (!open_) { throw Error("this File variable has nothing open"); }
  held_.CreateInStream(InStream, TextEncoding{});
}

void File::CreateInStream(InStream &InStream, const TextEncoding &Encoding) {
  if (!open_) { throw Error("this File variable has nothing open"); }
  held_.CreateInStream(InStream, Encoding);
}

void File::CreateOutStream(OutStream &OutStream) {
  if (!open_) { throw Error("this File variable has nothing open"); }
  OutStream = held_.CreateOutStream();
}

void File::CreateOutStream(OutStream &OutStream, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  CreateOutStream(OutStream);
}

void File::WriteLine(std::string_view Value) {
  if (!open_) { throw Error("this File variable has nothing open"); }
  std::vector<std::uint8_t> bytes = held_.Bytes();
  for (const char c : Value) { bytes.push_back(static_cast<std::uint8_t>(c)); }
  if (textMode_) {
    bytes.push_back(static_cast<std::uint8_t>('\r'));
    bytes.push_back(static_cast<std::uint8_t>('\n'));
  }
  held_.Set(std::move(bytes));
  position_ = held_.Length();
}

std::string File::ToText() const {
  throw Error("a File is not a value: AL puts one in an `Any` and nothing renders it");
}

void File::RefuseTyped() {
  throw Error("a typed File.Write puts the platform's own binary layout into the file, and this "
              "runtime does not have it. Only the text form is here");
}

Integer File::Read(Variant &Read) {
  if (!open_) { throw Error("this File variable has nothing open"); }
  if (!textMode_) {
    throw Error("File.Read in binary mode wants the platform's own layout, which this runtime does "
                "not have. Text mode reads a line");
  }
  const std::vector<std::uint8_t> &bytes = held_.Bytes();
  std::string line;
  std::size_t at = position_;
  while (at < bytes.size() && bytes[at] != static_cast<std::uint8_t>('\n')) {
    if (bytes[at] != static_cast<std::uint8_t>('\r')) {
      line.push_back(static_cast<char>(bytes[at]));
    }
    ++at;
  }
  const std::size_t read = at - position_;
  position_ = at < bytes.size() ? at + 1 : at;
  Read = line;
  return static_cast<Integer>(read);
}

Boolean File::Exists(std::string_view Name) {
  std::error_code failed;
  return std::filesystem::exists(std::filesystem::path(Name), failed) && !failed;
}

Boolean File::Erase(std::string_view Name) {
  std::error_code failed;
  return std::filesystem::remove(std::filesystem::path(Name), failed) && !failed;
}

Boolean File::Copy(std::string_view FromName, std::string_view ToName) {
  std::error_code failed;
  std::filesystem::copy_file(std::filesystem::path(FromName),
                             std::filesystem::path(ToName),
                             std::filesystem::copy_options::overwrite_existing,
                             failed);
  return !failed;
}

Boolean File::Rename(std::string_view OldName, std::string_view NewName) {
  std::error_code failed;
  std::filesystem::rename(std::filesystem::path(OldName), std::filesystem::path(NewName), failed);
  return !failed;
}

Boolean File::IsPathTemporary(std::string_view Name) {
  std::error_code failed;
  const std::filesystem::path where = std::filesystem::temp_directory_path(failed);
  if (failed) { return false; }
  return std::string_view(Name).starts_with(where.string());
}

}
