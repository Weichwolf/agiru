#include "runtime/test/RunnerDatabase.h"

#include "runtime/ConnectionInfo.h"
#include "runtime/Database.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {

namespace {

constexpr std::string_view kMaintenance = "postgres";
constexpr std::string_view kRunnerMarker = "agiru.runner.v1.from.";
constexpr std::string_view kQuiet = "SET client_min_messages = warning";

std::string Quoted(std::string_view name) {
  std::string quoted = "\"";
  for (const char character : name) {
    if (character == '"') { quoted += '"'; }
    quoted += character;
  }
  quoted += '"';
  return quoted;
}

std::string RequiredColumn(const Result &result, std::size_t column) {
  const auto value = result.Value(0, column);
  if (!value) { throw DatabaseError("The database identity query returned an unexpected null"); }
  return std::string(*value);
}

class DatabaseIdentity {
public:
  explicit DatabaseIdentity(const Result &result)
      : oid_(RequiredColumn(result, 0)),
        template_(RequiredColumn(result, 1) == "t"),
        owned_(RequiredColumn(result, 2) == "t"),
        marker_(RequiredColumn(result, 3)) {}

  [[nodiscard]] std::string_view Oid() const { return oid_; }

  [[nodiscard]] bool MayReplace(std::string_view marker) const {
    return !template_ && owned_ && marker_ == marker;
  }

private:
  std::string oid_;
  bool template_;
  bool owned_;
  std::string marker_;
};

std::optional<DatabaseIdentity> FindDatabase(const Connection &connection, std::string_view name) {
  const std::array<std::optional<std::string>, 1> bind{std::string(name)};
  const Result result = connection.Execute(
      "SELECT oid::text, datistemplate, datdba = current_user::pg_catalog.regrole, "
      "COALESCE(pg_catalog.shobj_description(oid, 'pg_database'), '') "
      "FROM pg_catalog.pg_database WHERE datname = $1",
      bind);
  if (result.Rows() == 0) { return std::nullopt; }
  return DatabaseIdentity(result);
}

void LockScratch(const Connection &connection, std::string_view name) {
  const std::array<std::optional<std::string>, 1> bind{"agiru.runner:" + std::string(name)};
  const Result result = connection.Execute(
      "SELECT pg_catalog.pg_advisory_lock(pg_catalog.hashtextextended($1, 0))", bind);
  if (result.Rows() != 1) { throw DatabaseError("The scratch database lock was not acquired"); }
}

void CheckName(const Connection &connection, std::string_view name) {
  const std::array<std::optional<std::string>, 1> bind{std::string(name)};
  const Result result =
      connection.Execute("SELECT pg_catalog.octet_length($1) <= "
                         "pg_catalog.current_setting('max_identifier_length')::integer",
                         bind);
  if (result.Value(0, 0) != "t") {
    throw DatabaseError("The scratch database name exceeds the server identifier limit");
  }
}

void CheckOwned(const DatabaseIdentity &database, std::string_view marker) {
  if (!database.MayReplace(marker)) {
    throw DatabaseError("The scratch database is a template, belongs to another role, "
                        "or lacks matching runner ownership");
  }
}

}

std::string PointedAt(std::string_view dsn, DatabaseName database) {
  return ConnectionInfo(dsn).AtDatabase(database.value);
}

RunnerDatabase::RunnerDatabase(const std::string &master, std::string_view name, bool fresh)
    : maintenance_(PointedAt(master, DatabaseName{kMaintenance})),
      name_(name),
      dsn_(PointedAt(master, DatabaseName{name})) {
  const ConnectionInfo source(master);
  if (name_ == source.Database() || name_ == kMaintenance) {
    throw DatabaseError(
        "The scratch database must differ from its source and maintenance database");
  }
  const Connection maintenance(maintenance_);
  maintenance.Run(kQuiet);
  CheckName(maintenance, name_);
  LockScratch(maintenance, name_);
  const auto templateDatabase = FindDatabase(maintenance, source.Database());
  if (!templateDatabase) { throw DatabaseError("The source database does not exist"); }
  marker_ = std::string(kRunnerMarker) + std::string(templateDatabase->Oid());
  auto existing = FindDatabase(maintenance, name_);
  if (existing) {
    CheckOwned(*existing, marker_);
    if (!fresh) {
      identity_ = existing->Oid();
      return;
    }
    maintenance.Run("DROP DATABASE " + Quoted(name_));
  }
  maintenance.Run("CREATE DATABASE " + Quoted(name_) + " TEMPLATE " + Quoted(source.Database()));
  maintenance.Run("COMMENT ON DATABASE " + Quoted(name_) + " IS '" + marker_ + "'");
  existing = FindDatabase(maintenance, name_);
  if (!existing) { throw DatabaseError("The newly cloned scratch database is absent"); }
  CheckOwned(*existing, marker_);
  identity_ = existing->Oid();
  cloned_ = true;
}

void RunnerDatabase::Drop() const {
  const Connection maintenance(maintenance_);
  maintenance.Run(kQuiet);
  LockScratch(maintenance, name_);
  const auto existing = FindDatabase(maintenance, name_);
  if (!existing) { return; }
  CheckOwned(*existing, marker_);
  if (existing->Oid() != identity_) {
    throw DatabaseError("The scratch database was replaced since this runner acquired it");
  }
  maintenance.Run("DROP DATABASE " + Quoted(name_));
}

}
