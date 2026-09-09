#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/RecordId.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace agiru::platform {

/// \brief `Record Link`.`Type`: a link to a document, or a note kept in `Note`.
enum class RecordLinkType : std::int32_t {
  Link = 0,
  Note = 1,
};

}

template <> struct agiru::OptionTraits<agiru::platform::RecordLinkType> {
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Link", .caption = "Link"},
      {.ordinal = 1, .name = "Note", .caption = "Note"},
  }};
};

namespace agiru::platform {

/// \brief The platform table `Record Link` (2000000068): the links and notes attached to any
///        record, which `Record.AddLink`, `CopyLinks` and the `Record Link Management` module
///        read and write. Database-wide, keyed by `Link ID`, which the platform assigns.
///
/// \note THE PLATFORM DECLARES IT AND NO `.al` FILE DOES, so the shape is written here from the
///       system-table reference (`devenv-table-object.md` names it among the system tables) and
///       from what the BaseApp reads of it: `Record ID`, `URL1`, `Description`, `Type`, `Note`,
///       `Created`, `User ID`, `Company`, `Notify`, `To User ID`. `URL2` to `URL4` are the
///       obsolete columns the platform still carries, so the field numbers stay the platform's.
class RecordLink_Table : public Table<RecordLink_Table> {
public:
  /// \brief The table number.
  static constexpr TableId kId{2000000068};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Record Link"};

  /// \brief The record variable's state; first, so the runtime reaches it at offset 0.
  detail::StateHandle State_Block;

  /// \brief A URL is `Text[2048]`.
  static constexpr std::size_t kUrlLength = 2048;
  /// \brief `Description` is `Text[250]`.
  static constexpr std::size_t kDescriptionLength = 250;
  /// \brief A user id is `Code[50]`.
  static constexpr std::size_t kUserLength = 50;
  /// \brief `Company` is `Text[30]`, the length of a company name.
  static constexpr std::size_t kCompanyLength = 30;

  /// \brief The primary key, assigned by the platform.
  ::agiru::Integer LinkID{};
  /// \brief The record the link belongs to.
  ::agiru::RecordId RecordID;
  /// \brief The link's address.
  Text<kUrlLength> URL1;
  /// \brief Obsolete; carried for the field number.
  Text<kUrlLength> URL2;
  /// \brief Obsolete; carried for the field number.
  Text<kUrlLength> URL3;
  /// \brief Obsolete; carried for the field number.
  Text<kUrlLength> URL4;
  /// \brief What the link shows.
  Text<kDescriptionLength> Description;
  /// \brief Link or note.
  Option<RecordLinkType> Type;
  /// \brief A note's text, written by `BinaryWriter` and read by `BinaryReader`.
  Blob Note;
  /// \brief When it was made.
  DateTime Created;
  /// \brief Who made it.
  Code<kUserLength> UserID;
  /// \brief The company the record lives in.
  Text<kCompanyLength> Company;
  /// \brief Whether the addressee is notified.
  Boolean Notify{};
  /// \brief The addressee.
  Code<kUserLength> ToUserID;

  /// \brief The field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo LinkID{1};
    static constexpr ::agiru::FieldNo RecordID{2};
    static constexpr ::agiru::FieldNo URL1{3};
    static constexpr ::agiru::FieldNo URL2{4};
    static constexpr ::agiru::FieldNo URL3{5};
    static constexpr ::agiru::FieldNo URL4{6};
    static constexpr ::agiru::FieldNo Description{7};
    static constexpr ::agiru::FieldNo Type{8};
    static constexpr ::agiru::FieldNo Note{9};
    static constexpr ::agiru::FieldNo Created{10};
    static constexpr ::agiru::FieldNo UserID{11};
    static constexpr ::agiru::FieldNo Company{12};
    static constexpr ::agiru::FieldNo Notify{13};
    static constexpr ::agiru::FieldNo ToUserID{14};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::LinkID}};
  /// \brief The secondary key the BaseApp walks links by.
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::RecordID}};
};

/// \brief The name the BaseApp uses.
using RecordLink = RecordLink_Table;

/// \brief The field table.
inline constexpr std::array<FieldDef, 14> kRecordLinkFields{{
    Declare<&RecordLink::LinkID>(RecordLink::Field_No::LinkID,
                                 "Link ID",
                                 "Link ID",
                                 offsetof(RecordLink, LinkID),
                                 Declared{.autoIncrement = true}),
    Declare<&RecordLink::RecordID>(
        RecordLink::Field_No::RecordID, "Record ID", "Record ID", offsetof(RecordLink, RecordID)),
    Declare<&RecordLink::URL1>(RecordLink::Field_No::URL1, "URL1", "URL1", offsetof(RecordLink, URL1)),
    Declare<&RecordLink::URL2>(RecordLink::Field_No::URL2, "URL2", "URL2", offsetof(RecordLink, URL2)),
    Declare<&RecordLink::URL3>(RecordLink::Field_No::URL3, "URL3", "URL3", offsetof(RecordLink, URL3)),
    Declare<&RecordLink::URL4>(RecordLink::Field_No::URL4, "URL4", "URL4", offsetof(RecordLink, URL4)),
    Declare<&RecordLink::Description>(RecordLink::Field_No::Description,
                                      "Description",
                                      "Description",
                                      offsetof(RecordLink, Description)),
    Declare<&RecordLink::Type>(RecordLink::Field_No::Type, "Type", "Type", offsetof(RecordLink, Type)),
    Declare<&RecordLink::Note>(RecordLink::Field_No::Note, "Note", "Note", offsetof(RecordLink, Note)),
    Declare<&RecordLink::Created>(
        RecordLink::Field_No::Created, "Created", "Created", offsetof(RecordLink, Created)),
    Declare<&RecordLink::UserID>(
        RecordLink::Field_No::UserID, "User ID", "User ID", offsetof(RecordLink, UserID)),
    Declare<&RecordLink::Company>(
        RecordLink::Field_No::Company, "Company", "Company", offsetof(RecordLink, Company)),
    Declare<&RecordLink::Notify>(
        RecordLink::Field_No::Notify, "Notify", "Notify", offsetof(RecordLink, Notify)),
    Declare<&RecordLink::ToUserID>(
        RecordLink::Field_No::ToUserID, "To User ID", "To User ID", offsetof(RecordLink, ToUserID)),
}};

/// \brief The keys.
inline constexpr std::array<KeyDef, 2> kRecordLinkKeys{{
    KeyDef{.name = "Key1", .fields = RecordLink::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = RecordLink::kKey2, .clustered = false},
}};

/// \brief The table.
inline constexpr TableDef kRecordLinkTable{
    .id = RecordLink::kId,
    .name = RecordLink::kName,
    .caption = "Record Link",
    .fields = kRecordLinkFields,
    .keys = kRecordLinkKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kRecordLinkTable), "the field table is sorted by number");
static_assert(offsetof(RecordLink, State_Block) == 0, "the state is the first member");

}

template <> struct agiru::TableTraits<agiru::platform::RecordLink_Table> {
  static constexpr const TableDef &kTable = agiru::platform::kRecordLinkTable;
};
