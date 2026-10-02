#include "runtime/ConnectionInfo.h"

#include "runtime/Database.h"

#include <memory>
#include <string>
#include <string_view>

#include <libpq-fe.h>

namespace agiru {

namespace {

std::string Value(std::string_view text) {
  std::string result = "'";
  for (const char character : text) {
    if (character == '\'' || character == '\\') { result += '\\'; }
    result += character;
  }
  result += '\'';
  return result;
}

void CheckDatabase(std::string_view database) {
  if (database.empty() || database.find('\0') != std::string_view::npos) {
    throw DatabaseError("The connection must explicitly name a nonempty database without NUL");
  }
}

}

ConnectionInfo::ConnectionInfo(std::string_view connection) {
  if (connection.find('\0') != std::string_view::npos) {
    throw DatabaseError("A PostgreSQL connection string cannot contain NUL");
  }
  char *error = nullptr;
  const std::unique_ptr<PQconninfoOption, decltype(&PQconninfoFree)> parsed(
      PQconninfoParse(std::string(connection).c_str(), &error), &PQconninfoFree);
  const bool syntaxError = error != nullptr;
  if (error != nullptr) { PQfreemem(error); }
  if (parsed == nullptr) {
    throw DatabaseError(syntaxError ? "Invalid PostgreSQL connection string"
                                    : "Unable to allocate PostgreSQL connection options");
  }
  for (const PQconninfoOption *option = parsed.get(); option->keyword != nullptr; ++option) {
    if (option->val == nullptr) { continue; }
    options_.emplace_back(option->keyword, option->val);
    if (std::string_view(option->keyword) == "dbname") { database_ = option->val; }
  }
  CheckDatabase(database_);
}

std::string ConnectionInfo::AtDatabase(std::string_view database) const {
  CheckDatabase(database);
  std::string result;
  for (const auto &[keyword, value] : options_) {
    if (!result.empty()) { result += ' '; }
    result += keyword + "=" + Value(keyword == "dbname" ? database : std::string_view(value));
  }
  return result;
}

}
