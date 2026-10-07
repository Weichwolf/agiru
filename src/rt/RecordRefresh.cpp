#include "runtime/RecordRefresh.h"

#include "meta/Ids.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "type/Integer.h"

#include "RecordChanges.h"

#include <optional>

namespace agiru {
namespace {

void Refresh(std::optional<TableId> table) {
  const auto &connection = Session::Current().Database();
  const auto isolation = connection.Execute("SHOW transaction_isolation");
  const auto value = isolation.Value(0, 0);
  if (!value.has_value() || (*value != "read committed" && *value != "read uncommitted")) {
    throw Error("SelectLatestVersion cannot refresh this transaction's SQL snapshot",
                "RecordRefreshIsolation");
  }
  detail::RefreshRecordReads(table);
}

}

void SelectLatestVersion() {
  Refresh(std::nullopt);
}

void SelectLatestVersion(Integer table) {
  Refresh(TableId{table});
}

}
