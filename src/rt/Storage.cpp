#include "runtime/Storage.h"

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"
#include "platform/AllProfile.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/Field.h"
#include "platform/PageMetadata.h"
#include "platform/TableMetadata.h"
#include "platform/TenantLicenseState.h"
#include "runtime/Catalogue.h"
#include "runtime/Codeunit.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"

#include "Rows.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <print>
#include <set>
#include <span>
#include <string>
#include <string_view>

namespace agiru {

namespace {

std::string Quoted(std::string_view identifier) {
  std::string out = "\"";
  for (const char c : identifier) {
    if (c == '"') { out += '"'; }
    out += c;
  }
  out += '"';
  return out;
}

std::string Placeholder(std::size_t oneBased) {
  return "$" + std::to_string(oneBased);
}

std::size_t IndexOf(const TableDef &table, FieldNo no) {
  for (std::size_t i = 0; i < table.fields.size(); ++i) {
    if (table.fields[i].no == no) { return i; }
  }
  throw Error("Storage: the table declares no such field");
}

std::string KeyPredicate(const TableDef &table, std::size_t firstPlaceholder) {
  if (table.keys.empty()) { throw Error("Storage: the table declares no key"); }
  std::string out;
  std::size_t n = firstPlaceholder;
  for (const FieldNo no : table.keys[0].fields) {
    if (!out.empty()) { out += " AND "; }
    out += Quoted(table.fields[IndexOf(table, no)].name) + " = " + Placeholder(n);
    ++n;
  }
  return out;
}

FieldValues RowOf(const Result &result, std::size_t row) {
  const std::size_t columns = result.Columns();
  FieldValues values;
  values.reserve(columns);
  for (std::size_t c = 0; c < columns; ++c) {
    const std::optional<std::string_view> v = result.Value(row, c);
    values.emplace_back(v.has_value() ? std::optional<std::string>(std::string(*v)) : std::nullopt);
  }
  return values;
}

}

std::string ColumnType(const FieldDef &def) {
  switch (def.type) {
    case FieldType::Boolean: return "boolean";
    case FieldType::Integer:
    case FieldType::Option:
    case FieldType::Enum: return "integer";
    case FieldType::BigInteger: return "bigint";
    case FieldType::Decimal: return "numeric(38,20)";
    case FieldType::Code:
    case FieldType::Text: return "varchar(" + std::to_string(def.length) + ")";
    case FieldType::Date: return "timestamp";
    case FieldType::Time: return "time";
    case FieldType::DateTime: return "timestamp";
    case FieldType::Guid: return "uuid";
    case FieldType::DateFormula: return "varchar(32)";
    case FieldType::RecordId:
    case FieldType::TableFilter: return "text";
    case FieldType::Duration: return "bigint";
    case FieldType::Blob: return "bytea";
    case FieldType::Media:
    case FieldType::MediaSet: return "uuid";
  }
  throw Error("ColumnType: no SQL type for this field type yet");
}

namespace {

std::string ColumnZero(const FieldDef &def) {
  switch (def.type) {
    case FieldType::Boolean: return "false";
    case FieldType::Option:
    case FieldType::Enum:
    case FieldType::Integer:
    case FieldType::BigInteger:
    case FieldType::Decimal:
    case FieldType::Duration: return "0";
    case FieldType::Date:
    case FieldType::DateTime: return "'1753-01-01 00:00:00'";
    case FieldType::Time: return "'00:00:00'";
    case FieldType::Guid:
    case FieldType::Media:
    case FieldType::MediaSet: return "'00000000-0000-0000-0000-000000000000'";
    case FieldType::Blob: return "''::bytea";
    default: return "''";
  }
}
}

void CreateTable(const Connection &connection, const TableDef &table) {
  std::string sql = "CREATE TABLE " + Quoted(table.name) + " (";
  bool written = false;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (written) { sql += ", "; }
    written = true;
    sql += Quoted(field.name) + " " + ColumnType(field) + " NOT NULL DEFAULT " + ColumnZero(field);
  }
  if (!table.keys.empty()) {
    sql += ", PRIMARY KEY (";
    bool first = true;
    for (const FieldNo no : table.keys[0].fields) {
      if (!first) { sql += ", "; }
      first = false;
      sql += Quoted(table.fields[IndexOf(table, no)].name);
    }
    sql += ")";
  }
  sql += ")";
  connection.Run(sql);

