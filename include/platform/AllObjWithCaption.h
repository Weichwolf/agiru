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
/// \brief The platform's `AllObjWithCaption` table (2000000058): every installed object, one row
/// each.
///
/// \note Fields and the primary key follow System.app's
///       `src/Virtual Tables/AllObjWithCaption.Table.al` (29.0.55365.0).
///       Row providers and installed-object provenance are separate runtime contracts.

namespace agiru::platform {

/// \brief Source-declared object catalogue with caption and application identity.
class AllObjWithCaption_Table : public Table<AllObjWithCaption_Table> {
public:
  /// \brief The System table number.
  static constexpr TableId kId{2000000058};
  /// \brief The original AL table name.
  static constexpr std::string_view kName{"AllObjWithCaption"};
  /// \brief Per-record state owned by the common table runtime.
  detail::StateHandle State_Block;
  /// \brief Declared Object Name and Object Subtype length, Text[30].
  static constexpr std::size_t kObjectNameLength = 30;
  /// \brief Declared Object Caption length, Text[249].
  static constexpr std::size_t kObjectCaptionLength = 249;
  /// \brief Length of the declared AL namespace, Text[500].
  static constexpr std::size_t kALNamespaceLength = 500;
  /// \brief AL Object Type, including reserved option positions.
  Option<AllObjType> ObjectType;
  /// \brief AL Object ID.
  ::agiru::Integer ObjectID{};
  /// \brief Original AL name; independent of its caption.
  Text<kObjectNameLength> ObjectName;
  /// \brief Full AL Name, field 5, Text[100]; independent of the legacy Object Name.
  Text<100> Name;
  /// \brief Source object caption, not an alternative object identity.
  Text<kObjectCaptionLength> ObjectCaption;
  /// \brief Source object subtype.
  Text<kObjectNameLength> ObjectSubtype;
  /// \brief AL App Package ID, field 60.
  Guid AppPackageID;
  /// \brief AL `AllObjWithCaption.SystemId`.
  Guid SystemId;
  /// \brief AL `AllObjWithCaption.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `AllObjWithCaption.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `AllObjWithCaption.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `AllObjWithCaption.SystemModifiedBy`.
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

  /// \brief AL App Runtime Package ID, field 61.
  Guid AppRuntimePackageID;
  /// \brief AL application identity; distinct from either package identity.
  Guid AppID;
  /// \brief Original AL namespace; never reconstructed from the C++ identifier.
  Text<kALNamespaceLength> ALNamespace;

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
    /// \brief Object Caption field number.
    static constexpr ::agiru::FieldNo ObjectCaption{20};
    /// \brief Object Subtype field number.
    static constexpr ::agiru::FieldNo ObjectSubtype{30};
    /// \brief App Package ID field number.
    static constexpr ::agiru::FieldNo AppPackageID{60};
    /// \brief App Runtime Package ID field number.
    static constexpr ::agiru::FieldNo AppRuntimePackageID{61};
    /// \brief AL field 62, App ID.
    static constexpr ::agiru::FieldNo AppID{62};
    /// \brief AL field 63, AL Namespace.
    static constexpr ::agiru::FieldNo ALNamespace{63};
  };

  /// \brief The sole declared key, Object Type followed by Object ID.
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{
      {Field_No::ObjectType, Field_No::ObjectID}};
};

/// \brief The native ABI name for AL AllObjWithCaption.
using AllObjWithCaption = AllObjWithCaption_Table;

/// \brief All ten source fields followed by the common implicit system fields.
inline constexpr auto kAllObjWithCaptionFields =
    WithImplicitFields<AllObjWithCaption,
                       ::agiru::SystemFieldProfile::Runtime18,
                       ::agiru::TableType::Normal,
                       false>(std::array<FieldDef, 10>{{
        Declare<&AllObjWithCaption::ObjectType>(AllObjWithCaption::Field_No::ObjectType,
                                                "Object Type",
                                                "Object Type",
                                                offsetof(AllObjWithCaption, ObjectType)),
        Declare<&AllObjWithCaption::ObjectID>(AllObjWithCaption::Field_No::ObjectID,
                                              "Object ID",
                                              "Object ID",
                                              offsetof(AllObjWithCaption, ObjectID)),
        Declare<&AllObjWithCaption::ObjectName>(AllObjWithCaption::Field_No::ObjectName,
                                                "Object Name",
                                                "Object Name",
                                                offsetof(AllObjWithCaption, ObjectName)),
        Declare<&AllObjWithCaption::Name>(
            AllObjWithCaption::Field_No::Name, "Name", "Name", offsetof(AllObjWithCaption, Name)),
        Declare<&AllObjWithCaption::ObjectCaption>(AllObjWithCaption::Field_No::ObjectCaption,
                                                   "Object Caption",
                                                   "Object Caption",
                                                   offsetof(AllObjWithCaption, ObjectCaption)),
        Declare<&AllObjWithCaption::ObjectSubtype>(AllObjWithCaption::Field_No::ObjectSubtype,
                                                   "Object Subtype",
                                                   "Object Subtype",
                                                   offsetof(AllObjWithCaption, ObjectSubtype)),
        Declare<&AllObjWithCaption::AppPackageID>(AllObjWithCaption::Field_No::AppPackageID,
                                                  "App Package ID",
                                                  "App Package ID",
                                                  offsetof(AllObjWithCaption, AppPackageID)),
        Declare<&AllObjWithCaption::AppRuntimePackageID>(
            AllObjWithCaption::Field_No::AppRuntimePackageID,
            "App Runtime Package ID",
            "App Runtime Package ID",
            offsetof(AllObjWithCaption, AppRuntimePackageID)),
        Declare<&AllObjWithCaption::AppID>(AllObjWithCaption::Field_No::AppID,
                                           "App ID",
                                           "App ID",
                                           offsetof(AllObjWithCaption, AppID)),
        Declare<&AllObjWithCaption::ALNamespace>(AllObjWithCaption::Field_No::ALNamespace,
                                                 "AL Namespace",
                                                 "AL Namespace",
                                                 offsetof(AllObjWithCaption, ALNamespace)),
    }});

/// \brief Only the System-source primary key; no invented name index.
inline constexpr std::array<KeyDef, 1> kAllObjWithCaptionKeys{{
    KeyDef{.name = "pk", .fields = AllObjWithCaption::kKey1, .clustered = true},
}};

/// \brief Tenant-wide catalogue metadata with the declared inherent permissions.
inline constexpr TableDef kAllObjWithCaptionTable{
    .id = AllObjWithCaption::kId,
    .name = AllObjWithCaption::kName,
    .caption = AllObjWithCaption::kName,
    .fields = kAllObjWithCaptionFields,
    .keys = kAllObjWithCaptionKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
};

static_assert(FieldsAreSorted(kAllObjWithCaptionTable), "the field table is searched by number");

}

/// \brief The native catalogue declaration used by generic table operations.
template <> struct agiru::TableTraits<agiru::platform::AllObjWithCaption> {
  /// \brief The immutable source-compatible table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllObjWithCaptionTable;
};
