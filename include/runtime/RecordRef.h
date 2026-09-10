#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
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

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard,performance-unnecessary-value-param)
namespace agiru::detail {

/// \brief A record several `RecordRef`s hold at once, freed when the last one lets go.
///
/// \note IT IS NOT A `std::shared_ptr`, AND THE REASON IS THE DOOR'S OWN BUILD TIME. Including
///       `<memory>` here costs every generated translation unit 1.2 s of the 3.4 s it takes to read
///       `agiru.h` at all (measured 2026-09-03, clang++-19 over libstdc++-14, where `<memory>`
///       pulls in `<format>` through `unique_ptr.h`). 5 835 generated sources pay that, so the
///       twenty lines are cheaper than the header. The shape is `Instance<T>`'s: a raw pointer and
///       the free function captured where the type was still complete (board:0037).
class SharedRecord {
public:
  /// \brief Holds nothing.
  SharedRecord() = default;

  /// \brief Takes ownership of a record.
  /// \param record The record.
  /// \param free   What unmakes it.
  SharedRecord(void *record, void (*free)(void *))
      : record_(record), free_(free), uses_(record == nullptr ? nullptr : new long{1}) {}

  /// \brief Shares another's record.
  /// \param o The other.
  SharedRecord(const SharedRecord &o) : record_(o.record_), free_(o.free_), uses_(o.uses_) {
    if (uses_ != nullptr) { ++*uses_; }
  }

  /// \brief Takes another's record.
  /// \param o The other.
  SharedRecord(SharedRecord &&o) noexcept : record_(o.record_), free_(o.free_), uses_(o.uses_) {
    o.record_ = nullptr;
    o.free_ = nullptr;
    o.uses_ = nullptr;
  }

  /// \brief Shares another's record, letting go of this one.
  /// \param o The other.
  /// \return This.
  SharedRecord &operator=(const SharedRecord &o) {
    if (this != &o) {
      SharedRecord copy(o);
      Swap(copy);
    }
    return *this;
  }

  /// \brief Takes another's record, letting go of this one.
  /// \param o The other.
  /// \return This.
  SharedRecord &operator=(SharedRecord &&o) noexcept {
    if (this != &o) { Swap(o); }
    return *this;
  }

  ~SharedRecord() { Reset(); }

  /// \brief Lets go, freeing the record when this was the last holder.
  void Reset() {
    if (uses_ != nullptr && --*uses_ == 0) {
      if (free_ != nullptr) { free_(record_); }
      delete uses_;
    }
    record_ = nullptr;
    free_ = nullptr;
    uses_ = nullptr;
  }

  /// \brief The record.
  /// \return It, or `nullptr`.
  void *Get() const { return record_; }

private:
  void Swap(SharedRecord &o) noexcept {
    void *record = record_;
    void (*free)(void *) = free_;
    long *uses = uses_;
    record_ = o.record_;
    free_ = o.free_;
    uses_ = o.uses_;
    o.record_ = record;
    o.free_ = free;
    o.uses_ = uses;
  }

  void *record_ = nullptr;
  void (*free_)(void *) = nullptr;
  long *uses_ = nullptr;
};

}