  for (std::size_t k = 1; k < table.keys.size(); ++k) {
    if (table.keys[k].fields.empty()) { continue; }
    if (!table.keys[k].enabled || !table.keys[k].maintainSqlIndex) { continue; }
    std::string index = "CREATE INDEX " +
                        Quoted(std::string(table.name) + "$" + std::string(table.keys[k].name)) +
                        " ON " + Quoted(table.name) + " (";
    bool first = true;
    for (const FieldNo no : table.keys[k].fields) {
      if (!first) { index += ", "; }
      first = false;
      index += Quoted(table.fields[IndexOf(table, no)].name);
    }
    index += ")";
    connection.Run(index);
  }
}

void DropTable(const Connection &connection, const TableDef &table) {
  connection.Run("DROP TABLE IF EXISTS " + Quoted(table.name));
}

namespace {

std::size_t StoredCount(const TableDef &table) {
  return static_cast<std::size_t>(std::ranges::count_if(table.fields, Stored));
}
}

namespace {

std::string StoredColumns(const TableDef &table) {
  std::string columns;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (!columns.empty()) { columns += ", "; }
    columns += Quoted(field.name);
  }
  return columns;
}
}

namespace {

std::size_t StoredIndexOf(const TableDef &table, FieldNo no) {
  std::size_t column = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (field.no == no) { return column; }
    ++column;
  }
  throw Error("the key names a field the schema does not store");
}
}

bool InsertRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> values) {
  if (values.size() != StoredCount(table)) {
    throw Error("Insert: the value count does not match the declaration");
  }
  const std::string columns = StoredColumns(table);
  std::string placeholders;
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i != 0) { placeholders += ", "; }
    placeholders += Placeholder(i + 1);
  }
  const Result written =
      connection.Execute("INSERT INTO " + Quoted(table.name) + " (" + columns + ") VALUES (" +
                             placeholders + ") ON CONFLICT DO NOTHING",
                         values);
  return written.Affected() == 1;
}

std::optional<FieldValues> GetRow(const Connection &connection,
                                  const TableDef &table,
                                  std::span<const std::optional<std::string>> key) {
  const std::string columns = StoredColumns(table);
  const Result result = connection.Execute("SELECT " + columns + " FROM " + Quoted(table.name) +
                                               " WHERE " + KeyPredicate(table, 1),
                                           key);
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

std::optional<FieldValues> GetRowWhere(const Connection &connection,
                                       const TableDef &table,
                                       const FieldDef &column,
                                       std::string_view value) {
  const std::string columns = StoredColumns(table);
  const std::optional<std::string> bound{std::string(value)};
  const Result result =
      connection.Execute("SELECT " + columns + " FROM " + Quoted(table.name) + " WHERE " +
                             Quoted(column.name) + " = " + Placeholder(1),
                         std::span<const std::optional<std::string>>(&bound, 1));
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

bool PlatformOwned(const FieldDef &field) {
  return field.no == SystemFieldNumbers::SystemId ||
         field.no == SystemFieldNumbers::SystemCreatedAt ||
         field.no == SystemFieldNumbers::SystemCreatedBy;
}

namespace {

std::string WrittenAssignments(const TableDef &table,
                               std::span<const std::optional<std::string>> values,
                               FieldValues &bound) {
  std::string assignments;
  std::size_t column = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field)) { continue; }
    const std::size_t index = column++;
    if (PlatformOwned(field)) { continue; }
    if (!bound.empty()) { assignments += ", "; }
    bound.push_back(values[index]);
    assignments += Quoted(field.name) + " = " + Placeholder(bound.size());
  }
  return assignments;
}

std::string OwnedColumns(const TableDef &table) {
  std::string columns;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field) || !PlatformOwned(field)) { continue; }
    if (!columns.empty()) { columns += ", "; }
    columns += Quoted(field.name);
  }
  return columns.empty() ? std::string("1") : columns;
}

