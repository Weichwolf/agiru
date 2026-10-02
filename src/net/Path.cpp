#include "dotnet/Path.h"

#include "runtime/Error.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::string_view kSeparators = "\\/";
constexpr std::string_view kNameChars = "abcdefghijklmnopqrstuvwxyz0123456789";
constexpr std::size_t kRandomStem = 8;
constexpr std::size_t kRandomExtension = 3;

std::size_t LastSeparator(std::string_view path) {
  return path.find_last_of(kSeparators);
}

std::size_t ExtensionAt(std::string_view path) {
  const std::size_t dot = path.find_last_of('.');
  const std::size_t separator = LastSeparator(path);
  if (dot == std::string_view::npos) { return std::string_view::npos; }
  if (separator != std::string_view::npos && dot < separator) { return std::string_view::npos; }
  return dot;
}

std::string Random(std::size_t length) {
  static std::mt19937_64 engine{std::random_device{}()};
  std::uniform_int_distribution<std::size_t> pick(0, kNameChars.size() - 1);
  std::string out;
  for (std::size_t i = 0; i < length; ++i) { out += kNameChars[pick(engine)]; }
  return out;
}

}

std::string Path::GetFileName(std::string_view path) {
  const std::size_t at = LastSeparator(path);
  return std::string(at == std::string_view::npos ? path : path.substr(at + 1));
}

std::string Path::GetFileNameWithoutExtension(std::string_view path) {
  const std::string name = GetFileName(path);
  const std::size_t dot = name.find_last_of('.');
  return dot == std::string::npos ? name : name.substr(0, dot);
}

std::string Path::GetExtension(std::string_view path) {
  const std::size_t dot = ExtensionAt(path);
  if (dot == std::string_view::npos || dot + 1 == path.size()) { return {}; }
  return std::string(path.substr(dot));
}

bool Path::HasExtension(std::string_view path) {
  return !GetExtension(path).empty();
}

std::string Path::GetDirectoryName(std::string_view path) {
  const std::size_t at = LastSeparator(path);
  if (at == std::string_view::npos) { return {}; }
  std::string_view directory = path.substr(0, at);
  while (!directory.empty() && kSeparators.find(directory.back()) != std::string_view::npos) {
    directory.remove_suffix(1);
  }
  return std::string(directory);
}

std::string Path::ChangeExtension(std::string_view path, std::string_view extension) {
  const std::size_t dot = ExtensionAt(path);
  std::string out(dot == std::string_view::npos ? path : path.substr(0, dot));
  if (extension.empty()) { return out; }
  if (extension.front() != '.') { out += '.'; }
  out += extension;
  return out;
}

std::string Path::Combine(std::string_view first, std::string_view second) {
  if (first.empty()) { return std::string(second); }
  if (second.empty()) { return std::string(first); }
  const bool rooted = kSeparators.find(second.front()) != std::string_view::npos ||
                      (second.size() > 1 && second[1] == ':');
  if (rooted) { return std::string(second); }
  std::string out(first);
  if (kSeparators.find(out.back()) == std::string_view::npos) {
    out += out.find('\\') != std::string::npos ? '\\' : '/';
  }
  out += second;
  return out;
}

std::string Path::GetRandomFileName() {
  return Random(kRandomStem) + "." + Random(kRandomExtension);
}

std::string Path::GetTempFileName() {
  const std::filesystem::path directory = std::filesystem::temp_directory_path();
  for (int attempt = 0; attempt < 100; ++attempt) {
    const std::filesystem::path made = directory / ("tmp" + Random(kRandomStem) + ".tmp");
    if (std::filesystem::exists(made)) { continue; }
    std::ofstream file(made, std::ios::binary);
    if (file) { return made.string(); }
  }
  throw Error("Path.GetTempFileName: no file could be made under " + directory.string());
}

std::string Path::GetInvalidFileNameChars() {
  std::string out;
  for (char c = 1; c < ' '; ++c) { out += c; }
  out += "\"<>|:*?\\/";
  return out;
}

std::string Path::GetInvalidPathChars() {
  std::string out;
  for (char c = 1; c < ' '; ++c) { out += c; }
  out += '|';
  return out;
}

}