namespace agiru {
class RecordRef;
class Variant;

namespace detail {
/// \brief `RecordRef.GetTable(Variant)`: opens the held record's table and copies the row.
/// \param into The reference.
/// \param held The Variant.
void RecordRefFromVariant(RecordRef &into, const Variant &held);

/// \brief AL `FieldRef.Relation()`: the number of the table the field's `TableRelation` names,
///        0 when it names none or the table is not in this build.
/// \param def The field, or nothing.
/// \return The table number.
[[nodiscard]] ::agiru::Integer RelationTableNo(const FieldDef *def);
}
}

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
  [[nodiscard]] Integer Number() const { return Def_().no.Value(); }

  /// \brief AL `FieldRef.Name()`.
  /// \return The AL name, spaces and all.
  [[nodiscard]] std::string_view Name() const { return Def_().name; }

  /// \brief AL `FieldRef.Caption()`.
  /// \return The caption an error message quotes.
  [[nodiscard]] std::string_view Caption() const { return Def_().caption; }

  /// \brief AL `FieldRef.Length()`.
  /// \return The declared length for Code and Text, 0 otherwise.
  [[nodiscard]] Integer Length() const { return Def_().length; }

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
  [[nodiscard]] bool IsEnum() const { return Def_().type == FieldType::Enum; }

  /// \brief AL `FieldRef.EnumValueCount()`.
  /// \return How many values the enumeration declares; 0 when the field is not one.
  [[nodiscard]] Integer EnumValueCount() const {
    return static_cast<Integer>(Def_().values.size());
  }

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
  [[nodiscard]] std::string OptionMembers() const;

  /// \brief AL `FieldRef.Value()`.
  /// \return The field's value, carrying its type.
  /// \throws Error when the field's type has no Variant alternative yet.
  [[nodiscard]] Variant Value() const;

  /// \brief AL `Format(FieldRef)` -- the field's value as text.
  /// \return The value, rendered the way a message shows it.
  ///
  /// \note WITHOUT IT `Format(FieldRef)` WAS INFINITE RECURSION. `AsText` falls through to
  ///       `Format` for a value it cannot render, and the non-Variant `Format` is defined as
  ///       `AsText` -- so a type with neither a text conversion nor a `ToText` called the two in
  ///       turn until the stack ran out (measured 2026-09-08, `ERM Table Fields UT`).
  [[nodiscard]] std::string ToText() const;

  /// \brief AL `FieldRef.Value(NewValue)` -- the setter, written as a call.
  ///
  /// \tparam T What AL handed it.
  /// \param value The value.
  ///
  /// \note AL WRITES A PROPERTY SETTER AS A CALL. `FieldRef.Value(NoSeriesCode)` and
  ///       `FieldRef.Value := NoSeriesCode` are the same statement, and the BaseApp writes both.
  /// \note AND `Any` IS THE PARAMETER, so it renders rather than casts. `FieldRef.Value := Amount`
  ///       assigns a Decimal and `:= WorkDate` a Date, neither of which is a `std::string_view` --
  ///       a cast refused both, which the property-access rule then surfaced at every such
  ///       assignment at once.
  template <typename T> void Value(const T &value) { SetValue(AsText(value)); }

  /// \brief AL `FieldRef.Value := X` -- writes the field from text.
  /// \param text The value, as the column would hold it.
  /// \throws Error when the value does not fit the field.
  void SetValue(std::string_view text);

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
  /// currently selected.
  /// \return The AL `FieldClass` the table declared for this field.
  [[nodiscard]] ::agiru::FieldClass Class() const { return Def_().fieldClass; }

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
  /// \brief AL `FieldRef.GetEnumValueCaption(Integer)` -- the caption of the value at a ONE-BASED
  ///        position in the field's enumeration (`fieldref-getenumvaluecaption-method.md`).
  /// \param Index The position.
  /// \return The caption, empty outside the enumeration.
  [[nodiscard]] std::string_view GetEnumValueCaption(::agiru::Integer Index) const;

  /// \brief AL `FieldRef.GetEnumValueCaptionFromOrdinalValue(Integer)`. Gets an Enum value (or
  /// Option member) caption for the from the Enum metadata for the field that is currently
  /// selected.
  /// \param Ordinal The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `FieldRef.GetEnumValueCaptionFromOrdinalValue(Integer)` -- the caption of the value
  ///        with that ordinal.
  /// \param Ordinal The ordinal.
  /// \return The caption, empty when no value carries it.
  [[nodiscard]] std::string_view
  GetEnumValueCaptionFromOrdinalValue(::agiru::Integer Ordinal) const;

  /// \brief AL `FieldRef.GetFilter()`. The filter standing on this field.
  ///
  /// \return The filter expression, or the empty string when nothing narrows the field.
  ///
  /// \note IT READS WHAT `SetRange` AND `SetFilter` WROTE, out of the record's own state and out
  ///       of the CURRENT filter group -- the same list `Record.GetFilter(Field)` reads, reached
  ///       by number instead of by member.
  [[nodiscard]] std::string GetFilter() const;

private:
  [[nodiscard]] ::agiru::Variant RangeBound_(bool upper) const;