std::optional<FieldValues> Updated(const Connection &connection,
                                   const TableDef &table,
                                   const std::string &assignments,
                                   std::size_t keyAt,
                                   const FieldValues &bound) {
  const Result result =
      connection.Execute("UPDATE " + Quoted(table.name) + " SET " + assignments + " WHERE " +
                             KeyPredicate(table, keyAt) + " RETURNING " + OwnedColumns(table),
                         bound);
  if (result.Rows() == 0) { return std::nullopt; }
  return RowOf(result, 0);
}

}

std::optional<FieldValues> ModifyRow(const Connection &connection,
                                     const TableDef &table,
                                     std::span<const std::optional<std::string>> values) {
  if (values.size() != StoredCount(table)) {
    throw Error("Modify: the value count does not match the declaration");
  }
  FieldValues bound;
  bound.reserve(values.size() + table.keys[0].fields.size());
  const std::string assignments = WrittenAssignments(table, values, bound);
  const std::size_t keyAt = bound.size() + 1;
  for (const FieldNo no : table.keys[0].fields) {
    bound.push_back(values[StoredIndexOf(table, no)]);
  }
  return Updated(connection, table, assignments, keyAt, bound);
}

std::optional<FieldValues> RenameRow(const Connection &connection,
                                     const TableDef &table,
                                     std::span<const std::optional<std::string>> values,
                                     std::span<const std::optional<std::string>> oldKey) {
  if (values.size() != StoredCount(table)) {
    throw Error("Rename: the value count does not match the declaration");
  }
  FieldValues bound;
  bound.reserve(values.size() + oldKey.size());
  const std::string assignments = WrittenAssignments(table, values, bound);
  const std::size_t keyAt = bound.size() + 1;
  bound.insert(bound.end(), oldKey.begin(), oldKey.end());
  return Updated(connection, table, assignments, keyAt, bound);
}

bool DeleteRow(const Connection &connection,
               const TableDef &table,
               std::span<const std::optional<std::string>> key) {
  const Result result = connection.Execute("DELETE FROM " + Quoted(table.name) + " WHERE " +
                                               KeyPredicate(table, 1) + " RETURNING 1",
                                           key);
  return result.Rows() != 0;
}

std::string_view Required(const std::optional<std::string> &value, const FieldDef &def) {
  if (!value.has_value()) {
    throw Error("the column for field '" + std::string(def.name) + "' is null");
  }
  return *value;
}

namespace {

std::string_view Fitted(std::string_view text, std::size_t length) {
  if (text.size() <= length) { return text; }
  std::size_t end = length;
  while (end > 0 && (static_cast<unsigned char>(text[end]) & 0xC0U) == 0x80U) { --end; }
  return text.substr(0, end);
}

}

namespace {

std::size_t AddMissingColumns(const Connection &into, const TableDef &table) {
  const std::array<std::optional<std::string>, 1> named{std::string(table.name)};
  const Result columns = into.Execute(
      "SELECT column_name FROM information_schema.columns WHERE table_schema = 'public' AND "
      "table_name = $1",
      named);
  std::set<std::string> there;
  for (std::size_t row = 0; row < columns.Rows(); ++row) {
    const std::optional<std::string_view> name = columns.Value(row, 0);
    if (name.has_value()) { there.emplace(*name); }
  }
  std::size_t added = 0;
  for (const FieldDef &field : table.fields) {
    if (!Stored(field) || there.contains(std::string(field.name))) { continue; }
    into.Run("ALTER TABLE " + Quoted(table.name) + " ADD COLUMN " + Quoted(field.name) + " " +
             ColumnType(field) + " NOT NULL DEFAULT " + ColumnZero(field));
    ++added;
  }
  return added;
}

}

