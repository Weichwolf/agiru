#pragma once

#include "runtime/Error.h"
#include "runtime/RecordRef.h"
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
/// \brief AL `FilterPageBuilder` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `FilterPageBuilder`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/filterpagebuilder/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class FilterPageBuilder {
public:
  /// \brief AL `FilterPageBuilder.AddField(Text, FieldRef, Text)`. Adds a table field to the filter
  /// control for a table on filter page.
  /// \param Name The AL `Text`.
  /// \param Field The AL `FieldRef`.
  /// \param Filter The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean
  AddField(std::string_view Name, const ::agiru::FieldRef &Field, std::string_view Filter);

  /// \brief AL `FilterPageBuilder.AddField(Text, Any, Text)`. Adds a table field to the filter
  /// control for a table on filter page.
  /// \param Name The AL `Text`.
  /// \param Field The AL `Any`.
  /// \param Filter The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean
  AddField(std::string_view Name, const ::agiru::Variant &Field, std::string_view Filter);

  /// \brief AL `FilterPageBuilder.AddFieldNo(Text, Integer, Text)`. Adds a table field to the
  /// filter control for a table on the filter page.
  /// \param Name The AL `Text`.
  /// \param FieldNo The AL `Integer`.
  /// \param Filter The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean
  AddFieldNo(std::string_view Name, ::agiru::Integer FieldNo, std::string_view Filter);

  /// \brief AL `FilterPageBuilder.AddRecord(Text, Record)`. Adds a filter control for a table to a
  /// filter page. The table is specified by a record data type variable that is passed to the
  /// method.
  /// \param Name The AL `Text`.
  /// \param Record The AL `Record`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string AddRecord(std::string_view Name, const ::agiru::RecordRef &Record);

  /// \brief AL `FilterPageBuilder.AddRecordRef(Text, RecordRef)`. Adds a filter control for a table
  /// to a filter page. The table is specified by a RecordRef variable that is passed to the method.
  /// This creates a filter control on the filter page, where users can set filter table data.
  /// \param Name The AL `Text`.
  /// \param RecordRef The AL `RecordRef`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string AddRecordRef(std::string_view Name, const ::agiru::RecordRef &RecordRef);

  /// \brief AL `FilterPageBuilder.AddTable(Text, Integer)`. Adds filter control for a table to a
  /// filter page.
  /// \param Name The AL `Text`.
  /// \param TableNo The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string AddTable(std::string_view Name, ::agiru::Integer TableNo);

  /// \brief AL `FilterPageBuilder.Count()`. Gets the number of filter controls that are specified
  /// in the FilterPageBuilder object instance.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Count();

  /// \brief AL `FilterPageBuilder.GetView(Text, Boolean)`. Gets the filter view (which defines the
  /// sort order, key, and filters) for the record in the specified filter control of a filter page.
  /// The view contains all fields in the filter control that have a default filter value.
  /// \param Name The AL `Text`.
  /// \param UseNames The AL `Boolean`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetView(std::string_view Name, ::agiru::Boolean UseNames);

  /// \brief AL `FilterPageBuilder.Name(Integer)`. Gets the name of a table filter control that is
  /// included on a filter page based on an index number that is assigned to the filter control.
  /// \param Index The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Name(::agiru::Integer Index);

  /// \brief AL `FilterPageBuilder.PageCaption(Text)`. Gets or sets the FilterPageBuilder UI
  /// caption. Defaults to the resource text if not explicitly set.
  /// \param PageCaption The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string PageCaption(std::string_view PageCaption);

  /// \brief AL `FilterPageBuilder.RunModal()`. Builds and runs the filter page that includes the
  /// filter controls that are stored in FilterPageBuilder object instance.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean RunModal();

  /// \brief AL `FilterPageBuilder.SetView(Text, Text)`. Sets the current filter view, which defines
  /// the sort order, key, and filters, for a record in a filter control on a filter page. The view
  /// contains all fields that have default filters, but does not contain fields without filters.
  /// \param Name The AL `Text`.
  /// \param View The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetView(std::string_view Name, std::string_view View);
};

} // namespace agiru