public:
  /// \brief AL `FieldRef.GetRangeMax()`. The upper bound of the range standing on the field.
  /// \return The bound as the field's value; the field's blank when nothing bounds it above.
  [[nodiscard]] ::agiru::Variant GetRangeMax() const { return RangeBound_(true); }

  /// \brief AL `FieldRef.GetRangeMin()`. The lower bound of the range standing on the field.
  /// \return The bound as the field's value; the field's blank when nothing bounds it below.
  [[nodiscard]] ::agiru::Variant GetRangeMin() const { return RangeBound_(false); }

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
    std::string out;
    for (const EnumValueDef &value : Def_().values) {
      if (!out.empty()) { out += ','; }
      out += value.name;
    }
    return out;
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
  /// \return A RecordRef on the record this field belongs to.
  /// \throws Error when the FieldRef names no field.
  ::agiru::RecordRef Record() const;

  /// \brief AL `FieldRef.Relation()`. Finds the table relationship of a given field.
  /// \return The related table's number, or 0 when the field relates to none in this build.
  ::agiru::Integer Relation() const { return detail::RelationTableNo(def_); }

  /// \brief AL `FieldRef.SetFilter(...)` where the filter text is a member the runtime has not
  ///        rebuilt.
  /// \tparam T The refusal's type, which marks itself with `IsAlRefusal`.
  /// \param refusal The refused member.
  /// \throws Error always, naming the member -- which is what reading it does anywhere else.
  ///
  /// \warning IT IS AN OVERLOAD AND NOT A CONVERSION. A refusal deliberately does not become a
  ///          `std::string_view`, because a `Code<N>` assignment then had two equally good
  ///          conversions; so every door method that takes text takes a refusal beside it, one at
  ///          a time as the tree asks for it (board:0035).
  template <typename T>
    requires requires { typename T::IsAlRefusal; }
  void SetFilter(const T &refusal) const {
    static_cast<void>(static_cast<std::int32_t>(refusal));
  }

  /// \brief AL `FieldRef.SetFilter(Text, Any)`. Assigns a filter to a field that you specify.
  /// \param String The AL `Text`.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetFilter(std::string_view String) const { SetFilterText(std::string(String)); }

  /// \brief AL `FieldRef.SetFilter(Text, Any)`: the one placeholder substituted.
  /// \param String The filter, with `%1`.
  /// \param Value  What `%1` stands for.
  void SetFilter(std::string_view String, const ::agiru::Variant &Value) const {
    SetFilterText(StrSubstNo(String, Value));
  }

  /// \brief AL `FieldRef.SetFilter(Text, Any, Any, ...)` -- the filter text with its `%1`
  ///        substitutions, any number of them.
  /// \tparam Values The substituted values.
  /// \param String The filter, with placeholders.
  /// \param Value1 The first value.
  /// \param Value2 The second.
  /// \param rest   Any more.
  template <typename... Values>
  void SetFilter(std::string_view String,
                 const ::agiru::Variant &Value1,
                 const ::agiru::Variant &Value2,
                 const Values &...rest) const {
    SetFilterText(StrSubstNo(String, Value1, Value2, rest...));
  }

  /// \brief The filter, already rendered, put on the field the way `Record.SetFilter` puts it.
  /// \param text The filter in AL's own language.
  /// \throws Error when the FieldRef names no field yet.
  void SetFilterText(const std::string &text) const;

  /// \brief AL `FieldRef.SetRange([From] [, To])`. Narrows the field to a value or a range.
  ///
  /// \param FromValue The value, or the lower bound; omitted, the filter is cleared.
  /// \param ToValue   The upper bound; omitted, the field must EQUAL `FromValue`.
  ///
  /// \note IT WRITES THE SAME FILTER `Record.SetRange` WRITES, into the same per-field list in the
  ///       record's own state. A FieldRef is a field of a record reached by number instead of by
  ///       member, so there is nothing else for it to write into.
  ///
  /// \note THE VALUE RENDERS AS FORMAT 9 AND NOT AS A CAPTION. A filter holds what the COLUMN
  ///       holds, so an option is its ordinal -- `Format(Value, 0, 9)` is the invariant form, and
  ///       the caption form would put `Email` where the column wants `3` (openerp WI-1008).
  void SetRange(const ::agiru::Variant &FromValue = {}, const ::agiru::Variant &ToValue = {}) const;

  /// \brief AL `FieldRef.Validate(Any)`. Use this method to enter a new value into a field and have
  /// the new value validated by the properties and code that have been defined for that field.
  /// \param NewValue The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Validate(const ::agiru::Variant &NewValue = {}) const;

  /// \brief AL `FieldRef.TestField()` -- raises when the field is blank.
  /// \throws Error with the platform's own wording when the field holds its zero.
  void TestField() const;

  /// \brief AL `FieldRef.TestField(Any)` -- `fieldref-testfield-*-method.md`, one page per type:
  ///        raises unless the field holds exactly the expected value.
  /// \tparam V Any value type a Variant can hold.
  /// \param Expected The value the field must hold.
  /// \throws Error naming the field, the way `Record.TestField(Field, Value)` does.
  template <typename V> void TestField(const V &Expected) const {
    if (!(Value() == ::agiru::Variant(Expected))) {
      throw Error(std::string(Name()) + " must be equal to '" + ::agiru::AsText(Expected) + "'");
    }
  }

