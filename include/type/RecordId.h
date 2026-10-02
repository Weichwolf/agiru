#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/Integer.h"
#include "type/Refusal.h"

#include <compare>
#include <expected>
#include <span>
#include <string>
#include <vector>

/// \file
/// \brief AL `RecordId` -- which table, and which row of it.

namespace agiru {

/// \brief AL `RecordId`.
///
/// From `recordid-data-type.md`: "Contains the table number and the primary key of a table."
///
/// \note THE TEXT FORM IS `Caption: key,key` AND THE SEPARATOR IS LOAD-BEARING. BaseApp code splits
///       on it twice, in `MergeDuplicate` and `MergeDuplicateConflicts`:
///       `PrimaryKey := CopyStr(Format(RecordID), StrPos(Format(RecordID), ': ') + 2)`. A worked
///       example gives `Sales Header: Order,101001` -- the table's caption, then `": "`, then the
///       primary key values separated by commas. A blank RecordId formats to the empty string,
///       which `CalcItemAvailability` and `ServiceConnection` both test for.
///
/// \note `GetRecord()` ANSWERS A TOKEN, NOT A `RecordRef`: `RecordRef` is built on the record base
///       this header sits under, and naming it here would turn the door's direction around. The
///       token carries the id, and `RecordRef::operator=` takes it -- so `RecRef := Id.GetRecord()`
///       reads as AL and lands where AL lands.
namespace detail {
struct RefusedRow;
}

class RecordId {
public:
  /// \brief A blank RecordId, which is what an unset field holds.
  RecordId() = default;

  /// \brief Builds one from a record's table and key.
  /// \param table   The AL table number.
  /// \param caption The table's caption, which the text form names first.
  /// \param key     The primary key values, already rendered, in key order.
  RecordId(TableId table, std::string caption, std::vector<std::string> key)
      : table_(table), caption_(std::move(caption)), key_(std::move(key)) {}

  /// \return True when this identifies no record.
  [[nodiscard]] bool IsEmpty() const { return key_.empty(); }

  /// \brief The primary key values this id carries, in key order.
  /// \return The values, in the form the database round-trips.
  ///
  /// \note IT IS NOT AN AL MEMBER. `Record.Get(RecordId)` is, and it has to write these into the
  ///       record's own key fields -- so the id lends what it holds rather than the record base
  ///       reaching into it.
  [[nodiscard]] std::span<const std::string> KeyValues() const { return key_; }

  /// \brief AL `RecordId.TableNo()`.
  ///
  /// \return The table number; 0 for a blank RecordId.
  ///
  /// \note THE PAGE SAYS "returns an error if the record is blank" AND THE BASEAPP SAYS OTHERWISE
  ///       FIVE TIMES: `NotificationLifecycleMgt` and `ErrorMessageManagement` branch on
  ///       `RecId.TableNo = 0`, the Error Messages pages enable an action on `RecID.TableNo <> 0`.
  ///       Those lines run on every notification and every error page with no context record,
  ///       so the sentence is the C/SIDE page's and not the platform's; refusing here failed 11
  ///       cases of `Inc Doc Attachment Overview UT` the moment notifications ran (2026-09-11).
  [[nodiscard]] Integer TableNo() const;

  /// \brief AL `RecordId.GetRecord()` -- a `RecordRef` on the table this id names, with the
  ///        primary key set from the id and NOTHING READ (`recordid-getrecord-method.md`: "No data
  ///        is read from the database ... no other fields in the record are set ... no filters").
  /// \return The token `RecordRef::operator=` takes; anything else that reads it refuses.
  [[nodiscard]] detail::RefusedRow GetRecord() const;

  /// \brief The form a COLUMN holds, which round-trips and is not the one a message shows.
  ///
  /// \return `<table>\x1f<caption>\x1f<key>\x1f<key>`, or the empty string when blank.
  ///
  /// \warning IT IS NOT `ToText()`, AND THE PAIR IS THE SAME ONE `FieldText` AND `StorageText`
  ///          ARE. `Format(RecordId)` renders `Customer: 10000` -- a caption, a colon and the key
  ///          -- and nothing can read that back, because a caption is not a table number and is
  ///          translated. The stored form carries the NUMBER, the caption beside it so the display
  ///          form survives the round trip, and the key values as their own columns hold them.
  ///          The separator is ASCII UNIT SEPARATOR, which no AL value contains.
  [[nodiscard]] std::string ToStorageText() const;

  /// \brief Reads back what `ToStorageText` wrote.
  /// \param text The stored form.
  /// \return The id, or WHY the text is not one.
  /// \note SQL SERVER'S BLANK IS SIX ZERO BYTES, and the demo database carries it as the hex text
  ///       `\x000000000000` in every RecordId column no row ever set; that is the empty id and
  ///       not a refusal (`Bank Account."Bank Stmt. Service Record ID"`, 9 UT cases, 2026-09-09).
  [[nodiscard]] static std::expected<RecordId, Refusal> FromStorageText(std::string_view text);

  /// \brief AL `Format(RecordId)`.
  /// \return `Caption: key,key`, or the empty string when blank.
  [[nodiscard]] std::string ToText() const;

  /// \brief Orders two RecordIds by table and then by key.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(const RecordId &o) const = default;

  /// \brief Compares two RecordIds.
  /// \param o The other.
  /// \return True when they name the same row of the same table.
  [[nodiscard]] bool operator==(const RecordId &o) const = default;

private:
  TableId table_{0};
  std::string caption_;
  std::vector<std::string> key_;
};

namespace detail {

/// \brief What `RecordId.GetRecord()` hands back: the id, for `RecordRef::operator=` to open and
///        key; read as anything else it refuses, because only a `RecordRef` is what AL declares.
struct RefusedRow {
  RecordId id; ///< The id the `RecordRef` will stand on.

  /// \brief Refuses to become a value of any other type.
  /// \tparam T The type the caller wants.
  /// \return Never.
  /// \throws Error always.
  template <typename T> operator T() const {
    throw Error("RecordId.GetRecord() answers a RecordRef and nothing else");
  }
};

}

inline detail::RefusedRow RecordId::GetRecord() const {
  return detail::RefusedRow{.id = *this};
}

}
