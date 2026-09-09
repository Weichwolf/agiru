#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/AllObjType.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The platform's `AllObjWithCaption` table (2000000058): every installed object, one row each.
///
/// \note IT HAS NO AL SOURCE AND ITS ROWS COME FROM THE CATALOGUES. `ProvisionInstalled` writes one
///       row per installed table, codeunit and page into a clone that lacks them (board:0004); a
///       report or query has no catalogue yet and is counted there. The field numbers are assigned
///       here [SET] in the order the BaseApp names the fields; nothing in AL names one by number.

namespace agiru::platform {

class AllObjWithCaption_Table : public Table<AllObjWithCaption_Table> {
public:
  static constexpr TableId kId{2000000058};
  static constexpr std::string_view kName{"AllObjWithCaption"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kObjectNameLength = 30;
  static constexpr std::size_t kObjectCaptionLength = 249;
  Option<AllObjType> ObjectType;
  ::agiru::Integer ObjectID{};
  Text<kObjectNameLength> ObjectName;
  Text<kObjectCaptionLength> ObjectCaption;
  Text<kObjectNameLength> ObjectSubtype;
  Guid AppPackageID;
  Guid AppRuntimePackageID;
  struct Field_No {
    static constexpr ::agiru::FieldNo ObjectType{1};
    static constexpr ::agiru::FieldNo ObjectID{3};
    static constexpr ::agiru::FieldNo ObjectName{4};
    static constexpr ::agiru::FieldNo ObjectCaption{5};
    static constexpr ::agiru::FieldNo ObjectSubtype{7};
    static constexpr ::agiru::FieldNo AppPackageID{20};
    static constexpr ::agiru::FieldNo AppRuntimePackageID{21};
  };
  static constexpr std::array<::agiru::FieldNo, 2> kKey1{{Field_No::ObjectType, Field_No::ObjectID}};
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::ObjectName}};
};

using AllObjWithCaption = AllObjWithCaption_Table;

inline constexpr std::array<FieldDef, 7> kAllObjWithCaptionFields{{
    Declare<&AllObjWithCaption::ObjectType>(
        AllObjWithCaption::Field_No::ObjectType, "Object Type", "Object Type", offsetof(AllObjWithCaption, ObjectType)),
    Declare<&AllObjWithCaption::ObjectID>(
        AllObjWithCaption::Field_No::ObjectID, "Object ID", "Object ID", offsetof(AllObjWithCaption, ObjectID)),
    Declare<&AllObjWithCaption::ObjectName>(
        AllObjWithCaption::Field_No::ObjectName, "Object Name", "Object Name", offsetof(AllObjWithCaption, ObjectName)),
    Declare<&AllObjWithCaption::ObjectCaption>(
        AllObjWithCaption::Field_No::ObjectCaption, "Object Caption", "Object Caption", offsetof(AllObjWithCaption, ObjectCaption)),
    Declare<&AllObjWithCaption::ObjectSubtype>(
        AllObjWithCaption::Field_No::ObjectSubtype, "Object Subtype", "Object Subtype", offsetof(AllObjWithCaption, ObjectSubtype)),
    Declare<&AllObjWithCaption::AppPackageID>(
        AllObjWithCaption::Field_No::AppPackageID, "App Package ID", "App Package ID", offsetof(AllObjWithCaption, AppPackageID)),
    Declare<&AllObjWithCaption::AppRuntimePackageID>(
        AllObjWithCaption::Field_No::AppRuntimePackageID, "App Runtime Package ID", "App Runtime Package ID", offsetof(AllObjWithCaption, AppRuntimePackageID)),
}};

inline constexpr std::array<KeyDef, 2> kAllObjWithCaptionKeys{{
    KeyDef{.name = "Key1", .fields = AllObjWithCaption::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = AllObjWithCaption::kKey2, .clustered = false},
}};

inline constexpr TableDef kAllObjWithCaptionTable{
    .id = AllObjWithCaption::kId,
    .name = AllObjWithCaption::kName,
    .caption = AllObjWithCaption::kName,
    .fields = kAllObjWithCaptionFields,
    .keys = kAllObjWithCaptionKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kAllObjWithCaptionTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::AllObjWithCaption> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllObjWithCaptionTable;
};
