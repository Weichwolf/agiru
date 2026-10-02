#include "platform/User.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::User>::kTable.id == ::agiru::TableId{2000000120} && ::agiru::TableTraits<::agiru::platform::User>::kTable.name == "User", "native table identity mismatch: User");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::User>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 12;
}(), "native field count mismatch: User");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "User Security ID" || field->caption != "User Security ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User.User Security ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "User Name" || field->caption != "User Name" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: User.User Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Full Name" || field->caption != "Full Name" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: User.Full Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "State" || field->caption != "State" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Enabled" || field->values[0].caption != "Enabled") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Disabled" || field->values[1].caption != "Disabled") { return false; }
  return true;
}(), "native field declaration mismatch: User.State");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Expiry Date" || field->caption != "Expiry Date" || field->type != ::agiru::FieldType::DateTime || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User.Expiry Date");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Windows Security ID" || field->caption != "Windows Security ID" || field->type != ::agiru::FieldType::Text || field->length != 119) { return false; }
  return true;
}(), "native field declaration mismatch: User.Windows Security ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Change Password" || field->caption != "Change Password" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User.Change Password");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "License Type" || field->caption != "License Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 10) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Full User" || field->values[0].caption != "Full User") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Limited User" || field->values[1].caption != "Limited User") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Device Only User" || field->values[2].caption != "Device Only User") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Windows Group" || field->values[3].caption != "Windows Group") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "External User" || field->values[4].caption != "External User") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "External Administrator" || field->values[5].caption != "External Administrator") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "External Accountant" || field->values[6].caption != "External Accountant") { return false; }
  if (field->values[7].ordinal != 7 || field->values[7].name != "Application" || field->values[7].caption != "Application") { return false; }
  if (field->values[8].ordinal != 8 || field->values[8].name != "AAD Group" || field->values[8].caption != "AAD Group") { return false; }
  if (field->values[9].ordinal != 9 || field->values[9].name != "Agent" || field->values[9].caption != "Agent") { return false; }
  return true;
}(), "native field declaration mismatch: User.License Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "Authentication Email" || field->caption != "Authentication Email" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: User.Authentication Email");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "Contact Email" || field->caption != "Contact Email" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: User.Contact Email");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{15});
  if (field == nullptr || field->name != "Exchange Identifier" || field->caption != "Exchange Identifier" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: User.Exchange Identifier");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::User>::kTable, ::agiru::FieldNo{16});
  if (field == nullptr || field->name != "Application ID" || field->caption != "Application ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User.Application ID");
static_assert(::agiru::TableTraits<::agiru::platform::User>::kTable.keys.size() == 3, "native key count mismatch: User");
static_assert(::agiru::TableTraits<::agiru::platform::User>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1}, "native key declaration mismatch: User.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::User>::kTable.keys.size() > 1 && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[1].name == "Key2" && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[1].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[1].fields[0] == ::agiru::FieldNo{2}, "native key declaration mismatch: User.Key2");
static_assert(::agiru::TableTraits<::agiru::platform::User>::kTable.keys.size() > 2 && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[2].name == "Key3" && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[2].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::User>::kTable.keys[2].fields[0] == ::agiru::FieldNo{7}, "native key declaration mismatch: User.Key3");
static_assert(::agiru::TableTraits<::agiru::platform::User>::kTable.dataPerCompany == false, "native company scope mismatch: User");

