#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/AllObjType.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The System `AllObj` declaration (2000000038), without caption-only fields.
///
/// \note `src/Virtual Tables/AllObj.Table.al` in System.app (29.0.55365.0) owns the fields/key.
///       The read-only provider and installed-object provenance are separate runtime contracts.

namespace agiru::platform {

/// \brief AL AllObj with its seven declared fields and tenant-wide scope.
class AllObj_Table : public Table<AllObj_Table> {
public:
  /// \brief The System table number.
  static constexpr TableId kId{2000000038};
  /// \brief The original AL table name.
  static constexpr std::string_view kName{"AllObj"};
  /// \brief Per-record state owned by the common table runtime.
  detail::StateHandle State_Block;
  /// \brief Declared Object Name length, Text[30].
  static constexpr std::size_t kObjectNameLength = 30;
  /// \brief Declared AL Namespace length, Text[500].
  static constexpr std::size_t kALNamespaceLength = 500;
  /// \brief AL Object Type, including reserved option positions.
  Option<AllObjType> ObjectType;
  /// \brief AL Object ID.
  ::agiru::Integer ObjectID{};
  /// \brief Original AL Object Name, without Code normalization.
  Text<kObjectNameLength> ObjectName;
  /// \brief Full AL Name, field 5, Text[100]; independent of the legacy Object Name.
  Text<100> Name;
  /// \brief AL App Package ID, field 60.
  Guid AppPackageID;
  /// \brief AL App Runtime Package ID, field 61.
  Guid AppRuntimePackageID;
  /// \brief Original AL Namespace, field 62, not AllObjWithCaption's App ID.
  Text<kALNamespaceLength> ALNamespace;
  /// \brief AL `AllObj.SystemId`.
  Guid SystemId;
  /// \brief AL `AllObj.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `AllObj.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `AllObj.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `AllObj.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief Implicit read-only SQL rowversion buffer; provider ownership is separate.
  BigInteger SystemRowVersion{};
  /// \brief Current creator User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemCreatedByUserName{};
  /// \brief Current creator full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemCreatedByFullName{};
  /// \brief Current modifier User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemModifiedByUserName{};
  /// \brief Current modifier full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemModifiedByFullName{};

  /// \brief System-source field numbers, not display order.
  struct Field_No : SystemFieldNumbers {
    /// \brief Object Type field number.
    static constexpr ::agiru::FieldNo ObjectType{1};
    /// \brief Object ID field number.
    static constexpr ::agiru::FieldNo ObjectID{3};
    /// \brief Object Name field number.
    static constexpr ::agiru::FieldNo ObjectName{4};
    /// \brief Full Name field number.
    static constexpr ::agiru::FieldNo Name{5};
    /// \brief App Package ID field number.
    static constexpr ::agiru::FieldNo AppPackageID{60};
    /// \brief App Runtime Package ID field number.
    static constexpr ::agiru::FieldNo AppRuntimePackageID{61};
    /// \brief AL Namespace field number.
    static constexpr ::agiru::FieldNo ALNamespace{62};
  };

  /// \brief The sole declared key, Object Type followed by Object ID.
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{
      {Field_No::ObjectType, Field_No::ObjectID}};
};

/// \brief The native ABI name for AL AllObj.
using AllObj = AllObj_Table;

/// \brief All seven source fields followed by the common implicit system fields.
inline constexpr auto kAllObjFields = WithImplicitFields<AllObj,
                                                         ::agiru::SystemFieldProfile::Runtime18,
                                                         ::agiru::TableType::Normal,
                                                         false>(std::array<FieldDef, 7>{{
    Declare<&AllObj::ObjectType>(
        AllObj::Field_No::ObjectType, "Object Type", "Object Type", offsetof(AllObj, ObjectType)),
    Declare<&AllObj::ObjectID>(
        AllObj::Field_No::ObjectID, "Object ID", "Object ID", offsetof(AllObj, ObjectID)),
    Declare<&AllObj::ObjectName>(
        AllObj::Field_No::ObjectName, "Object Name", "Object Name", offsetof(AllObj, ObjectName)),
    Declare<&AllObj::Name>(AllObj::Field_No::Name, "Name", "Name", offsetof(AllObj, Name)),
    Declare<&AllObj::AppPackageID>(AllObj::Field_No::AppPackageID,
                                   "App Package ID",
                                   "App Package ID",
                                   offsetof(AllObj, AppPackageID)),
    Declare<&AllObj::AppRuntimePackageID>(AllObj::Field_No::AppRuntimePackageID,
                                          "App Runtime Package ID",
                                          "App Runtime Package ID",
                                          offsetof(AllObj, AppRuntimePackageID)),
    Declare<&AllObj::ALNamespace>(AllObj::Field_No::ALNamespace,
                                  "AL Namespace",
                                  "AL Namespace",
                                  offsetof(AllObj, ALNamespace)),
}});

/// \brief Only the System-source primary key; no invented name index.
inline constexpr std::array<KeyDef, 1> kAllObjKeys{{
    KeyDef{.name = "pk", .fields = AllObj::kKey1, .clustered = true},
}};

/// \brief Tenant-wide AllObj metadata; permissions retain the declared spelling.
inline constexpr TableDef kAllObjTable{
    .id = AllObj::kId,
    .name = AllObj::kName,
    .caption = AllObj::kName,
    .fields = kAllObjFields,
    .keys = kAllObjKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
};

static_assert(FieldsAreSorted(kAllObjTable), "the field table is searched by number");

}

/// \brief The native AllObj declaration used by generic table operations.
template <> struct agiru::TableTraits<agiru::platform::AllObj> {
  /// \brief The immutable source-compatible table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllObjTable;
};