void ProvisionInstalled(const Connection &into) {
  const Result standing =
      into.Execute("SELECT tablename FROM pg_tables WHERE schemaname = 'public'");
  std::set<std::string> there;
  for (std::size_t row = 0; row < standing.Rows(); ++row) {
    const std::optional<std::string_view> name = standing.Value(row, 0);
    if (name.has_value()) { there.emplace(*name); }
  }
  std::size_t made = 0;
  std::size_t widened = 0;
  for (const TableEntry *entry : InstalledTables()) {
    if (there.contains(std::string(entry->table->name))) {
      widened += AddMissingColumns(into, *entry->table);
      continue;
    }
    CreateTable(into, *entry->table);
    ++made;
  }
  if (made != 0) { std::println("{} table(s) created in the runner's database", made); }
  if (widened != 0) {
    std::println("{} column(s) added to platform tables the database already had", widened);
  }
  if (!Session::HasCurrent() || &Session::Current().Database() != &into) { return; }
  const std::string_view company = Session::Current().CompanyName();
  if (!company.empty()) {
    platform::Company standing;
    standing.SetRange(standing.Name, company);
    if (!standing.FindFirst()) {
      platform::Company row;
      row.Name = company;
      row.DisplayName = company;
      row.Insert();
      std::println("the company {} is written into Company", company);
    }
  }
  {
    platform::TenantLicenseState standing;
    if (standing.IsEmpty()) {
      platform::TenantLicenseState row;
      constexpr int kLicensedFromYear = 1980;
      constexpr int kLicensedToYear = 2079;
      constexpr unsigned kDecember = 12;
      constexpr unsigned kLastOfDecember = 31;
      row.StartDate = DateTime::Create(Date::FromYmd(kLicensedFromYear, 1, 1), Time{});
      row.EndDate =
          DateTime::Create(Date::FromYmd(kLicensedToYear, kDecember, kLastOfDecember), Time{});
      row.State = platform::TenantLicenseStateState::Paid;
      row.Insert();
      std::println("the tenant licence is written into Tenant License State as Paid");
    }
  }
  std::size_t profiles = 0;
  for (const ProfileDef *profile : InstalledProfiles()) {
    platform::AllProfile standing;
    standing.SetRange(standing.ProfileID, std::string_view(profile->profileId));
    if (standing.FindFirst()) { continue; }
    platform::AllProfile row;
    row.Scope = platform::PersonalizationScope::System;
    row.ProfileID = std::string_view(profile->profileId);
    row.Description = Fitted(profile->description, platform::AllProfile::kDescriptionLength);
    row.RoleCenterID = profile->roleCenter.Value();
    row.Caption = Fitted(profile->caption, platform::AllProfile::kCaptionLength);
    row.Enabled = profile->enabled;
    row.Promoted = profile->promoted;
    row.Insert();
    ++profiles;
  }
  if (profiles != 0) { std::println("{} profile(s) written into All Profile", profiles); }
  platform::Date anyPeriod;
  if (!anyPeriod.FindFirst()) {
    static constexpr int kFirstYear = 1980;
    static constexpr int kLastYear = 2079;
    static constexpr std::array<std::string_view, 7> kDays{
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    static constexpr std::array<std::string_view, 12> kMonths{"January",
                                                              "February",
                                                              "March",
                                                              "April",
                                                              "May",
                                                              "June",
                                                              "July",
                                                              "August",
                                                              "September",
                                                              "October",
                                                              "November",
                                                              "December"};
    std::size_t periods = 0;
    const auto period = [&periods](platform::PeriodType type,
                                   ::agiru::Date start,
                                   ::agiru::Date end,
                                   ::agiru::Integer no,
                                   std::string_view name) {
      platform::Date row;
      row.PeriodType_ = type;
      row.PeriodStart = start;
      row.PeriodEnd = end.Closing();
      row.PeriodNo = no;
      row.PeriodName = name;
      row.PeriodInvariantName = name;
      row.Insert();
      ++periods;
    };
    const std::int32_t first = calendar::DaysFromCivil(kFirstYear, 1, 1);
    const std::int32_t last = calendar::DaysFromCivil(kLastYear, 12, 31);
    for (std::int32_t days = first; days <= last; ++days) {
      const calendar::Civil civil = calendar::CivilFromDays(days);
      const ::agiru::Date day = ::agiru::Date::FromYmd(civil.year, civil.month, civil.day);
      period(platform::PeriodType::Date, day, day, day.DayOfWeek(), kDays[day.DayOfWeek() - 1]);
      if (day.DayOfWeek() == 1) {
        const calendar::Civil sunday = calendar::CivilFromDays(days + 6);
        period(platform::PeriodType::Week,
               day,
               ::agiru::Date::FromYmd(sunday.year, sunday.month, sunday.day),
               day.WeekNo(),
               "Week " + std::to_string(day.WeekNo()));
      }
      if (civil.day == 1) {
        const std::int32_t next =
            calendar::DaysFromCivil(civil.month == 12 ? civil.year + 1 : civil.year,
                                    civil.month == 12 ? 1 : civil.month + 1,
                                    1);
        const calendar::Civil monthEnd = calendar::CivilFromDays(next - 1);
        period(platform::PeriodType::Month,
               day,
               ::agiru::Date::FromYmd(monthEnd.year, monthEnd.month, monthEnd.day),
               static_cast<::agiru::Integer>(civil.month),
               kMonths[civil.month - 1]);
        if (civil.month % 3 == 1) {
          const unsigned quarter = (civil.month - 1) / 3 + 1;
          const std::int32_t after = quarter == 4
                                         ? calendar::DaysFromCivil(civil.year + 1, 1, 1)
                                         : calendar::DaysFromCivil(civil.year, quarter * 3 + 1, 1);
          const calendar::Civil quarterEnd = calendar::CivilFromDays(after - 1);
          period(platform::PeriodType::Quarter,
                 day,
                 ::agiru::Date::FromYmd(quarterEnd.year, quarterEnd.month, quarterEnd.day),
                 static_cast<::agiru::Integer>(quarter),
                 "Quarter " + std::to_string(quarter));
        }
        if (civil.month == 1) {
          period(platform::PeriodType::Year,
                 day,
                 ::agiru::Date::FromYmd(civil.year, 12, 31),
                 civil.year,
                 std::to_string(civil.year));
        }
      }
    }
    std::println("{} period(s) written into Date", periods);
  }
  platform::AllObj anyObject;
  if (anyObject.FindFirst()) { return; }
  std::size_t objects = 0;
  const auto object = [&objects](platform::AllObjType type,
                                 std::int32_t id,
                                 std::string_view name,
                                 std::string_view caption) {
    platform::AllObj bare;
    bare.ObjectType = type;
    bare.ObjectID = id;
    bare.ObjectName = name;
    bare.Insert();
    platform::AllObjWithCaption captioned;
    captioned.ObjectType = type;
    captioned.ObjectID = id;
    captioned.ObjectName = name;
    captioned.ObjectCaption = caption.empty() ? name : caption;
    captioned.Insert();
    ++objects;
  };
  for (const TableEntry *entry : InstalledTables()) {
    object(platform::AllObjType::Table,
           entry->table->id.Value(),
           entry->table->name,
           entry->table->caption);
  }
  for (const CodeunitEntry *entry : InstalledCodeunits()) {
    object(platform::AllObjType::Codeunit, entry->id.Value(), entry->name, entry->name);
  }
  for (const PageEntry *entry : InstalledPages()) {
    object(platform::AllObjType::Page,
           entry->page->id.Value(),
           entry->page->name,
           entry->page->caption);
  }
  if (objects != 0) { std::println("{} object(s) written into AllObj", objects); }
  platform::TableMetadata anyTable;
  if (anyTable.FindFirst()) { return; }
  std::size_t tables = 0;
  for (const TableEntry *entry : InstalledTables()) {
    platform::TableMetadata row;
    row.ID = entry->table->id.Value();
    row.Name = entry->table->name;
    row.Caption = entry->table->caption.empty() ? entry->table->name : entry->table->caption;
    row.ObsoleteState = platform::TableMetadataObsoleteState::No;
    row.TableType = [type = entry->table->tableType] {
      switch (type) {
        case TableType::Normal: return platform::TableMetadataTableType::Normal;
        case TableType::CRM: return platform::TableMetadataTableType::CRM;
        case TableType::CDS: return platform::TableMetadataTableType::CDS;
        case TableType::ExternalSQL: return platform::TableMetadataTableType::ExternalSQL;
        case TableType::Exchange: return platform::TableMetadataTableType::Exchange;
        case TableType::MicrosoftGraph: return platform::TableMetadataTableType::MicrosoftGraph;
        case TableType::Temporary: return platform::TableMetadataTableType::Temporary;
      }
      return platform::TableMetadataTableType::Normal;
    }();
    row.DataPerCompany = entry->table->dataPerCompany;
    row.LookupPageID = entry->table->lookupPageId.Value();
    row.DrillDownPageID = entry->table->drillDownPageId.Value();
    row.Insert();
    ++tables;
  }
  if (tables != 0) { std::println("{} table(s) written into Table Metadata", tables); }
  platform::Field anyField;
  if (!anyField.FindFirst()) {
    std::size_t fields = 0;
    for (const TableEntry *entry : InstalledTables()) {
      const std::span<const FieldNo> key = entry->table->keys.empty()
                                               ? std::span<const FieldNo>{}
                                               : entry->table->keys.front().fields;
      for (const FieldDef &field : entry->table->fields) {
        platform::Field row;
        row.TableNo = entry->table->id.Value();
        row.No = field.no.Value();
        row.TableName = Fitted(entry->table->name, platform::Field::kNameLength);
        row.FieldName = Fitted(field.name, platform::Field::kNameLength);
        row.Type = field.type;
        row.Len = static_cast<::agiru::Integer>(field.length);
        row.Class = field.fieldClass;
        row.RelationTableNo = ::agiru::detail::RelationTableNo(&field);
        row.RelationFieldNo = ::agiru::detail::RelationFieldNo(&field);
        row.ObsoleteState = platform::ObsoleteState::No;
        row.FieldCaption = Fitted(field.caption.empty() ? field.name : field.caption,
                                  platform::Field::kCaptionLength);
        row.Enabled = true;
        row.IsPartOfPrimaryKey = std::ranges::find(key, field.no) != key.end();
        row.Insert();
        ++fields;
      }
    }
    if (fields != 0) { std::println("{} field(s) written into Field", fields); }
  }
  platform::PageMetadata anyPage;
  if (anyPage.FindFirst()) { return; }
  std::size_t pages = 0;
  for (const PageEntry *entry : InstalledPages()) {
    platform::PageMetadata row;
    row.ID = entry->page->id.Value();
    row.Name = entry->page->name;
    row.Caption = entry->page->caption.empty() ? entry->page->name : entry->page->caption;
    row.PageType = [type = entry->page->type] {
      switch (type) {
        case PageType::Card: return platform::PageMetadataPageType::Card;
        case PageType::List: return platform::PageMetadataPageType::List;
        case PageType::RoleCenter: return platform::PageMetadataPageType::RoleCenter;
        case PageType::CardPart: return platform::PageMetadataPageType::CardPart;
        case PageType::ListPart: return platform::PageMetadataPageType::ListPart;
        case PageType::Document: return platform::PageMetadataPageType::Document;
        case PageType::Worksheet: return platform::PageMetadataPageType::Worksheet;
        case PageType::ListPlus: return platform::PageMetadataPageType::ListPlus;
        case PageType::ConfirmationDialog:
          return platform::PageMetadataPageType::ConfirmationDialog;
        case PageType::NavigatePage: return platform::PageMetadataPageType::NavigatePage;
        case PageType::StandardDialog: return platform::PageMetadataPageType::StandardDialog;
        case PageType::Api: return platform::PageMetadataPageType::Api;
        case PageType::ReportPreview: return platform::PageMetadataPageType::ReportPreview;
        case PageType::ReportProcessingOnly:
          return platform::PageMetadataPageType::ReportProcessingOnly;
        case PageType::HeadlinePart: return platform::PageMetadataPageType::HeadlinePart;
        case PageType::PromptDialog: return platform::PageMetadataPageType::PromptDialog;
        case PageType::UserControlHost: return platform::PageMetadataPageType::UserControlHost;
        case PageType::XmlPort:
        case PageType::ConfigurationDialog: return platform::PageMetadataPageType::Card;
      }
      return platform::PageMetadataPageType::Card;
    }();
    row.SourceTable = entry->page->source.Value();
    row.Editable = entry->page->editable.empty() || entry->page->editable != "false";
    row.InsertAllowed = entry->page->insertAllowed != "false";
    row.ModifyAllowed = entry->page->modifyAllowed != "false";
    row.DeleteAllowed = entry->page->deleteAllowed != "false";
    row.Insert();
    ++pages;
  }
  if (pages != 0) { std::println("{} page(s) written into Page Metadata", pages); }
}

}
