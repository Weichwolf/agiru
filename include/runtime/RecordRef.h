#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/ErrorInfo.h"
#include "type/FieldClass.h"
#include "type/Integer.h"
#include "type/IsolationLevel.h"
#include "type/KeyRef.h"
#include "type/List.h"
#include "type/SecurityFilter.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `RecordRef` and `FieldRef` -- a record reached by NUMBER rather than by name.

// A REFUSAL DOES NOT USE `this`, and every generated body below is one -- so the checks that
// say so are true of the STATE and not of the design. They go away as the bodies land, and
// the suppression goes with them (board:0035). The parameter orders are AL's own, which is
// what makes the surface checkable against the documentation at all.
// NOLINTBEGIN(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard,performance-unnecessary-value-param)
namespace agiru {

/// \brief AL `FieldRef` -- one field of one record, reached without naming its type.
///
/// \note THIS IS WHAT THE FIELD TABLE WAS BUILT FOR. A generated record addresses its fields by
///       `offsetof` through `FieldDef`, and a FieldRef is that same address arrived at from the
///       other side: a number instead of a member. Nothing new is needed to hold one.
class FieldRef {
public:
  /// \brief A FieldRef pointing at nothing, which is what `var F: FieldRef` declares.
  ///
  /// \note AL DECLARES ONE BEFORE IT HAS ONE. `var RecRef: RecordRef; FieldRef: FieldRef;` then
  ///       `FieldRef := RecRef.Field(15)` -- so the declared state is empty and every question
  ///       asked of it before the assignment refuses rather than reading a null.
  FieldRef() = default;

  /// \brief A FieldRef over one field of one record.
  /// \param record The record.
  /// \param table  Its declaration.
  /// \param def    The field's declaration.
  FieldRef(void *record, const TableDef &table, const FieldDef &def)
      : record_(record), table_(&table), def_(&def) {}

  /// \brief AL `FieldRef.Number()`.
  /// \return The AL field number.
  [[nodiscard]] Integer Number() const { return def_->no.Value(); }

  /// \brief AL `FieldRef.Name()`.
  /// \return The AL name, spaces and all.
  [[nodiscard]] std::string_view Name() const { return def_->name; }

  /// \brief AL `FieldRef.Caption()`.
  /// \return The caption an error message quotes.
  [[nodiscard]] std::string_view Caption() const { return def_->caption; }

  /// \brief AL `FieldRef.Length()`.
  /// \return The declared length for Code and Text, 0 otherwise.
  [[nodiscard]] Integer Length() const { return def_->length; }

  /// \brief AL `FieldRef.Type()`.
  ///
  /// \return The field's AL data type, as the platform reports it.
  ///
  /// \warning AN ENUM FIELD REPORTS `Option`, AND THAT IS THE PLATFORM'S OWN ANSWER RATHER THAN A
  ///          simplification. `fieldtype-option.md` lists every member of the FieldType this
  ///          returns -- Boolean, Integer, BigInteger, Decimal, Option, Text, Code, DateTime, Time,
  ///          Date, DateFormula, Duration, Guid, RecordId, TableFilter, Blob, Media, MediaSet --
  ///          and there is NO `Enum` among them. BC tells the two apart through `IsEnum()` and
  ///          nowhere else. `BankPmtApplRuleUT` stands on it: it reads `Field.Type` from the
  ///          virtual Field table, leaves the procedure unless it is `Option`, and then asks for
  ///          `OptionMembers` -- on `Sales Header."Document Type"`, which is an Enum.
  [[nodiscard]] FieldType Type() const;

  /// \brief AL `FieldRef.IsEnum()`.
  /// \return True when the field is an enum rather than an option or anything else.
  [[nodiscard]] bool IsEnum() const { return def_->type == FieldType::Enum; }

  /// \brief AL `FieldRef.EnumValueCount()`.
  /// \return How many values the enumeration declares; 0 when the field is not one.
  [[nodiscard]] Integer EnumValueCount() const { return static_cast<Integer>(def_->values.size()); }

  /// \brief AL `FieldRef.GetEnumValueName(Index)`.
  ///
  /// \param index The ONE-BASED position in the value list, which is what the page says.
  /// \return The value's name, or empty when the index is outside the list.
  ///
  /// \note POSITION, NOT ORDINAL. The platform gives both this and
  ///       GetEnumValueNameFromOrdinalValue precisely because they are different questions:
  ///       `enum 50130 YesNo { value(0; Yes) value(10; No) }` answers `No` for index 2 and for
  ///       ordinal 10.
  [[nodiscard]] std::string_view GetEnumValueName(Integer index) const;

