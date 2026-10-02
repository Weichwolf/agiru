#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/AllObjType.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
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
///       `src/Virtual Tables/AllObjWithCaption.Table.al` (28.0.53152.0).
///       Row providers and installed-object provenance are separate runtime contracts.

namespace agiru::platform {

class AllObjWithCaption_Table : public Table<AllObjWithCaption_Table> {
public:
  static constexpr TableId kId{2000000058};
  static constexpr std::string_view kName{"AllObjWithCaption"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kObjectNameLength = 30;
  static constexpr std::size_t kObjectCaptionLength = 249;
  /// \brief Length of the declared AL namespace, Text[500].
  static constexpr std::size_t kALNamespaceLength = 500;
  Option<AllObjType> ObjectType;
  ::agiru::Integer ObjectID{};
  Text<kObjectNameLength> ObjectName;
  Text<kObjectCaptionLength> ObjectCaption;
  Text<kObjectNameLength> ObjectSubtype;
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

  Guid AppRuntimePackageID;
  /// \brief AL application identity; distinct from either package identity.
  Guid AppID;
  /// \brief Original AL namespace; never reconstructed from the C++ identifier.
  Text<kALNamespaceLength> ALNamespace;

  struct Field_No {
    static constexpr ::agiru::FieldNo ObjectType{1};
    static constexpr ::agiru::FieldNo ObjectID{3};
    static constexpr ::agiru::FieldNo ObjectName{4};
    static constexpr ::agiru::FieldNo ObjectCaption{20};
    static constexpr ::agiru::FieldNo ObjectSubtype{30};
    static constexpr ::agiru::FieldNo AppPackageID{60};
    static constexpr ::agiru::FieldNo AppRuntimePackageID{61};
    /// \brief AL field 62, App ID.
    static constexpr ::agiru::FieldNo AppID{62};
    /// \brief AL field 63, AL Namespace.
    static constexpr ::agiru::FieldNo ALNamespace{63};
  };

  static constexpr std::array<::agiru::FieldNo, 2> kKey1{
      {Field_No::ObjectType, Field_No::ObjectID}};
};

using AllObjWithCaption = AllObjWithCaption_Table;

inline constexpr std::array<FieldDef, 9> kAllObjWithCaptionFields{{
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
    Declare<&AllObjWithCaption::AppID>(
        AllObjWithCaption::Field_No::AppID, "App ID", "App ID", offsetof(AllObjWithCaption, AppID)),
    Declare<&AllObjWithCaption::ALNamespace>(AllObjWithCaption::Field_No::ALNamespace,
                                             "AL Namespace",
                                             "AL Namespace",
                                             offsetof(AllObjWithCaption, ALNamespace)),
}};

inline constexpr std::array<KeyDef, 1> kAllObjWithCaptionKeys{{
    KeyDef{.name = "pk", .fields = AllObjWithCaption::kKey1, .clustered = true},
}};

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

template <> struct agiru::TableTraits<agiru::platform::AllObjWithCaption> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllObjWithCaptionTable;
};
