#pragma once

#include "meta/Ids.h"
#include "type/Integer.h"

#include <compare>
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
/// \note `GetRecord()` IS ABSENT. It returns a RecordRef, and there is no RecordRef in this runtime
///       yet; writing it would mean inventing a return the platform documents as something else.
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

  /// \brief AL `RecordId.TableNo()`.
  ///
  /// \return The table number.
  /// \throws Error when the RecordId is blank, which the page says it must: "This function returns
  ///         an error if the record is blank."
  [[nodiscard]] Integer TableNo() const;

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

} // namespace agiru
