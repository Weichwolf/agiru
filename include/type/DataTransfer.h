#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `DataTransfer` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `DataTransfer`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/datatransfer/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class DataTransfer {
public:
  /// \brief AL `DataTransfer.AddConstantValue(Any, Integer)`. Specifies the given value is to be
  /// set in the given field in the destination table.
  /// \param Value The AL `Any`.
  /// \param DestinationField The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddConstantValue(const ::agiru::Variant &Value, ::agiru::Integer DestinationField);

  /// \brief AL `DataTransfer.AddDestinationFilter(Integer, Text, Any)`. Adds a filter for the
  /// destination table for the data transfer.
  /// \param DestinationField The AL `Integer`.
  /// \param String The AL `Text`.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddDestinationFilter(::agiru::Integer DestinationField,
                            std::string_view String,
                            const ::agiru::Variant &Value);

  /// \brief AL `DataTransfer.AddFieldValue(Integer, Integer)`. Specifies a source and destination
  /// field, where the values from the source field are to be copied to the destination field. The
  /// data types of the fields must match, except CODE to TEXT which is allowed.
  /// \param SourceField The AL `Integer`.
  /// \param DestinationField The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddFieldValue(::agiru::Integer SourceField, ::agiru::Integer DestinationField);

  /// \brief AL `DataTransfer.AddJoin(Integer, Integer)`. Adds a field pair to be used to create a
  /// join condition which determines which rows to transfer, optional for same table transfers.
  /// \param SourceField The AL `Integer`.
  /// \param DestinationField The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddJoin(::agiru::Integer SourceField, ::agiru::Integer DestinationField);

  /// \brief AL `DataTransfer.AddSourceFilter(Integer, Text, Any)`. Adds a filter for the source
  /// table for the data transfer.
  /// \param SourceField The AL `Integer`.
  /// \param String The AL `Text`.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddSourceFilter(::agiru::Integer SourceField,
                       std::string_view String,
                       const ::agiru::Variant &Value);

  /// \brief AL `DataTransfer.CopyFields()`. Copies the fields specified in AddFields with filters
  /// from AddSourceFilter, and the join conditions from AddJoins in one bulk operation in SQL.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CopyFields();

  /// \brief AL `DataTransfer.CopyRows()`. Copies the rows from the source table to the destination
  /// table with the fields selected with AddFields and the filters applied with AddSourceFilter, in
  /// one bulk operation in SQL.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CopyRows();

  /// \brief AL `DataTransfer.SetTables(Integer, Integer)`. Sets the source and destination tables
  /// for the data transfer.
  /// \param SourceTable The AL `Integer`.
  /// \param DestinationTable The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetTables(::agiru::Integer SourceTable, ::agiru::Integer DestinationTable);

  /// \brief AL `DataTransfer.UpdateAuditFields(Boolean)`. Sets if audit fields should be updated.
  /// If the value is set to false, the audit fields are not updated when calling the CopyFields
  /// method. Default value is true.
  /// \param UpdateAuditFields The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean UpdateAuditFields(::agiru::Boolean UpdateAuditFields);
};

}