private:
  [[nodiscard]] const FieldDef &Def_() const {
    if (def_ == nullptr) {
      throw Error("FieldRef: it names no field yet -- nothing has been assigned to it");
    }
    return *def_;
  }

  [[nodiscard]] const TableDef &Table_() const {
    if (table_ == nullptr) {
      throw Error("FieldRef: it names no field yet -- nothing has been assigned to it");
    }
    return *table_;
  }

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
namespace detail {

/// \brief What every copy of one AL `RecordRef` variable shares: the table it is open on, the
///        record it points at, and the ownership of that record.
///
/// \warning A `RecordRef` IS A REFERENCE TYPE IN AL, so a copy is a second handle on the SAME
///          object and not a second object. `GetRecRefAndFieldsNoByType(RecRef: RecordRef; ...)`
///          in the BaseApp takes the RecordRef BY VALUE, calls `RecRef.Open(...)` inside, and the
///          caller then runs `RecRef.SetView` on what the callee opened -- 658 by-value RecordRef
///          parameters are declared in the generated tree (counted 2026-09-09), and each relies on
///          the handle being shared. `Open`, `Close`, `GetTable` and `SetTable` therefore act on
///          this box, and `RecordRef.Copy(RecordRef)` is the one way to a second object.
struct RecordRefState {
  /// \brief The record, or nothing when the reference is closed.
  void *record = nullptr;
  /// \brief The table it is open on, or nothing.
  const TableDef *table = nullptr;
  /// \brief The record when this reference made it; empty when it was given one.
  SharedRecord owned;
  /// \brief How many handles share this box.
  long uses = 1;
};

}

class RecordRef {
public:
  /// \brief A RecordRef pointing at nothing.
  RecordRef() = default;

  /// \brief A second handle on the same object, which is what passing one by value gives AL.
  RecordRef(const RecordRef &o) : state_(o.Shared()) {}

  /// \brief Takes over the handle.
  RecordRef(RecordRef &&o) noexcept : state_(o.state_) { o.state_ = nullptr; }

  /// \brief Points this handle at the other's object, the way `RecRef2 := RecRef1` does in AL.
  RecordRef &operator=(const RecordRef &o) {
    if (this != &o) {
      detail::RecordRefState *next = o.Shared();
      Release();
      state_ = next;
    }
    return *this;
  }

  /// \brief Takes over the handle.
  RecordRef &operator=(RecordRef &&o) noexcept {
    if (this != &o) {
      Release();
      state_ = o.state_;
      o.state_ = nullptr;
    }
    return *this;
  }

  /// \brief Lets go of the handle, freeing the box with the last one.
  ~RecordRef() { Release(); }

  /// \brief AL `RecordRef.GetTable(Record)` -- when what AL held was an `Any`.
  ///
  /// \param rec The Variant, which must hold a record.
  /// \throws Error when it holds something else, or a table this binary does not carry.
  ///
  /// \note AL PASSES A RECORD THROUGH AN `Any` AND THE PLATFORM UNWRAPS IT.
  ///       `Assert.RecordIsEmpty(RecVariant)` is the shape, and the Variant carries the record's
  ///       address beside its table NUMBER -- which is what the catalogue looks the declaration up
  ///       by when the caller cannot name the table either.
  void GetTable(Variant &rec);

  /// \brief AL `RecordRef.GetTable(Record)` -- points at an existing record.
  /// \tparam T The generated table class.
  /// \param rec The record.
  template <typename T>
    requires requires { T::kId; }
  void GetTable(T &rec) {
    Open(TableTraits<T>::kTable.id.Value());
    static_cast<std::remove_cvref_t<T> *>(State().record)->Copy(rec);
    if (detail::RuntimeIsTemporary(&rec)) { detail::RuntimeAdoptTemporary(State().record, &rec); }
  }

  /// \brief AL `RecordRef.GetTable(Record)` on a record global held by handle, which is how a
  ///        codeunit's temporary globals arrive.
  /// \tparam H The handle type.
  /// \param handle The handle.
  template <typename H>
    requires requires(H &h) {
      { std::remove_cvref_t<decltype(*h)>::kId } -> std::convertible_to<TableId>;
    } && (!requires { H::kId; })
  void GetTable(H &handle) {
    GetTable(*handle);
  }

