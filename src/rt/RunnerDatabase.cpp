#include "runtime/test/RunnerDatabase.h"

#include "runtime/Database.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {

namespace {

constexpr std::string_view kMaintenance = "postgres";

constexpr std::string_view kQuiet = "SET client_min_messages = warning";

bool IsUri(std::string_view dsn) {
  return dsn.starts_with("postgresql://") || dsn.starts_with("postgres://");
}

std::string Quoted(std::string_view name) {
  std::string quoted = "\"";
  for (const char c : name) {
    if (c == '"') {
      quoted += "\"\"";
    } else {
      quoted += c;
    }
  }
  quoted += '"';
  return quoted;
}

[[noreturn]] void Refuse(std::string_view dsn) {
  throw DatabaseError("the connection string " + std::string(dsn) + " names no database");
}

struct Split {
  std::size_t at;
  std::size_t length;
};

Split UriDatabase(std::string_view dsn) {
  const std::size_t authority = dsn.find("//") + 2;
  const std::size_t slash = dsn.find('/', authority);
  if (slash == std::string_view::npos) { Refuse(dsn); }
  const std::size_t end = dsn.find_first_of("?#", slash);
  return {.at = slash + 1,
          .length = (end == std::string_view::npos ? dsn.size() : end) - slash - 1};
}

Split KeywordDatabase(std::string_view dsn) {
  std::size_t at = 0;
  while (at < dsn.size()) {
    const std::size_t start = dsn.find_first_not_of(" \t", at);
    if (start == std::string_view::npos) { break; }
    std::size_t stop = start;
    bool quoted = false;
    while (stop < dsn.size() && (quoted || (dsn[stop] != ' ' && dsn[stop] != '\t'))) {
      if (dsn[stop] == '\'') { quoted = !quoted; }
      ++stop;
    }
    if (dsn.substr(start, stop - start).starts_with("dbname=")) {
      const std::size_t value = start + std::string_view("dbname=").size();
      return {.at = value, .length = stop - value};
    }
    at = stop;
  }
  Refuse(dsn);
}

Split DatabaseIn(std::string_view dsn) {
  return IsUri(dsn) ? UriDatabase(dsn) : KeywordDatabase(dsn);
}

}

std::string PointedAt(std::string_view dsn, DatabaseName database) {
  const Split split = DatabaseIn(dsn);
  std::string pointed(dsn.substr(0, split.at));
  pointed += database.value;
  pointed += dsn.substr(split.at + split.length);
  return pointed;
}

RunnerDatabase::RunnerDatabase(const std::string &master, std::string_view name, bool fresh)
    : maintenance_(PointedAt(master, DatabaseName{kMaintenance})),
      name_(name),
      dsn_(PointedAt(master, DatabaseName{name})) {
  const Split split = DatabaseIn(master);
  const std::string_view templateName = std::string_view(master).substr(split.at, split.length);
  if (templateName.empty()) { Refuse(master); }
  const Connection maintenance(maintenance_);
  maintenance.Run(kQuiet);
  if (fresh) { maintenance.Run("DROP DATABASE IF EXISTS " + Quoted(name_)); }
  const std::array<std::optional<std::string>, 1> bind{std::string(name_)};
  if (maintenance.Execute("SELECT 1 FROM pg_database WHERE datname = $1", bind).Rows() == 1) {
    return;
  }
  maintenance.Run("CREATE DATABASE " + Quoted(name_) + " TEMPLATE " + Quoted(templateName));
  cloned_ = true;
}

void RunnerDatabase::Drop() const {
  const Connection maintenance(maintenance_);
  maintenance.Run(kQuiet);
  maintenance.Run("DROP DATABASE IF EXISTS " + Quoted(name_));
}

}
