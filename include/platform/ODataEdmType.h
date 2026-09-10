#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The platform table `OData Edm Type` (2000000203): the EDM definitions the OData layer
///        publishes, which `Graph Mgt - General Tools` writes and the OData EDM pages show.

namespace agiru::platform {

/// \brief The platform table `OData Edm Type` (2000000203).
///
/// \note THE PLATFORM DECLARES IT AND NO `.al` FILE DOES, so the shape is written here from what
///       the BaseApp reads of it: `Key`, `Description` and the `Edm Xml` blob its pages and
///       `GraphMgtGeneralTools` name.
class ODataEdmType_Table : public Table<ODataEdmType_Table> {
public:
  /// \brief The table number.
  static constexpr TableId kId{2000000203};
  /// \brief The AL name.
  static constexpr std::string_view kName{"OData Edm Type"};

  /// \brief The record variable's state; first, so the runtime reaches it at offset 0.
  detail::StateHandle State_Block;

  /// \brief The key is `Code[50]`.
  static constexpr std::size_t kKeyLength = 50;
  /// \brief The description is `Text[250]`.
  static constexpr std::size_t kDescriptionLength = 250;

  /// \brief What the definition is called; the primary key.
  Code<kKeyLength> Key;
  /// \brief What it is for.
  Text<kDescriptionLength> Description;
  /// \brief The definition itself.
  Blob EdmXml;

  /// \brief AL `OData Edm Type.SystemId`.
  Guid SystemId;
  /// \brief AL `OData Edm Type.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `OData Edm Type.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `OData Edm Type.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `OData Edm Type.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief The field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo Key{1};
    static constexpr ::agiru::FieldNo Description{2};
    static constexpr ::agiru::FieldNo EdmXml{3};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::Key}};
};

/// \brief The name the BaseApp uses.
using ODataEdmType = ODataEdmType_Table;

/// \brief The field table.
inline constexpr auto kODataEdmTypeFields =
    WithSystemFields<ODataEdmType>(std::array<FieldDef, 3>{{
        Declare<&ODataEdmType::Key>(
            ODataEdmType::Field_No::Key, "Key", "Key", offsetof(ODataEdmType, Key)),
        Declare<&ODataEdmType::Description>(ODataEdmType::Field_No::Description,
                                            "Description",
                                            "Description",
                                            offsetof(ODataEdmType, Description)),
        Declare<&ODataEdmType::EdmXml>(
            ODataEdmType::Field_No::EdmXml, "Edm Xml", "Edm Xml", offsetof(ODataEdmType, EdmXml)),
    }});

/// \brief The keys.
inline constexpr std::array<KeyDef, 1> kODataEdmTypeKeys{{
    KeyDef{.name = "Key1", .fields = ODataEdmType::kKey1, .clustered = true},
}};

/// \brief The table.
inline constexpr TableDef kODataEdmTypeTable{
    .id = ODataEdmType::kId,
    .name = ODataEdmType::kName,
    .caption = "OData Edm Type",
    .fields = kODataEdmTypeFields,
    .keys = kODataEdmTypeKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kODataEdmTypeTable), "the field table is sorted by number");
static_assert(offsetof(ODataEdmType, State_Block) == 0, "the state is the first member");

}

/// \brief The traits the runtime reaches the table through.
template <> struct agiru::TableTraits<agiru::platform::ODataEdmType_Table> {
  /// \brief The table.
  static constexpr const TableDef &kTable = agiru::platform::kODataEdmTypeTable;
};
