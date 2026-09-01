#include "Format.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>

namespace agiru::gen {

namespace {

constexpr std::size_t kPipeBuffer = 4096;

std::string Capture(const std::string &command) {
  std::FILE *pipe = popen(command.c_str(), "r");
  if (pipe == nullptr) { throw std::runtime_error("gen: cannot run " + command); }
  std::string out;
  std::array<char, kPipeBuffer> buffer{};
  while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
    out += buffer.data();
  }
  if (pclose(pipe) != 0) { throw std::runtime_error("gen: failed to run " + command); }
  return out;
}

std::string Formatter() {
  for (const char *name : {"clang-format-19", "clang-format"}) {
    if (!Capture(std::string("command -v ") + name + " 2>/dev/null || true").empty()) {
      return name;
    }
  }
  throw std::runtime_error("gen: clang-format is missing -- see scripts/install.sh");
}

} // namespace

std::string Formatted(const FormatRequest &request) {
  const std::string path = "/tmp/agiru-format-in";
  {
    std::ofstream file(path, std::ios::binary);
    if (!file) { throw std::runtime_error("gen: cannot write " + path); }
    file << request.source;
  }
  return Capture(Formatter() + " -style=file:" + request.stylePath + " " + path);
}

} // namespace agiru::gen
