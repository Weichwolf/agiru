#include "runtime/TablePermissions.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include "Temporary.h"

#include <string>
#include <string_view>

namespace agiru {

bool HasTablePermission(const TableDef &table, TableOperation operation) {
  return !Session::HasCurrent() || Session::Current().AllowsTable(table, operation);
}

bool HasTableWritePermission(const TableDef &table) {
  return HasTablePermission(table, TableOperation::Insert) &&
         HasTablePermission(table, TableOperation::Modify) &&
         HasTablePermission(table, TableOperation::Delete);
}

void detail::RequireRecordPermission(const void *record,
                                     const TableDef &table,
                                     TableOperation operation) {
  if (TempOf(record) == nullptr) {
    RequireTablePermission(table, operation);
    if (operation != TableOperation::Read) { RequireWrite(); }
  }
}

void RequireTablePermission(const TableDef &table, TableOperation operation) {
  if (HasTablePermission(table, operation)) { return; }
  std::string_view name;
  switch (operation) {
    case TableOperation::Read: name = "Read"; break;
    case TableOperation::Insert: name = "Insert"; break;
    case TableOperation::Modify: name = "Modify"; break;
    case TableOperation::Delete: name = "Delete"; break;
  }
  throw Error("Sorry, the current permissions prevented the action. (TableData " +
                  std::to_string(table.id.Value()) + " " + std::string(table.name) + " " +
                  std::string(name) + ")",
              "Permission");
}

}
