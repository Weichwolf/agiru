#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/File.h"
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
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace agiru {

namespace {

constexpr int kTemporaryNameAttemptLimit = 64;

std::vector<std::uint8_t> BytesOf(const std::filesystem::path &path) {
  std::ifstream file{path, std::ios::binary};
  if (!file) { throw Error("File.Open(" + path.string() + "): it cannot be read"); }
  const std::string held{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  return {held.begin(), held.end()};
}

void WriteBytes(const std::filesystem::path &path, const std::vector<std::uint8_t> &bytes) {
  std::ofstream file{path, std::ios::binary | std::ios::trunc};
  if (!file) { throw Error("File.Close(" + path.string() + "): it cannot be written"); }
  file.write(reinterpret_cast<const char *>(bytes.data()),
             static_cast<std::streamsize>(bytes.size()));
}

std::string TemporaryName() {
  static std::mt19937_64 turns{std::random_device{}()};
  const std::filesystem::path where = std::filesystem::temp_directory_path();
  for (int tries = 0; tries < kTemporaryNameAttemptLimit; ++tries) {
    const std::filesystem::path made = where / ("agiru_" + std::to_string(turns()) + ".tmp");
    if (!std::filesystem::exists(made)) { return made.string(); }
  }
  throw Error("File.CreateTempFile(): no unused name in " + where.string());
}

}

void File::Bind(std::string_view name, bool truncate) {
  name_ = std::string(name);
  position_ = 0;
  open_ = true;
  if (truncate) {
    held_.Set({});
    WriteBytes(std::filesystem::path(name_), {});
  } else {
    held_.Set(BytesOf(std::filesystem::path(name_)));
  }
}

Boolean File::Create(std::string_view Name, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  Bind(Name, true);
  writeMode_ = true;
  return true;
}

Boolean File::Open(std::string_view Name, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  if (!Exists(Name)) { return false; }
  Bind(Name, false);
  return true;
}

Boolean File::CreateTempFile() {
  return Create(TemporaryName());
}

Boolean File::CreateTempFile(const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  return CreateTempFile();
}

void File::Close() {
  if (!open_) { return; }
  if (writeMode_) { WriteBytes(std::filesystem::path(name_), held_.Bytes()); }
  open_ = false;
  position_ = 0;
}

std::string File::Name() {
  return name_;
}

Integer File::Len() {
  return static_cast<Integer>(held_.Bytes().size());
}

Integer File::Pos() const {
  return static_cast<Integer>(position_);
}

void File::Seek(Integer Position) {
  const std::size_t at = Position < 0 ? 0 : static_cast<std::size_t>(Position);
  position_ = at > held_.Bytes().size() ? held_.Bytes().size() : at;
}

void File::WriteLine(std::string_view text) {
  std::vector<std::uint8_t> bytes = held_.Bytes();
  if (position_ < bytes.size()) { bytes.resize(position_); }
  bytes.insert(bytes.end(), text.begin(), text.end());
  if (textMode_) {
    bytes.push_back(static_cast<std::uint8_t>('\r'));
    bytes.push_back(static_cast<std::uint8_t>('\n'));
  }
  position_ = bytes.size();
  held_.Set(std::move(bytes));
}

Integer File::Read(Variant &Read) {
  std::string text;
  const Integer read = ReadText(text, 0);
  Read = Variant{text};
  return read;
}

Integer File::ReadText(std::string &text, std::size_t maximum) {
  if (maximum > kMaximumDeclaredReadText) {
    throw Error("File.Read: the declared text length exceeds the supported stream text length");
  }
  if (!open_) { throw Error("File.Read: the file is not open"); }
  const std::vector<std::uint8_t> &bytes = held_.Bytes();
  text.clear();
  if (position_ >= bytes.size()) { return 0; }
  if (textMode_) {
    while (position_ < bytes.size()) {
      const auto byte = bytes[position_++];
      if (byte == 0 || byte == static_cast<std::uint8_t>('\n')) { break; }
      if (byte != static_cast<std::uint8_t>('\r')) { text += static_cast<char>(byte); }
    }
    return static_cast<Integer>(text.size());
  }
  const std::size_t available = bytes.size() - position_;
  const std::size_t requested = maximum == 0 ? available : maximum + 1;
  const std::size_t consumed = requested < available ? requested : available;
  const std::size_t start = position_;
  position_ += consumed;
  if (maximum != 0 && consumed > maximum && bytes[position_ - 1] != 0) {
    throw Error("File.Read: invalid stream data beyond the declared text capacity");
  }
  std::size_t end = start;
  while (end < position_ && bytes[end] != 0) { ++end; }
  text.assign(bytes.begin() + static_cast<std::ptrdiff_t>(start),
              bytes.begin() + static_cast<std::ptrdiff_t>(end));
  return static_cast<Integer>(consumed);
}

Boolean File::TextMode(Boolean Mode) {
  const Boolean was = textMode_;
  textMode_ = Mode;
  return was;
}

Boolean File::TextMode() const {
  return textMode_;
}

Boolean File::WriteMode(Boolean Mode) {
  const Boolean was = writeMode_;
  writeMode_ = Mode;
  return was;
}

Boolean File::WriteMode() const {
  return writeMode_;
}

void File::CreateInStream(InStream &InStream) {
  InStream = held_.CreateInStream();
}

void File::CreateInStream(InStream &InStream, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  CreateInStream(InStream);
}

void File::CreateOutStream(OutStream &OutStream) {
  OutStream = held_.CreateOutStream();
}

void File::CreateOutStream(OutStream &OutStream, const TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  CreateOutStream(OutStream);
}

Boolean File::Exists(std::string_view Name) {
  std::error_code failed;
  return std::filesystem::is_regular_file(std::filesystem::path(Name), failed) && !failed;
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
  const std::string where = std::filesystem::temp_directory_path().string();
  return std::string_view(Name).starts_with(where);
}

}
