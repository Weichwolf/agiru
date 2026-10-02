#include "platform/UserPersonalization.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.id == ::agiru::TableId{2000000073} && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.name == "User Personalization", "native table identity mismatch: User Personalization");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 19;
}(), "native field count mismatch: User Personalization");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "User SID" || field->caption != "User SID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.User SID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "User ID" || field->caption != "User ID" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.User ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Full Name" || field->caption != "Full Name" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Full Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Profile ID" || field->caption != "Profile ID" || field->type != ::agiru::FieldType::Code || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Profile ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "App ID" || field->caption != "App ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.App ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "Scope" || field->caption != "Scope" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "System" || field->values[0].caption != "System") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Tenant" || field->values[1].caption != "Tenant") { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Scope");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "Language ID" || field->caption != "Language ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Language ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "Language Name" || field->caption != "Language" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Language Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{15});
  if (field == nullptr || field->name != "Company" || field->caption != "Company" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Company");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{18});
  if (field == nullptr || field->name != "Debugger Break On Error" || field->caption != "Debugger Break On Error" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Debugger Break On Error");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{21});
  if (field == nullptr || field->name != "Debugger Break On Rec Changes" || field->caption != "Debugger Break On Rec Changes" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Debugger Break On Rec Changes");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{24});
  if (field == nullptr || field->name != "Debugger Skip System Triggers" || field->caption != "Debugger Skip System Triggers" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Debugger Skip System Triggers");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{27});
  if (field == nullptr || field->name != "Locale ID" || field->caption != "Locale ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Locale ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{28});
  if (field == nullptr || field->name != "Region" || field->caption != "Region" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Region");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{30});
  if (field == nullptr || field->name != "Time Zone" || field->caption != "Time Zone" || field->type != ::agiru::FieldType::Text || field->length != 180) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Time Zone");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{31});
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
}(), "native field declaration mismatch: User Personalization.License Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{32});
  if (field == nullptr || field->name != "Customization Status" || field->caption != "Customization Status" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 3) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Updated" || field->values[0].caption != "Updated") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Recompilation Needed" || field->values[1].caption != "Recompilation Needed") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Recompilation Failed" || field->values[2].caption != "Recompilation Failed") { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Customization Status");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{33});
  if (field == nullptr || field->name != "Role" || field->caption != "Role" || field->type != ::agiru::FieldType::Text || field->length != 100) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Role");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable, ::agiru::FieldNo{34});
  if (field == nullptr || field->name != "Emit Version" || field->caption != "Emit Version" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: User Personalization.Emit Version");
static_assert(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys.size() == 3, "native key count mismatch: User Personalization");
static_assert(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[0].fields[0] == ::agiru::FieldNo{3}, "native key declaration mismatch: User Personalization.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys.size() > 1 && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[1].name == "Key2" && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[1].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[1].fields[0] == ::agiru::FieldNo{9}, "native key declaration mismatch: User Personalization.Key2");
static_assert(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys.size() > 2 && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[2].name == "Key3" && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[2].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.keys[2].fields[0] == ::agiru::FieldNo{15}, "native key declaration mismatch: User Personalization.Key3");
static_assert(::agiru::TableTraits<::agiru::platform::UserPersonalization>::kTable.dataPerCompany == false, "native company scope mismatch: User Personalization");

