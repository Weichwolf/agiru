#include "dotnet/File.h"

#include "runtime/Error.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

namespace agiru::dotnet {

Boolean File::Exists(std::string_view path) {
  std::error_code failed;
  return std::filesystem::is_regular_file(std::filesystem::path(path), failed) && !failed;
}

void File::Delete(std::string_view path) {
  std::error_code failed;
  static_cast<void>(std::filesystem::remove(std::filesystem::path(path), failed));
}

void File::Copy(std::string_view from, std::string_view to) {
  std::error_code failed;
  std::filesystem::copy_file(std::filesystem::path(from),
                             std::filesystem::path(to),
                             std::filesystem::copy_options::overwrite_existing,
                             failed);
  if (failed) {
    throw Error("File.Copy(" + std::string(from) + ", " + std::string(to) +
                "): " + failed.message());
  }
}

void File::Copy(std::string_view from, std::string_view to, Boolean overwrite) {
  if (!overwrite && Exists(to)) {
    throw Error("File.Copy(" + std::string(from) + ", " + std::string(to) +
                "): the target is already there");
  }
  Copy(from, to);
}

void File::Move(std::string_view from, std::string_view to) {
  std::error_code failed;
  std::filesystem::rename(std::filesystem::path(from), std::filesystem::path(to), failed);
  if (failed) {
    throw Error("File.Move(" + std::string(from) + ", " + std::string(to) +
                "): " + failed.message());
  }
}

::agiru::Text<0> File::ReadAllText(std::string_view path) {
  std::ifstream file{std::string(path), std::ios::binary};
  if (!file) { throw Error("File.ReadAllText(" + std::string(path) + "): it cannot be read"); }
  return ::agiru::Text<0>{
      std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()}};
}

::agiru::dotnet::Array File::ReadAllLines(std::string_view path) {
  std::ifstream file{std::string(path), std::ios::binary};
  if (!file) { throw Error("File.ReadAllLines(" + std::string(path) + "): it cannot be read"); }
  ::agiru::dotnet::Array lines;
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') { line.pop_back(); }
    lines.Add(::agiru::Variant{line});
  }
  return lines;
}

void File::AppendAllText(std::string_view path, std::string_view text) {
  std::ofstream file{std::string(path), std::ios::binary | std::ios::app};
  if (!file) { throw Error("File.AppendAllText(" + std::string(path) + "): it cannot be written"); }
  file.write(text.data(), static_cast<std::streamsize>(text.size()));
}

void File::WriteAllText(std::string_view path, std::string_view text) {
  std::ofstream file{std::string(path), std::ios::binary | std::ios::trunc};
  if (!file) { throw Error("File.WriteAllText(" + std::string(path) + "): it cannot be written"); }
  file.write(text.data(), static_cast<std::streamsize>(text.size()));
}

}
