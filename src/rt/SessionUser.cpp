#include "SessionUser.h"

#include "meta/TableDef.h"
#include "platform/User.h"
#include "runtime/Session.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/Guid.h"

#include "Rows.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>

namespace agiru::detail {

platform::User RequireActiveUser(const Connection &connection, const Guid &identity) {
  if (identity.IsNull()) { throw SessionError("session user is not active"); }
  platform::User user;
  const auto &table = platform::kUserTable;
  const std::array<std::optional<std::string>, 1> key{identity.ToStorageText()};
  const auto row = GetRow(connection, table, key);
  if (!row) { throw SessionError("session user is not active"); }
  std::size_t column = 0;
  for (const auto &field : table.fields) {
    if (Stored(field)) { SetFieldText(&user, field, Required((*row)[column++], field)); }
  }
  if (user.UserName.Value().empty() || user.State != platform::UserState::Enabled ||
      (!user.ExpiryDate.IsUndefined() && user.ExpiryDate <= CurrentDateTime())) {
    throw SessionError("session user is not active");
  }
  return user;
}

}
