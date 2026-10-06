#pragma once

#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <unistd.h>

namespace gate {

inline void PrivateAuthFile(const std::string &path, const std::string &secret) {
  const std::string body = std::string(R"({"authorization":"Bearer )") + secret + "\"}\n";
  const int file = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
  if (file < 0) { throw std::runtime_error("cannot exclusively create private fixture auth file"); }
  const auto written = write(file, body.data(), body.size());
  const int closed = close(file);
  if (written != static_cast<ssize_t>(body.size()) || closed != 0) {
    throw std::runtime_error("cannot write private fixture auth file");
  }
}

}