  /// \brief AL `FieldRef.GetEnumValueOrdinal(Index)`.
  /// \param index The one-based position.
  /// \return The declared ordinal there, or 0 when the index is outside the list.
  [[nodiscard]] Integer GetEnumValueOrdinal(Integer index) const;

  /// \brief AL `FieldRef.GetEnumValueNameFromOrdinalValue(Ordinal)`.
  /// \param ordinal The declared number.
  /// \return The value's name, or empty when the enumeration declares no such ordinal.
  [[nodiscard]] std::string_view GetEnumValueNameFromOrdinalValue(Integer ordinal) const;

  /// \brief AL `FieldRef.OptionMembers()`.
  /// \return The member names, in declaration order.
  [[nodiscard]] List<std::string> OptionMembers() const;

  /// \brief AL `FieldRef.Value()`.
  /// \return The field's value, carrying its type.
  /// \throws Error when the field's type has no Variant alternative yet.
  [[nodiscard]] Variant Value() const;

  /// \brief AL `FieldRef.Value := X` -- writes the field from text.
  /// \param text The value, as the column would hold it.
  /// \throws Error when the value does not fit the field.
  void SetValue(std::string_view text);

  // THE REST OF THE FieldRef SURFACE, from `methods-auto/fieldref/`. `FldRef.Class` and its
  // neighbours are what AL code branches on when it walks a table it cannot name.
  /// \brief AL `FieldRef.Active()`. Checks whether the field that is currently selected is enabled.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Active() const {
    throw Error("FieldRef.Active() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.CalcField()`. Updates FlowFields in a record.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean CalcField() const {
    throw Error("FieldRef.CalcField() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.CalcSum()`. Calculates the total of all values of a SumIndexField in a
  /// table.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean CalcSum() const {
    throw Error("FieldRef.CalcSum() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.Class()`. Gets the value of the FieldClass Property of the field that is
  /// currently selected. This method returns an error if no field is selected.
  /// \return The AL `FieldClass`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::FieldClass Class() const {
    throw Error("FieldRef.Class() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.FieldError(ErrorInfo)`. Stops the execution of the code, causing a
  /// run-time error, and creates an error message for a field.
  /// \param ErrorInfo The AL `ErrorInfo`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void FieldError(const ::agiru::ErrorInfo &ErrorInfo) const {
    static_cast<void>(ErrorInfo);
    throw Error("FieldRef.FieldError(ErrorInfo) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.FieldError(String)`. Stops the execution of the code, causing a run-time
  /// error, and creates an error message for a field.
  /// \param Text The AL `String`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void FieldError(std::string_view Text = {}) const {
    static_cast<void>(Text);
    throw Error("FieldRef.FieldError(String) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.GetEnumValueCaption(Integer)`. Gets an Enum value (or Option member)
  /// caption for the from the Enum metadata for the field that is currently selected.
  /// \param Index The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void GetEnumValueCaption(::agiru::Integer Index) const {
    static_cast<void>(Index);
    throw Error(
        "FieldRef.GetEnumValueCaption(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.GetEnumValueCaptionFromOrdinalValue(Integer)`. Gets an Enum value (or
  /// Option member) caption for the from the Enum metadata for the field that is currently
  /// selected.
  /// \param Ordinal The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void GetEnumValueCaptionFromOrdinalValue(::agiru::Integer Ordinal) const {
    static_cast<void>(Ordinal);
    throw Error("FieldRef.GetEnumValueCaptionFromOrdinalValue(Integer) is declared and not "
                "implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.GetFilter()`. Gets the filter that is currently applied to the field
  /// referred to by FieldRef.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetFilter() const {
    throw Error("FieldRef.GetFilter() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.GetRangeMax()`. Gets the maximum value in a range for a field.
  /// \return The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Variant GetRangeMax() const {
    throw Error("FieldRef.GetRangeMax() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.GetRangeMin()`. Gets the minimum value in a range for a field.
  /// \return The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Variant GetRangeMin() const {
    throw Error("FieldRef.GetRangeMin() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.IsOptimizedForTextSearch()`. Gets if the field is optimized for textual
  /// search.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsOptimizedForTextSearch() const {
    throw Error(
        "FieldRef.IsOptimizedForTextSearch() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.OptionCaption()`. Gets the option caption of the field that is currently
  /// selected.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string OptionCaption() const {
    throw Error("FieldRef.OptionCaption() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.OptionString()`. The 'OptionString' property has been deprecated and will
  /// be removed in the future. Use the 'OptionMembers' property instead.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string OptionString() const {
    throw Error("FieldRef.OptionString() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.Record()`. Gets the RecordRef of the field that is currently selected.
  /// This method returns an error if no field is selected.
  /// \return The AL `RecordRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::RecordRef Record() const;

  /// \brief AL `FieldRef.Relation()`. Finds the table relationship of a given field.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Relation() const {
    throw Error("FieldRef.Relation() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.SetFilter(Text, Any)`. Assigns a filter to a field that you specify.
  /// \param String The AL `Text`.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetFilter(std::string_view String, const ::agiru::Variant &Value) const {
    static_cast<void>(String);
    static_cast<void>(Value);
    throw Error("FieldRef.SetFilter(Text, Any) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.SetRange(Any, Any)`. Sets a simple filter on a field, such as a single
  /// range or a single value.
  /// \param FromValue The AL `Any`.
  /// \param ToValue The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetRange(const ::agiru::Variant &FromValue, const ::agiru::Variant &ToValue) const {
    static_cast<void>(FromValue);
    static_cast<void>(ToValue);
    throw Error("FieldRef.SetRange(Any, Any) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.Validate(Any)`. Use this method to enter a new value into a field and have
  /// the new value validated by the properties and code that have been defined for that field.
  /// \param NewValue The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Validate(const ::agiru::Variant &NewValue) const {
    static_cast<void>(NewValue);
    throw Error("FieldRef.Validate(Any) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `FieldRef.TestField()` -- raises when the field is blank.
  /// \throws Error with the platform's own wording when the field holds its zero.
  void TestField() const;

private:
  void *record_ = nullptr;
  const TableDef *table_ = nullptr;
  const FieldDef *def_ = nullptr;
};

/// \brief AL `RecordRef` -- a record reached without naming its table.
///
/// \note IT DOES NOT OWN THE RECORD. `GetTable(Rec)` points a RecordRef at a record that already
///       exists, which is how the BaseApp uses it: `RecRef.GetTable(SalesLine)` and then walk the
///       fields. `Open(TableNo)` -- which makes a record out of a number alone -- needs a registry
///       from table number to declaration that this runtime does not have yet, and refuses rather
///       than handing back something empty.
class RecordRef {
public:
  /// \brief A RecordRef pointing at nothing.
  RecordRef() = default;

  /// \brief AL `RecordRef.GetTable(Record)` -- points at an existing record.
  /// \tparam T The generated table class.
  /// \param rec The record.
  template <typename T> void GetTable(T &rec) {
    record_ = &rec;
    table_ = &TableTraits<T>::kTable;
  }

  /// \brief AL `RecordRef.Open(TableNo, Temporary [, Company])`.
  ///
  /// \param tableNo   The AL table number.
  /// \param temporary Whether the record has no database behind it.
  /// \param company   The company to open it in.
  /// \throws Error always, for now.
  void Open(Integer tableNo, Boolean temporary, std::string_view company = {}) {
    static_cast<void>(temporary);
    static_cast<void>(company);
    Open(tableNo);
  }

  /// \brief AL `RecordRef.Open(TableNo)`.
  /// \param tableNo The AL table number.
  /// \throws Error always, for now.
  /// \warning REFUSED. Making a record from a number needs a registry from table number to
  ///          declaration, and a RecordRef that opened nothing would answer every question with a
  ///          zero rather than saying it opened nothing.
  void Open(Integer tableNo);

  /// \return True when this RecordRef points at a record.
  [[nodiscard]] bool IsOpen() const { return record_ != nullptr; }

  /// \brief AL `RecordRef.Number()`.
  /// \return The AL table number.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer Number() const;

  /// \brief AL `RecordRef.Name()`.
  /// \return The table's AL name.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] std::string_view Name() const;

  /// \brief AL `RecordRef.FieldCount()`.
  /// \return How many fields the table declares.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer FieldCount() const;

  /// \brief AL `RecordRef.Field(FieldNo)`.
  /// \param fieldNo The AL field number.
  /// \return A FieldRef over it.
  /// \throws Error when the table declares no such field.
  [[nodiscard]] FieldRef Field(Integer fieldNo) const;

  /// \brief AL `RecordRef.FieldIndex(Index)`.
  /// \param index The ONE-BASED position in the field list.
  /// \return A FieldRef over the field there.
  /// \throws Error when the index is outside the list.
  [[nodiscard]] FieldRef FieldIndex(Integer index) const;

  /// \brief AL `RecordRef.FieldExist(FieldNo)`.
  /// \param fieldNo The AL field number.
  /// \return True when the table declares it.
  [[nodiscard]] bool FieldExist(Integer fieldNo) const;

  // WHAT A RecordRef CAN DO, from `methods-auto/recordref/`. AL code opens one and then works it
  // like a record it cannot name -- `RecRef.ChangeCompany(X)`, `RecRef.Next()` -- and every one of
  // those is a signature the documentation states, optional parameters and all.
  /// \brief AL `RecordRef.AddLink(Text, Text)`. Adds a link to a record in a table.
  /// \param URL The AL `Text`.
  /// \param Description The AL `Text`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer AddLink(std::string_view URL, std::string_view Description = {}) {
    static_cast<void>(URL);
    static_cast<void>(Description);
    throw Error("RecordRef.AddLink(Text, Text) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.AddLoadFields(Integer)`. Specifies additional fields to be initially
  /// loaded when the record is retrieved from its data source. Subsequent calls to AddLoadFields
  /// will not overwrite fields already selected for the initial load.
  /// \param Fields The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddLoadFields(::agiru::Integer Fields = {}) {
    static_cast<void>(Fields);
    throw Error(
        "RecordRef.AddLoadFields(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.AreFieldsLoaded(Integer)`. Checks whether the specified fields are all
  /// initially loaded.
  /// \param Fields The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AreFieldsLoaded(::agiru::Integer Fields) {
    static_cast<void>(Fields);
    throw Error(
        "RecordRef.AreFieldsLoaded(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Ascending(Boolean)`. Changes or checks the order in which a search
  /// through the table that is referred to by RecordRef will be performed.
  /// \param SetAscending The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Ascending(::agiru::Boolean SetAscending = {}) {
    static_cast<void>(SetAscending);
    throw Error("RecordRef.Ascending(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Caption()`. Gets the caption of the table that is currently selected.
  /// Returns an error if no table is selected.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Caption() {
    throw Error("RecordRef.Caption() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.ChangeCompany(Text)`. Redirects references to table data from one company
  /// to another.
  /// \param CompanyName The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ChangeCompany(std::string_view CompanyName = {}) {
    static_cast<void>(CompanyName);
    throw Error("RecordRef.ChangeCompany(Text) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.ClearMarks()`. Removes all the marks from a record.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void ClearMarks() {
    throw Error("RecordRef.ClearMarks() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Copy(RecordRef, Boolean)`. Copies a specified record referece's filters,
  /// views, automatically calculated FlowFields, marks, fields, and keys that are associated with
  /// the record from a table or creates a reference to a record.
  /// \param FromRecordRef The AL `RecordRef`.
  /// \param ShareTable The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Copy(const ::agiru::RecordRef &FromRecordRef, ::agiru::Boolean ShareTable = {}) {
    static_cast<void>(FromRecordRef);
    static_cast<void>(ShareTable);
    throw Error(
        "RecordRef.Copy(RecordRef, Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Copy(Record, Boolean)`. Copies a specified record's filters, views,
  /// automatically calculated FlowFields, marks, fields, and keys that are associated with the
  /// record from a table or creates a reference to a record.
  /// \param FromRecord The AL `Record`.
  /// \param ShareTable The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Copy(::agiru::RecordRef &FromRecord, ::agiru::Boolean ShareTable = {}) {
    static_cast<void>(FromRecord);
    static_cast<void>(ShareTable);
    throw Error("RecordRef.Copy(Record, Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.CopyLinks(RecordRef)`. Copies all the links from a particular record.
  /// \param FromRecord The AL `RecordRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CopyLinks(const ::agiru::RecordRef &FromRecord) {
    static_cast<void>(FromRecord);
    throw Error("RecordRef.CopyLinks(RecordRef) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.CopyLinks(Variant)`. Copies all the links from a particular record.
  /// \param FromRecordOrRecordRef The AL `Variant`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CopyLinks(const ::agiru::Variant &FromRecordOrRecordRef) {
    static_cast<void>(FromRecordOrRecordRef);
    throw Error("RecordRef.CopyLinks(Variant) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Count()`. Counts the number of records that are in the filters that are
  /// currently applied to the table referred to by the RecordRef.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Count() {
    throw Error("RecordRef.Count() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.CountApprox()`. Gets an approximate count of the number of records in the
  /// table
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer CountApprox() {
    throw Error("RecordRef.CountApprox() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.CurrentCompany()`. Gets the current company of a database table referred
  /// to by a RecordRef.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string CurrentCompany() {
    throw Error("RecordRef.CurrentCompany() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.CurrentKey()`. Gets the current key of the table referred to by the
  /// RecordRef. The current key is returned as a string.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string CurrentKey() {
    throw Error("RecordRef.CurrentKey() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.CurrentKeyIndex(Integer)`. Gets or sets the current key of the table
  /// referred to by the RecordRef. The current key is set or returned as a number. This first key =
  /// 1, and so on. If RecordRef does not have an active record, CURRENTKEYINDEX will return -1. If
  /// this value is then passed to KEYINDEX, an index out of bounds error will occur. Therefore it
  /// is important to implement a check of the RecordRef parameter.
  /// \param NewKeyIndex The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer CurrentKeyIndex(::agiru::Integer NewKeyIndex = {}) {
    static_cast<void>(NewKeyIndex);
    throw Error(
        "RecordRef.CurrentKeyIndex(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Delete(Boolean)`. Deletes a record in a table.
  /// \param RunTrigger The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Delete(::agiru::Boolean RunTrigger = {}) {
    static_cast<void>(RunTrigger);
    throw Error("RecordRef.Delete(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.DeleteAll(Boolean)`. Deletes all records in a table that fall within a
  /// specified range.
  /// \param RunTrigger The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void DeleteAll(::agiru::Boolean RunTrigger = {}) {
    static_cast<void>(RunTrigger);
    throw Error("RecordRef.DeleteAll(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.DeleteLink(Integer)`. Deletes a specified link from a record in a table.
  /// \param ID The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void DeleteLink(::agiru::Integer ID) {
    static_cast<void>(ID);
    throw Error("RecordRef.DeleteLink(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.DeleteLinks()`. Deletes all of the links that have been added to a
  /// record.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void DeleteLinks() {
    throw Error("RecordRef.DeleteLinks() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Duplicate()`. Duplicates the table that contains the RecordRef.
  /// \return The AL `RecordRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::RecordRef Duplicate() {
    throw Error("RecordRef.Duplicate() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.FilterGroup(Integer)`. Changes the filter group that is being applied to
  /// the table. You can also use this method to return the number of the current filtergroup. You
  /// cannot return the number of the filtergroup and set a new filtergroup at the same time.
  /// \param NewGroup The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer FilterGroup(::agiru::Integer NewGroup = {}) {
    static_cast<void>(NewGroup);
    throw Error("RecordRef.FilterGroup(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Find(Text)`. Finds a record in a table based on the values stored in the
  /// key fields.
  /// \param Which The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Find(std::string_view Which = {}) {
    static_cast<void>(Which);
    throw Error("RecordRef.Find(Text) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.FindFirst()`. Finds the first record in a table based on the current key
  /// and filter.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindFirst() {
    throw Error("RecordRef.FindFirst() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.FindLast()`. Finds the last record in a table based on the current key
  /// and filter.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindLast() {
    throw Error("RecordRef.FindLast() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.FindSet(Boolean, Boolean)`. Finds a set of records in a table based on
  /// the current key and filter. FindSet can only retrieve records in ascending order.
  /// \param ForUpdate The AL `Boolean`.
  /// \param UpdateKey The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindSet(::agiru::Boolean ForUpdate, ::agiru::Boolean UpdateKey) {
    static_cast<void>(ForUpdate);
    static_cast<void>(UpdateKey);
    throw Error(
        "RecordRef.FindSet(Boolean, Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.FindSet(Boolean)`. Finds a set of records in a table based on the current
  /// key and filter. FINDSET can only retrieve records in ascending order.
  /// \param ForUpdate The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindSet(::agiru::Boolean ForUpdate = {}) {
    static_cast<void>(ForUpdate);
    throw Error("RecordRef.FindSet(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.FullyQualifiedName()`. Identifies the fully qualified name of the table.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string FullyQualifiedName() {
    throw Error("RecordRef.FullyQualifiedName() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Get(RecordId)`. Gets a record based on the ID of the record.
  /// \param RecordID The AL `RecordId`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(::agiru::RecordId RecordID) {
    static_cast<void>(RecordID);
    throw Error("RecordRef.Get(RecordId) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.GetBySystemId(Guid)`. Gets a record based on the ID of the record. The
  /// RecordRef must already be opened.
  /// \param SystemId The AL `Guid`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetBySystemId(::agiru::Guid SystemId) {
    static_cast<void>(SystemId);
    throw Error("RecordRef.GetBySystemId(Guid) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.GetFilters()`. Determines which filters have been applied to the table
  /// referred to by the RecordRef.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetFilters() {
    throw Error("RecordRef.GetFilters() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.GetPosition(Boolean)`. Gets a string that contains the primary key of the
  /// current record.
  /// \param UseNames The AL `Boolean`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetPosition(::agiru::Boolean UseNames = {}) {
    static_cast<void>(UseNames);
    throw Error("RecordRef.GetPosition(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.GetView(Boolean)`. Returns a string that describes the current sort
  /// order, key, and filters on a table.
  /// \param UseNames The AL `Boolean`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetView(::agiru::Boolean UseNames = {}) {
    static_cast<void>(UseNames);
    throw Error("RecordRef.GetView(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.HasFilter()`. Determines whether a filter has been applied to the table
  /// that the RecordRef refers to.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HasFilter() {
    throw Error("RecordRef.HasFilter() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.HasLinks()`. Determines whether a record contains any links.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HasLinks() {
    throw Error("RecordRef.HasLinks() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Init()`. Initializes a record in a table.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Init() { throw Error("RecordRef.Init() is declared and not implemented yet (board:0035)"); }

  /// \brief AL `RecordRef.Insert()`. Inserts a record into a table without executing the code in
  /// the OnInsert trigger.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert() {
    throw Error("RecordRef.Insert() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Insert(Boolean, Boolean)`. Inserts a record into a table.
  /// \param RunTrigger The AL `Boolean`.
  /// \param InsertWithSystemId The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Boolean RunTrigger, ::agiru::Boolean InsertWithSystemId) {
    static_cast<void>(RunTrigger);
    static_cast<void>(InsertWithSystemId);
    throw Error(
        "RecordRef.Insert(Boolean, Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Insert(Boolean)`. Inserts a record into a table.
  /// \param RunTrigger The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Boolean RunTrigger) {
    static_cast<void>(RunTrigger);
    throw Error("RecordRef.Insert(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.IsDirty()`. Gets a boolean value that indicates whether the current
  /// in-memory instance of a record or filtered set of records has changed since being retrieved
  /// from the database.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsDirty() {
    throw Error("RecordRef.IsDirty() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.IsEmpty()`. Determines whether any records exist in a filtered set of
  /// records in a table.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsEmpty() {
    throw Error("RecordRef.IsEmpty() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.IsTemporary()`. Determines whether a RecordRef refers to a temporary
  /// table.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTemporary() {
    throw Error("RecordRef.IsTemporary() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.KeyIndex(Integer)`. Gets the KeyRef of the key that has the index
  /// specified in the table that is currently selected. The key can be composed of fields of any
  /// supported data type. Data types that are not supported include BLOBs, FlowFilters, variables,
  /// and functions. If the sorting key is set to a field that is not part of a key, then the
  /// KEYINDEX is -1.
  /// \param Index The AL `Integer`.
  /// \return The AL `KeyRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::KeyRef KeyIndex(::agiru::Integer Index) {
    static_cast<void>(Index);
    throw Error("RecordRef.KeyIndex(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.LoadFields(Integer)`. Accesses the table's corresponding data source and
  /// loads the values of the specified fields on the record.
  /// \param Fields The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean LoadFields(::agiru::Integer Fields) {
    static_cast<void>(Fields);
    throw Error("RecordRef.LoadFields(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.LockTable(Boolean, Boolean)`. Starts locking on a table to protect it
  /// from write transactions that conflict with each other.
  /// \param Wait The AL `Boolean`.
  /// \param VersionCheck The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void LockTable(::agiru::Boolean Wait = {}, ::agiru::Boolean VersionCheck = {}) {
    static_cast<void>(Wait);
    static_cast<void>(VersionCheck);
    throw Error(
        "RecordRef.LockTable(Boolean, Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Mark(Boolean)`. Marks a record. You can also use this method to determine
  /// whether a record is marked.
  /// \param Mark The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Mark(::agiru::Boolean Mark = {}) {
    static_cast<void>(Mark);
    throw Error("RecordRef.Mark(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.MarkedOnly(Boolean)`. Activates a special filter. After you use this
  /// function, your view of the table includes only records marked by the Mark method (RecordRef).
  /// \param MarkedOnly The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean MarkedOnly(::agiru::Boolean MarkedOnly = {}) {
    static_cast<void>(MarkedOnly);
    throw Error("RecordRef.MarkedOnly(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Modify(Boolean)`. Modifies a record in a table.
  /// \param RunTrigger The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Modify(::agiru::Boolean RunTrigger = {}) {
    static_cast<void>(RunTrigger);
    throw Error("RecordRef.Modify(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Next(Integer)`. Steps through a specified number of records and retrieves
  /// a record.
  /// \param Steps The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Next(::agiru::Integer Steps = {}) {
    static_cast<void>(Steps);
    throw Error("RecordRef.Next(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.ReadConsistency()`. Gets a value indicating whether read consistency is
  /// enabled.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadConsistency() {
    throw Error("RecordRef.ReadConsistency() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.ReadIsolation(IsolationLevel)`. Gets or sets the read isolation level.
  /// \param ReadIsolation The AL `IsolationLevel`.
  /// \return The AL `IsolationLevel`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::IsolationLevel ReadIsolation(const ::agiru::IsolationLevel &ReadIsolation) {
    static_cast<void>(ReadIsolation);
    throw Error(
        "RecordRef.ReadIsolation(IsolationLevel) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.ReadPermission()`. Determines if you can read from a table.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadPermission() {
    throw Error("RecordRef.ReadPermission() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.RecordId()`. Gets the RecordID of the record that is currently selected
  /// in the table. If no table is selected, an error is generated.
  /// \return The AL `RecordId`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::RecordId RecordId() {
    throw Error("RecordRef.RecordId() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.RecordLevelLocking()`. Gets a value indicating whether record level
  /// locking is enabled.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean RecordLevelLocking() {
    throw Error("RecordRef.RecordLevelLocking() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Rename(Any, Any)`. Changes the value of a primary key in a table.
  /// \param Value1 The AL `Any`.
  /// \param Value2 The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Rename(const ::agiru::Variant &Value1, const ::agiru::Variant &Value2) {
    static_cast<void>(Value1);
    static_cast<void>(Value2);
    throw Error("RecordRef.Rename(Any, Any) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Reset()`. Removes all filters, including any special filters set by the
  /// MarkedOnly method (Record), changes fields select for loading back to all, sets the read
  /// isolation level to the default value, and changes the current key to the primary key. Also
  /// removes any marks on the record and clears any AL variables defined on its table definition.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Reset() {
    throw Error("RecordRef.Reset() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SecurityFiltering(SecurityFilter)`. Gets or sets how security filters are
  /// applied to the RecordRef.
  /// \param NewSecurityFiltering The AL `SecurityFilter`.
  /// \return The AL `SecurityFilter`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::SecurityFilter SecurityFiltering(const ::agiru::SecurityFilter &NewSecurityFiltering) {
    static_cast<void>(NewSecurityFiltering);
    throw Error("RecordRef.SecurityFiltering(SecurityFilter) is declared and not implemented yet "
                "(board:0035)");
  }

  /// \brief AL `RecordRef.SetAutoCalcFields(Integer)`. Sets the FlowFields that you specify to be
  /// automatically calculated when the RecordRef is retrieved from the database.
  /// \param Fields The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetAutoCalcFields(::agiru::Integer Fields = {}) {
    static_cast<void>(Fields);
    throw Error(
        "RecordRef.SetAutoCalcFields(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetLoadFields(Integer)`. Sets the fields to be initially loaded when the
  /// record is retrieved from its data source. This will overwrite fields previously selected for
  /// initial load.
  /// \param Fields The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetLoadFields(::agiru::Integer Fields = {}) {
    static_cast<void>(Fields);
    throw Error(
        "RecordRef.SetLoadFields(Integer) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetPermissionFilter()`. Applies the user's security filter to the
  /// referenced record. The security filter is combined with any other filters that are placed on
  /// the record with SetFilter or SetRange. The combined filter will not include any records
  /// outside the range of the security filter and this will prevent a runtime permission error from
  /// occuring when the record is read. If the permission filter is not set, an error can occur if
  /// you attempt to read a record that is outside the range of the user's security filter.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetPermissionFilter() {
    throw Error("RecordRef.SetPermissionFilter() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetPosition(Text)`. Sets the fields in a primary key on a record to the
  /// values specified in the String parameter. The remaining fields are not changed.
  /// \param String The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetPosition(std::string_view String) {
    static_cast<void>(String);
    throw Error("RecordRef.SetPosition(Text) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetRecFilter()`. Sets a filter on a record that is referred to by a
  /// RecordRef.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetRecFilter() {
    throw Error("RecordRef.SetRecFilter() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetTable(Record, Boolean)`. Sets the table to which a Record variable
  /// refers as the same table as a RecordRef variable.
  /// \param Rec The AL `Record`.
  /// \param ShareTable The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetTable(const ::agiru::RecordRef &Rec, ::agiru::Boolean ShareTable) {
    static_cast<void>(Rec);
    static_cast<void>(ShareTable);
    throw Error(
        "RecordRef.SetTable(Record, Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetTable(Record)`. Sets the table to which a Record variable refers as
  /// the same table as a RecordRef variable.
  /// \param Rec The AL `Record`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetTable(const ::agiru::RecordRef &Rec) {
    static_cast<void>(Rec);
    throw Error("RecordRef.SetTable(Record) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetView(Text)`. Sets the current sort order, key, and filters on a table.
  /// \param String The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetView(std::string_view String) {
    static_cast<void>(String);
    throw Error("RecordRef.SetView(Text) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SystemCreatedAtNo()`. Gets the field number that is used by the
  /// SystemCreatedAt field. The SystemCreatedAt field is a system field that the platform adds to
  /// all table objects.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer SystemCreatedAtNo() {
    throw Error("RecordRef.SystemCreatedAtNo() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SystemCreatedByNo()`. Gets the field number that is used by the
  /// SystemCreatedBy field. The SystemCreatedBy field is a system field that the platform adds to
  /// all table objects.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer SystemCreatedByNo() {
    throw Error("RecordRef.SystemCreatedByNo() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SystemIdNo()`. Gets the field number that is used by the SystemId field.
  /// The SystemId field is a system field that the platform adds to all table objects.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer SystemIdNo() {
    throw Error("RecordRef.SystemIdNo() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SystemModifiedAtNo()`. Gets the field number that is used by the
  /// SystemModifiedAt field. The SystemModifiedAt field is a system field that the platform adds to
  /// all table objects.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer SystemModifiedAtNo() {
    throw Error("RecordRef.SystemModifiedAtNo() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SystemModifiedByNo()`. Gets the field number that is used by the
  /// SystemModifiedBy field. The SystemModifiedBy field is a system field that the platform adds to
  /// all table objects.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer SystemModifiedByNo() {
    throw Error("RecordRef.SystemModifiedByNo() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Truncate(Boolean)`. Deletes all records in a table that fall within a
  /// specified range, in an efficient maner. Keep in mind that Truncate allows for less concurrency
  /// than DeleteAll, as the entire table will be locked until the transaction is committed.
  /// \param ResetAutoIncrement The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Truncate(::agiru::Boolean ResetAutoIncrement = {}) {
    static_cast<void>(ResetAutoIncrement);
    throw Error("RecordRef.Truncate(Boolean) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.WritePermission()`. Determines if you can write to a table.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WritePermission() {
    throw Error("RecordRef.WritePermission() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Close()` -- lets go of the record it was opened on.
  /// \throws Error until `Open` can make one (board:0025).
  void Close() {
    record_ = nullptr;
    table_ = nullptr;
  }

  /// \brief AL `RecordRef.KeyCount()`.
  /// \return How many keys the table declares.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer KeyCount() const;

private:
  [[nodiscard]] const TableDef &Table() const;

  void *record_ = nullptr;
  const TableDef *table_ = nullptr;
};

// NOLINTEND(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard,performance-unnecessary-value-param)

} // namespace agiru
