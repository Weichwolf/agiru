#include "type/IsolatedStorage.h"

#include "runtime/Database.h"
#include "runtime/Session.h"
#include "type/Boolean.h"
#include "type/DataScope.h"
#include "type/SecretText.h"
#include "type/StringValue.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

using Binds = std::vector<std::optional<std::string>>;

constexpr std::string_view kTable = "\"platform$IsolatedStorage\"";

void Declared() {
  Session::Current().Database().Run(
      "CREATE TABLE IF NOT EXISTS " + std::string(kTable) +
      " (scope integer NOT NULL, company text NOT NULL, \"user\" text NOT NULL, "
      "key text NOT NULL, value text NOT NULL, secret boolean NOT NULL DEFAULT false, "
      "PRIMARY KEY (scope, company, \"user\", key))");
}

bool PerCompany(const DataScope &scope) {
  return scope == DataScope::Company || scope == DataScope::CompanyAndUser;
}

bool PerUser(const DataScope &scope) {
  return scope == DataScope::User || scope == DataScope::CompanyAndUser;
}

Binds Where(std::string_view key, const DataScope &scope) {
  return Binds{std::to_string(static_cast<std::int32_t>(scope)),
               PerCompany(scope) ? std::string(Session::Current().CompanyName()) : std::string{},
               PerUser(scope) ? std::string(Session::Current().UserId()) : std::string{},
               std::string(key)};
}

}

Boolean
IsolatedStorage::Set(const TextArgument &Key, std::string_view Value, const DataScope &DataScope) {
  Declared();
  Binds bound = Where(std::string_view(Key), DataScope);
  bound.emplace_back(std::string(Value));
  Session::Current().Database().Run(
      "INSERT INTO " + std::string(kTable) +
          " (scope, company, \"user\", key, value) VALUES ($1, $2, $3, $4, $5) "
          "ON CONFLICT (scope, company, \"user\", key) DO UPDATE SET value = EXCLUDED.value",
      bound);
  return true;
}

Boolean IsolatedStorage::SetEncrypted(const TextArgument &Key,
                                      const SecretText &Value,
                                      const DataScope &DataScope) {
  return SetEncrypted(Key, TextArgument(Value.Unwrap()), DataScope);
}

Boolean IsolatedStorage::SetEncrypted(const TextArgument &Key,
                                      const TextArgument &Value,
                                      const DataScope &DataScope) {
  Declared();
  Binds bound = Where(std::string_view(Key), DataScope);
  bound.emplace_back(std::string(std::string_view(Value)));
  Session::Current().Database().Run(
      "INSERT INTO " + std::string(kTable) +
          " (scope, company, \"user\", key, value, secret) VALUES ($1, $2, $3, $4, $5, true) "
          "ON CONFLICT (scope, company, \"user\", key) DO UPDATE SET value = EXCLUDED.value, "
          "secret = true",
      bound);
  return true;
}

Boolean IsolatedStorage::Get(const TextArgument &Key, const DataScope &DataScope, Text<0> &Value) {
  Declared();
  const Result found = Session::Current().Database().Execute(
      "SELECT value FROM " + std::string(kTable) +
          " WHERE scope = $1 AND company = $2 AND \"user\" = $3 AND key = $4",
      Where(std::string_view(Key), DataScope));
  if (found.Rows() == 0) { return false; }
  const std::optional<std::string_view> held = found.Value(0, 0);
  Value = held.value_or(std::string_view{});
  return true;
}

Boolean
IsolatedStorage::Get(const TextArgument &Key, const DataScope &DataScope, SecretText &Value) {
  Text<0> read;
  if (!Get(Key, DataScope, read)) { return false; }
  Value = SecretText(std::string(std::string_view(read)));
  return true;
}

Boolean IsolatedStorage::Get(const TextArgument &Key, Text<0> &Value) {
  return Get(Key, DataScope{}, Value);
}

Boolean IsolatedStorage::Get(const TextArgument &Key, SecretText &Value) {
  return Get(Key, DataScope{}, Value);
}

Boolean IsolatedStorage::Contains(const TextArgument &Key, const DataScope &DataScope) {
  Text<0> read;
  return Get(Key, DataScope, read);
}

Boolean
IsolatedStorage::Contains(const TextArgument &Key, const DataScope &DataScope, Boolean &isSecret) {
  Declared();
  const Result found = Session::Current().Database().Execute(
      "SELECT secret FROM " + std::string(kTable) +
          " WHERE scope = $1 AND company = $2 AND \"user\" = $3 AND key = $4",
      Where(std::string_view(Key), DataScope));
  if (found.Rows() == 0) {
    isSecret = false;
    return false;
  }
  const std::optional<std::string_view> held = found.Value(0, 0);
  isSecret = held.has_value() && *held == "t";
  return true;
}

Boolean IsolatedStorage::Delete(const TextArgument &Key, const DataScope &DataScope) {
  if (!Contains(Key, DataScope)) { return false; }
  Session::Current().Database().Run(
      "DELETE FROM " + std::string(kTable) +
          " WHERE scope = $1 AND company = $2 AND \"user\" = $3 AND key = $4",
      Where(std::string_view(Key), DataScope));
  return true;
}

}