  /// \brief AL `RecordRef.SetTable(Record)` on a record global held by handle.
  /// \tparam H The handle type.
  /// \param handle The handle.
  template <typename H>
    requires requires(H &h) {
      { std::remove_cvref_t<decltype(*h)>::kId } -> std::convertible_to<TableId>;
    } && (!requires { H::kId; })
  void SetTable(H &handle) {
    SetTable(*handle);
  }

  /// \brief AL `RecordRef.GetTable(Variant)`: the record the Variant carries, or the RecordRef it
  ///        refers to, which `Find Record Management` hands over as `SourceRec: Variant`.
  /// \param held The Variant.
  /// \throws Error when it holds neither a record nor a RecordRef.
  void GetTable(const ::agiru::Variant &held) { detail::RecordRefFromVariant(*this, held); }

  /// \brief The record the reference stands on, for a caller that hands it to a report's
  ///        dataitem (`Report.Run(Number, ..., RecRef)`). \return The record, or `nullptr`
  ///        before `Open`.
  [[nodiscard]] const void *RecordPointer() const { return State().record; }

  /// \brief The declaration of the table the reference opened. \return It, or `nullptr`.
  [[nodiscard]] const TableDef *TableDefinition() const { return State().table; }

  friend void detail::RecordRefFromVariant(RecordRef &into, const ::agiru::Variant &held);

  /// \brief AL `RecordRef.GetTable(Record)` on a record whose table this build does not carry.
  /// \tparam T The stand-in for the absent table.
  /// \param rec The stand-in.
  /// \throws Error always, naming the gap rather than instantiating traits the table lacks.
  template <typename T>
    requires(!requires { T::kId; }) && (!std::same_as<std::remove_cvref_t<T>, ::agiru::Variant>) &&
            (!requires(T &h) { std::remove_cvref_t<decltype(*h)>::kId; })
  void GetTable(T &rec) {
    static_cast<void>(rec);
    throw Error("RecordRef.GetTable: the record's table is not translated in this build");
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
  [[nodiscard]] bool IsOpen() const { return State().record != nullptr; }

  /// \brief The record this reference stands for, when it is the type the caller expects.
  /// \tparam T The record's class.
  /// \return The record, or nothing when the reference is closed or names another table.
  ///
  /// \note IT IS HOW `Rec := RecordRef` COPIES. AL assigns a `RecordRef` to a `Record` in a
  ///       lookup trigger -- `OnAfterLookup(Selected: RecordRef)` is 30-odd call sites -- and what
  ///       AL copies is the ROW, which is this record's fields.
  template <typename T> [[nodiscard]] const T *As() const {
    if (State().record == nullptr || State().table != &TableTraits<T>::kTable) { return nullptr; }
    return static_cast<const T *>(State().record);
  }

  /// \brief AL `RecordRef.Number()`.
  /// \return The AL table number.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer Number() const;

  /// \brief AL `RecordRef.Name()`.
  /// \return The table's AL name.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] std::string_view Name() const;

  /// \brief AL `RecordRef.FieldCount()`.
  /// \return How many fields the table declares -- the AL declarations, and NOT the five the
  ///         platform adds.
  /// \throws Error when the RecordRef points at nothing.
  ///
  /// \warning THE SYSTEM FIELDS ARE NOT IN THE INDEX. `ApplicationAreaMgmt` walks
  ///          `FieldIndex(First) .. FieldCount()` and reads every field into a Boolean; on BC that
  ///          loop ends at the last declared field, and with `SystemId` in the count it reads a
  ///          Guid into a Boolean instead (214 UT cases, 2026-09-09). `Field(2000000000)` still
  ///          reaches a system field by number.
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
  ///
  /// \note THE PRIMARY KEY COMES FIRST: "The fields in the primary key are always listed first in
  ///       the index. Therefore, the order of the fields in the index is not necessarily the same
  ///       as the order of the fields in the table" (`recordref-fieldindex-method.md`); the rest
  ///       follow in field-number order, and the system fields are not in the list (see
  ///       `FieldCount`).
  [[nodiscard]] FieldRef FieldIndex(Integer index) const;

  /// \brief AL `RecordRef.FieldExist(FieldNo)`.
  /// \param fieldNo The AL field number.
  /// \return True when the table declares it.
  [[nodiscard]] bool FieldExist(Integer fieldNo) const;

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
  /// \return True, the way the platform answers: a partial record is a LOAD optimisation, and a
  ///         record that loads every field satisfies every read the partial one would.
  ::agiru::Boolean AddLoadFields(::agiru::Integer Fields = {}) {
    static_cast<void>(Fields);
    return true;
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
  [[nodiscard]] ::agiru::Boolean Ascending() const;

  /// \brief AL `RecordRef.Ascending(SetAscending)`: sets the sort direction of the record the
  ///        reference opened, the way `Record.Ascending(Boolean)` does. \param SetAscending Which
  ///        way. \return The direction set. \throws Error when the RecordRef is not open.
  ::agiru::Boolean Ascending(::agiru::Boolean SetAscending);

  /// \brief AL `RecordRef.Caption()` -- the caption of the table this RecordRef is open on.
  /// \return The caption, which the declaration carries as `constexpr` data.
  /// \throws Error when the RecordRef is not open.
  [[nodiscard]] std::string Caption() const { return std::string(Table().caption); }

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
  void CopyLinks(const ::agiru::RecordRef &FromRecord) {
    detail::RuntimeCopyLinks(FromRecord.RecordId(), RecordId());
  }

  /// \brief AL `RecordRef.CopyLinks(Variant)`. Copies all the links from a particular record.
  /// \param FromRecordOrRecordRef The AL `Variant`.
  void CopyLinks(const ::agiru::Variant &FromRecordOrRecordRef) {
    detail::RuntimeCopyLinks(detail::RecordIdInVariant(FromRecordOrRecordRef), RecordId());
  }

  /// \brief AL `RecordRef.Count()`. Counts the number of records that are in the filters that are
  /// currently applied to the table referred to by the RecordRef.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Count();

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
  ::agiru::Boolean Delete(::agiru::Boolean RunTrigger = {});

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
  ::agiru::Integer FilterGroup(::agiru::Integer NewGroup = {});

  /// \brief AL `RecordRef.Find(Text)`. Finds a record in a table based on the values stored in the
  /// key fields.
  /// \param Which The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Find(std::string_view Which = {});

  /// \brief AL `RecordRef.FindFirst()`. Finds the first record in a table based on the current key
  /// and filter.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindFirst();

  /// \brief AL `RecordRef.FindLast()`. Finds the last record in a table based on the current key
  /// and filter.
  /// \return True when a row was read.
  ::agiru::Boolean FindLast();

  /// \brief AL `RecordRef.FindSet(Boolean, Boolean)`. Finds a set of records in a table based on
  /// the current key and filter. FindSet can only retrieve records in ascending order.
  /// \param ForUpdate The AL `Boolean`.
  /// \param UpdateKey The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindSet(::agiru::Boolean ForUpdate, ::agiru::Boolean UpdateKey) {
    static_cast<void>(ForUpdate);
    static_cast<void>(UpdateKey);
    return FindSet();
  }

  /// \brief AL `RecordRef.FindSet(Boolean)`. Finds a set of records in a table based on the current
  /// key and filter. FINDSET can only retrieve records in ascending order.
  /// \param ForUpdate The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean FindSet(::agiru::Boolean ForUpdate) {
    static_cast<void>(ForUpdate);
    return FindSet();
  }

