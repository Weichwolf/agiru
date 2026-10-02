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
/// \brief The platform's `AllObj` table (2000000038): every installed object, one row each.
///
/// \note IT HAS NO AL SOURCE AND ITS ROWS COME FROM THE CATALOGUES. `ProvisionInstalled` writes one
///       row per installed table, codeunit and page into a clone that lacks them (board:0004); a
///       report or query has no catalogue yet and is counted there. The field numbers are assigned
///       here [SET] in the order the BaseApp names the fields; nothing in AL names one by number.

namespace agiru::platform {

class AllObj_Table : public Table<AllObj_Table> {
public:
  static constexpr TableId kId{2000000038};
  static constexpr std::string_view kName{"AllObj"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kObjectNameLength = 30;
  static constexpr std::size_t kObjectCaptionLength = 249;
  Option<AllObjType> ObjectType;
  ::agiru::Integer ObjectID{};
  Text<kObjectNameLength> ObjectName;
  Text<kObjectNameLength> ObjectSubtype;
  Guid AppPackageID;
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

  Guid AppRuntimePackageID;

  struct Field_No {
    static constexpr ::agiru::FieldNo ObjectType{1};
    static constexpr ::agiru::FieldNo ObjectID{3};
    static constexpr ::agiru::FieldNo ObjectName{4};
    static constexpr ::agiru::FieldNo ObjectSubtype{7};
    static constexpr ::agiru::FieldNo AppPackageID{20};
    static constexpr ::agiru::FieldNo AppRuntimePackageID{21};
  };

  static constexpr std::array<::agiru::FieldNo, 2> kKey1{
      {Field_No::ObjectType, Field_No::ObjectID}};
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::ObjectName}};
};

using AllObj = AllObj_Table;

inline constexpr std::array<FieldDef, 6> kAllObjFields{{
    Declare<&AllObj::ObjectType>(
        AllObj::Field_No::ObjectType, "Object Type", "Object Type", offsetof(AllObj, ObjectType)),
    Declare<&AllObj::ObjectID>(
        AllObj::Field_No::ObjectID, "Object ID", "Object ID", offsetof(AllObj, ObjectID)),
    Declare<&AllObj::ObjectName>(
        AllObj::Field_No::ObjectName, "Object Name", "Object Name", offsetof(AllObj, ObjectName)),
    Declare<&AllObj::ObjectSubtype>(AllObj::Field_No::ObjectSubtype,
                                    "Object Subtype",
                                    "Object Subtype",
                                    offsetof(AllObj, ObjectSubtype)),
    Declare<&AllObj::AppPackageID>(AllObj::Field_No::AppPackageID,
                                   "App Package ID",
                                   "App Package ID",
                                   offsetof(AllObj, AppPackageID)),
    Declare<&AllObj::AppRuntimePackageID>(AllObj::Field_No::AppRuntimePackageID,
                                          "App Runtime Package ID",
                                          "App Runtime Package ID",
                                          offsetof(AllObj, AppRuntimePackageID)),
}};

inline constexpr std::array<KeyDef, 2> kAllObjKeys{{
    KeyDef{.name = "Key1", .fields = AllObj::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = AllObj::kKey2, .clustered = false},
}};

inline constexpr TableDef kAllObjTable{
    .id = AllObj::kId,
    .name = AllObj::kName,
    .caption = AllObj::kName,
    .fields = kAllObjFields,
    .keys = kAllObjKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kAllObjTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::AllObj> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllObjTable;
};
