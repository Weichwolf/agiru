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
#include <vector>

/// \file
/// \brief AL `FilterPageBuilder` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `FilterPageBuilder` -- named filter controls, one per table, each a `RecordRef`
///        whose filters the page would show and edit.
///
/// `filterpagebuilder-data-type.md`: a control is added by name for a table (`AddTable`,
/// `AddRecord`, `AddRecordRef`), fields are put on it (`AddField`, `AddFieldNo`), its view is set
/// and read (`SetView`, `GetView`), and `RunModal` shows the page. Here the controls are held as
/// RecordRefs and `RunModal` reaches the case's `[FilterPageHandler]`, which receives the first
/// control's RecordRef and answers whether the user accepted; without one the page is cancelled.
class FilterPageBuilder {
public:
  /// \brief AL `FilterPageBuilder.AddField(Text, FieldRef, Text)`. Adds a table field to the filter
  /// control for a table on filter page.
  /// \param Name The AL `Text`.
  /// \param Field The AL `FieldRef`.
  /// \param Filter The AL `Text`.
  /// \return The AL `Boolean`.
  ::agiru::Boolean
  AddField(std::string_view Name, const ::agiru::FieldRef &Field, std::string_view Filter = {});

  /// \brief AL `FilterPageBuilder.AddField(Text, Any, Text)`. Adds a table field to the filter
  /// control for a table on filter page.
  /// \param Name The AL `Text`.
  /// \param Field The AL `Any`.
  /// \param Filter The AL `Text`.
  /// \return The AL `Boolean`.
  ::agiru::Boolean
  AddField(std::string_view Name, const ::agiru::Variant &Field, std::string_view Filter = {});

  /// \brief AL `FilterPageBuilder.AddFieldNo(Text, Integer, Text)`. Adds a table field to the
  /// filter control for a table on the filter page.
  /// \param Name The AL `Text`.
  /// \param FieldNo The AL `Integer`.
  /// \param Filter The AL `Text`.
  /// \return The AL `Boolean`.
  ::agiru::Boolean
  AddFieldNo(std::string_view Name, ::agiru::Integer FieldNo, std::string_view Filter = {});

  /// \brief AL `FilterPageBuilder.AddRecord(Text, Record)`. Adds a filter control for a table to a
  /// filter page. The table is specified by a record data type variable that is passed to the
  /// method.
  /// \param Name The AL `Text`.
  /// \param Record The AL `Record`.
  /// \return The AL `Text`.
  std::string AddRecord(std::string_view Name, const ::agiru::RecordRef &Record);

  /// \brief AL `FilterPageBuilder.AddRecordRef(Text, RecordRef)`. Adds a filter control for a table
  /// to a filter page. The table is specified by a RecordRef variable that is passed to the method.
  /// This creates a filter control on the filter page, where users can set filter table data.
  /// \param Name The AL `Text`.
  /// \param RecordRef The AL `RecordRef`.
  /// \return The AL `Text`.
  std::string AddRecordRef(std::string_view Name, const ::agiru::RecordRef &RecordRef);

  /// \brief AL `FilterPageBuilder.AddTable(Text, Integer)`. Adds filter control for a table to a
  /// filter page.
  /// \param Name The AL `Text`.
  /// \param TableNo The AL `Integer`.
  /// \return The AL `Text`.
  std::string AddTable(const ::agiru::TextArgument &Name, ::agiru::Integer TableNo);

  /// \brief AL `FilterPageBuilder.Count()`. Gets the number of filter controls that are specified
  /// in the FilterPageBuilder object instance.
  /// \return The AL `Integer`.
  ::agiru::Integer Count();

  /// \brief AL `FilterPageBuilder.GetView(Text, Boolean)`. Gets the filter view (which defines the
  /// sort order, key, and filters) for the record in the specified filter control of a filter page.
  /// The view contains all fields in the filter control that have a default filter value.
  /// \param Name The AL `Text`.
  /// \param UseNames The AL `Boolean`.
  /// \return The AL `Text`.
  std::string GetView(std::string_view Name, ::agiru::Boolean UseNames = {});

  /// \brief AL `FilterPageBuilder.Name(Integer)`. Gets the name of a table filter control that is
  /// included on a filter page based on an index number that is assigned to the filter control.
  /// \param Index The AL `Integer`.
  /// \return The AL `Text`.
  std::string Name(::agiru::Integer Index);

  /// \brief AL `FilterPageBuilder.PageCaption(Text)`. Gets or sets the FilterPageBuilder UI
  /// caption. Defaults to the resource text if not explicitly set.
  /// \param PageCaption The AL `Text`.
  /// \return The AL `Text`.
  /// \brief AL `FilterPageBuilder.PageCaption()` -- the READING form, which the documentation's
  /// syntax block brackets: `[X := ] FilterPageBuilder.PageCaption([NewX])`.
  /// \return The value it holds.
  std::string PageCaption();

  std::string PageCaption(std::string_view PageCaption);

  /// \brief AL `FilterPageBuilder.RunModal()`. Builds and runs the filter page that includes the
  /// filter controls that are stored in FilterPageBuilder object instance.
  /// \return The AL `Boolean`.
  ::agiru::Boolean RunModal();

  /// \brief AL `FilterPageBuilder.SetView(Text, Text)`. Sets the current filter view, which defines
  /// the sort order, key, and filters, for a record in a filter control on a filter page. The view
  /// contains all fields that have default filters, but does not contain fields without filters.
  /// \param Name The AL `Text`.
  /// \param View The AL `Text`.
  /// \return The AL `Boolean`.
  ::agiru::Boolean SetView(std::string_view Name, std::string_view View);

private:
  struct Control_ {
    std::string name;
    ::agiru::RecordRef record;
    std::vector<::agiru::Integer> fields;
  };

  Control_ *Named_(std::string_view name);
  Control_ &Require_(std::string_view name, std::string_view method);
  std::string Add_(std::string_view name, ::agiru::RecordRef record);

  std::vector<Control_> controls_;
  std::string pageCaption_;
};

}