  /// \brief AL `RecordRef.FindSet()`. Finds a set of rows, the way `Record.FindSet` does.
  /// \return Whether a row was found.
  ::agiru::Boolean FindSet();

  /// \brief AL `RecordRef.FullyQualifiedName()`. Identifies the fully qualified name of the table.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string FullyQualifiedName() {
    throw Error("RecordRef.FullyQualifiedName() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Get(RecordId)`. Gets a record based on the ID of the record.
  /// \param RecordID The AL `RecordId`.
  /// \return The AL `Boolean`.
  /// \throws Error when the id is blank, or names a table this build does not carry.
  /// \note IT OPENS THE RECORDREF ON THE ID'S TABLE when it is not open there already, which
  ///       is what `recordref-get-method.md` describes: the id carries the table and the key.
  ::agiru::Boolean Get(::agiru::RecordId RecordID);

  /// \brief AL `RecordRef.GetBySystemId(Guid)`. Gets a record based on the ID of the record. The
  /// RecordRef must already be opened.
  /// \param SystemId The AL `Guid`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetBySystemId(::agiru::Guid SystemId) {
    static_cast<void>(SystemId);
    throw Error("RecordRef.GetBySystemId(Guid) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.GetFilters()` -- every filter standing on the record, as BC shows it.
  ///
  /// \return `<Field Caption>: <filter>` per filtered field, joined by `, ` and ordered by field
  ///         number; the empty string when nothing filters.
  ///
  /// \note THE SHAPE IS BC'S OWN AND CODE BRANCHES ON IT. A report header prints it, and
  ///       `PlanningRoutingLine.Caption` starts with `if GetFilters = '' then exit('')` -- a record
  ///       with no filter has no meaningful caption (openerp read the same call site).
  [[nodiscard]] std::string GetFilters() const;

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
  /// \param UseNames Captions when true (AL's default), `Field<no>` when false.
  /// \return `VERSION(1) SORTING(...) ORDER(...) WHERE(...)`, what `SetView` reads back.
  /// \throws Error when the RecordRef is not open.
  [[nodiscard]] std::string GetView(::agiru::Boolean UseNames = true) const;

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
  ::agiru::Boolean HasLinks() { return detail::RuntimeHasLinks(RecordId()); }

  /// \brief AL `RecordRef.Init()` -- every field to its default, the way `Record.Init` does it
  ///        (`recordref-init-method.md`: the table's InitValue, else the type's zero).
  /// \throws Error when the RecordRef is not open.
  void Init();

  /// \brief AL `RecordRef.Insert()`. Inserts a record into a table without executing the code in
  /// the OnInsert trigger.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert();

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
  ::agiru::Boolean Insert(::agiru::Boolean RunTrigger);

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
  ::agiru::Boolean IsEmpty();

  /// \brief AL `RecordRef.IsTemporary()`. Determines whether a RecordRef refers to a temporary
  /// table.
  /// \return The AL `Boolean`.
  ::agiru::Boolean IsTemporary();

  /// \brief AL `RecordRef.KeyIndex(Integer)`. Gets the KeyRef of the key that has the index
  /// specified in the table that is currently selected. The key can be composed of fields of any
  /// supported data type. Data types that are not supported include BLOBs, FlowFilters, variables,
  /// and functions. If the sorting key is set to a field that is not part of a key, then the
  /// KEYINDEX is -1.
  /// \param Index The AL `Integer`.
  /// \return The AL `KeyRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::KeyRef KeyIndex(::agiru::Integer Index) const;

  /// \brief AL `RecordRef.LoadFields(Integer)`. Accesses the table's corresponding data source and
  /// loads the values of the specified fields on the record.
  /// \param Fields The AL `Integer`.
  /// \return True, the way the platform answers: a partial record is a LOAD optimisation, and a
  ///         record that loads every field satisfies every read the partial one would.
  ::agiru::Boolean LoadFields(::agiru::Integer Fields) {
    static_cast<void>(Fields);
    return true;
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
  ::agiru::Boolean Modify(::agiru::Boolean RunTrigger = {});

  /// \brief AL `RecordRef.Next(Integer)`. Steps through a specified number of records and retrieves
  /// a record.
  /// \param Steps The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Next(::agiru::Integer Steps = {});

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
  ::agiru::IsolationLevel ReadIsolation(const ::agiru::IsolationLevel &ReadIsolation = {});

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
  ::agiru::RecordId RecordId() const;

  /// \brief AL `RecordRef.RecordLevelLocking()`. Gets a value indicating whether record level
  /// locking is enabled.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean RecordLevelLocking() {
    throw Error("RecordRef.RecordLevelLocking() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Rename(Value1: Any [, Value2: Any,...])`. Changes the value of a
  ///        primary key in a table.
  /// \tparam More The remaining key values, which AL writes as the page's own `...`.
  /// \param Value1 The AL `Any`.
  /// \param more The AL `Any` values of the remaining primary key fields.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \warning THE COUNT IS THE PRIMARY KEY'S AND NOT THIS FILE'S. `recordref-rename-method.md`
  ///          writes `Rename(Value1: Any [, Value2: Any,...])`, and a primary key names at most 16
  ///          fields (`devenv-table-keys.md`), so a fixed two refused what AL accepts.
  template <typename... More>
  ::agiru::Boolean Rename(const ::agiru::Variant &Value1, const More &...more) {
    static_cast<void>(Value1);
    (static_cast<void>(more), ...);
    throw Error("RecordRef.Rename(Any) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.Reset()`. Removes all filters, including any special filters set by the
  /// MarkedOnly method (Record), changes fields select for loading back to all, sets the read
  /// isolation level to the default value, and changes the current key to the primary key. Also
  /// removes any marks on the record and clears any AL variables defined on its table definition.
  /// \note What `record-reset-method.md` lists, and nothing else: a temporary RecordRef stays
  ///       temporary (board:0035's `RuntimeReset`).
  void Reset();

  /// \brief AL `RecordRef.SecurityFiltering(SecurityFilter)`. Gets or sets how security filters are
  /// applied to the RecordRef.
  /// \param NewSecurityFiltering The AL `SecurityFilter`.
  /// \return The AL `SecurityFilter`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `RecordRef.SecurityFiltering()` -- how security filters apply to this record.
  /// \return The mode, `Validated` for a RecordRef that never set one.
  ::agiru::SecurityFilter SecurityFiltering() const;

  /// \brief AL `RecordRef.SecurityFiltering(SecurityFilter)` -- sets how security filters apply,
  ///        carried on the record's state the way `Record.SecurityFiltering` carries it.
  /// \param NewSecurityFiltering The mode.
  /// \return The mode that was set before.
  ::agiru::SecurityFilter SecurityFiltering(const ::agiru::SecurityFilter &NewSecurityFiltering);

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
  /// \note A HINT AND NOT A CONTRACT: `recordref-setloadfields-method.md` says the fields are
  ///       loaded "initially" and any other on access, so a runtime that loads every column is
  ///       correct and only slower; the answer is true.
  ::agiru::Boolean SetLoadFields(::agiru::Integer Fields = {}) {
    static_cast<void>(Fields);
    return true;
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
  /// \note A range on each primary key field at the record's value, as `Record.SetRecFilter`.
  void SetRecFilter();

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
  /// \note THE PARAMETER IS A TEMPLATE, BECAUSE AL'S IS A `Record` AND NOT A `RecordRef`. The
  ///       generated tables are 1 609 unrelated classes with no common base a signature could name;
  ///       what they share is the `kId` every one of them declares, which is what this asks for.
  ///       A `RecordRef` parameter took none of them and made every call site a compile error.
  template <typename R>
    requires requires {
      { R::kId } -> std::convertible_to<TableId>;
    }
  void SetTable(R &Rec) {
    if (State().record == nullptr || State().table == nullptr) {
      throw Error("RecordRef.SetTable: the RecordRef is not open");
    }
    if (State().table->id != TableTraits<R>::kTable.id) {
      throw Error("RecordRef.SetTable: the RecordRef refers to " +
                  std::string(State().table->name) + " and the record is a " +
                  std::string(TableTraits<R>::kTable.name));
    }
    Rec.Copy(*static_cast<R *>(State().record));
  }

  /// \brief AL `RecordRef.SetView(Text)`. Sets the current sort order, key, and filters on a table.
  /// \param String The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `RecordRef.SetTable(Variant)` -- the record a Variant carries.
  /// \param Rec The Variant.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetTable(const ::agiru::Variant &Rec) {
    static_cast<void>(Rec);
    throw Error("RecordRef.SetTable(Variant) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `RecordRef.SetView(Text)`. Sets the sort order, direction and filters a view
  ///        string names, the `SourceTableView` form `GetView` writes.
  /// \param String The view; empty clears the filters and returns to the primary key.
  /// \throws Error when the RecordRef is not open, or the view names a field the table lacks.
  void SetView(std::string_view String);

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
  ::agiru::Integer SystemIdNo();

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
  ///
  /// \note IT LETS GO OF A RECORD IT MADE AND OF ONE IT WAS GIVEN, and only the first is freed:
  ///       `GetTable(Rec)` points a RecordRef at somebody else's record and `Open(18)` makes one.
  void Close() {
    detail::RecordRefState &box = State();
    box.record = nullptr;
    box.table = nullptr;
    box.owned.Reset();
  }

  /// \brief AL `RecordRef.KeyCount()`.
  /// \return How many keys the table declares.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer KeyCount() const;

private:
  friend class KeyRef;
  friend class FieldRef;

  RecordRef(void *record, const TableDef &table) : state_(new detail::RecordRefState{}) {
    state_->record = record;
    state_->table = &table;
  }

  [[nodiscard]] const TableDef &Table() const;

  [[nodiscard]] detail::RecordRefState &State() const {
    if (state_ == nullptr) { state_ = new detail::RecordRefState{}; }
    return *state_;
  }

  [[nodiscard]] detail::RecordRefState *Shared() const {
    detail::RecordRefState &box = State();
    ++box.uses;
    return &box;
  }

  void Release() noexcept {
    if (state_ != nullptr && --state_->uses == 0) { delete state_; }
    state_ = nullptr;
  }

  mutable detail::RecordRefState *state_ = nullptr;
};

// NOLINTEND(bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard,performance-unnecessary-value-param)

}
