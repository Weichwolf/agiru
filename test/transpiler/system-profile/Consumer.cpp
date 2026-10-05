#include "meta/ModuleDef.h"
#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "fixture/codeunit/ProfileCaller.h"
#include "fixture/table/ProfileRow.h"

#include <cstddef>
#include <span>
#include <string_view>

namespace {

using Row = agiru::Fixture::ProfileRow_Table;

template <class T>
concept Audit = requires(T row) {
  row.SystemCreatedAt;
  row.SystemCreatedBy;
  row.SystemModifiedAt;
  row.SystemModifiedBy;
};
template <class T>
concept Lookups = requires(T row) {
  row.SystemCreatedByUserName;
  row.SystemCreatedByFullName;
  row.SystemModifiedByUserName;
  row.SystemModifiedByFullName;
};

static_assert(Audit<Row> == (AGIRU_PROFILE_AUDIT != 0));
static_assert(Lookups<Row> == (AGIRU_PROFILE_LOOKUP != 0));

void Declarations() {
  const auto &table = agiru::TableTraits<Row>::kTable;
  CHECK_TRUE("effective fields follow the selected host, not app version/minimum",
             table.fields.size() == AGIRU_PROFILE_FIELDS);
  CHECK_TRUE("source minimum runtime remains independent module evidence",
             table.module != nullptr && table.module->minimumRuntime == "12.0");
}

void Writes() {
  const gate::OwnedDatabase database("generated_profile");
  const agiru::Session session(database.Dsn());
  const auto &table = agiru::TableTraits<Row>::kTable;
  agiru::CreateTable(session.Database(), table);
  Row row;
  agiru::Fixture::ProfileCaller_Codeunit caller;
  CHECK_TRUE("generated AL writes return the actual second rowversion", caller.Write(row) == 2);
  CHECK_TRUE("generated AL alias and implicit timestamp share their returned value",
             row.Version == 2 && row.SystemRowVersion == 2);
  row.Rename(2);
  CHECK_TRUE("generated Rename returns the next stored alias",
             row.Version == 3 && row.SystemRowVersion == 3);
  row = Row{};
  CHECK_TRUE("generated SQL read resolves the canonical timestamp", row.Get(2));
  CHECK_TRUE("read alias and implicit value remain identical",
             row.Version == 3 && row.SystemRowVersion == 3);
  const auto stored =
      session.Database().Execute(R"sql(SELECT "timestamp" FROM "ProfileRow" WHERE "ID" = 2)sql");
  CHECK_TRUE("independent SQL observes the exact rowversion and renamed key",
             stored.Rows() == 1 && stored.Columns() == 1 && stored.Value(0, 0) == "3");
  const auto columns = session.Database().Execute(
      "SELECT count(*) FROM information_schema.columns WHERE table_name = 'ProfileRow' "
      "AND column_name IN ('Version', 'SystemRowVersion', 'SystemCreatedByUserName', "
      "'SystemCreatedByFullName', 'SystemModifiedByUserName', 'SystemModifiedByFullName')");
  CHECK_TRUE("aliases and user lookup FlowFields never become physical columns",
             columns.Rows() == 1 && columns.Value(0, 0) == "0");
  agiru::RecordRef reference;
  reference.GetTable(row);
  CHECK_TRUE("implicit fields never inflate the source-declared field index",
             reference.FieldCount() == 2);
  CHECK_TEXT(
      "timestamp reflection retains its original name", reference.Field(0).Name(), "timestamp");
}

}

int main(int argc, char **argv) {
  const std::span<char *> arguments(argv, static_cast<std::size_t>(argc));
  if (arguments.size() == 2 && std::string_view(arguments[1]) == "--declarations") {
    return gate::Run("GeneratedSystemProfileDeclarations", Declarations);
  }
  return gate::Run("GeneratedSystemProfile", [] {
    Declarations();
    Writes();
  });
}
