#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

/// \file
/// \brief AL `Variant` -- one value, of whichever AL type it was given.

/// \brief Whether a type is one of a variant's alternatives, exactly.
namespace agiru::detail {

/// \brief Whether a type is one of a variant's alternatives.
/// \tparam T The type.
/// \tparam V The variant.
template <typename T, typename V> struct InVariant : std::false_type {};

/// \brief Whether a type is one of a variant's alternatives.
/// \tparam T  The type.
/// \tparam Ts The alternatives.
template <typename T, typename... Ts>
struct InVariant<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

}

// NOLINTBEGIN(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard)
namespace agiru {

/// \brief What a Variant holding a RECORD holds.
///
/// \note A HANDLE AND NOT A COPY, which is what AL does. A Variant alternative that stored the
///       record by value would need 1 609 alternatives and would copy a 240-field row every time a
///       message was assembled; AL's Variant refers to the record and reads it back through
///       `RecordRef.GetTable`. The table number travels with the pointer so the reader can refuse
///       the wrong table by NUMBER rather than by a cast that cannot fail.
///
/// \warning IT DOES NOT OWN THE RECORD. The Variant is valid while the record it was made from is,
///          which is AL's own lifetime and is why `LibraryVariableStorage.Enqueue(Rec)` inside a
///          procedure whose record is local is a defect in the AL and not here.
struct RecordInVariant {
  const void *record; ///< The record.
  TableId table;      ///< Which table it is, so a reader can refuse the wrong one.

  /// \brief Two handles are equal when they refer to the same row of the same table.
  /// \param o The other.
  /// \return Whether they do.
  [[nodiscard]] bool operator==(const RecordInVariant &o) const {
    return record == o.record && table.Value() == o.table.Value();
  }
};

/// \brief AL `Variant`.
///
/// From `variant-data-type.md`: "Represents an AL variable object. The AL variant data type can
/// contain many AL data types."
///
/// \note IT ANSWERS WHAT IT HOLDS AND NEVER CONVERTS. The page gives some sixty `IsX()` predicates
///       and no conversions, because a Variant is how AL passes a value whose type the callee
///       decides on. A `Get<T>()` that coerced would turn "this is not a Date" into a plausible
///       wrong Date, which is the class of defect this whole tree is built to move to compile time.
///
/// \note THE OBJECT TYPES ARE NOT IN IT YET -- Record, RecordRef, InStream, DotNet and the rest.
///       They are not values, they are handles, and each needs its own type in the runtime first.
///       A Variant handed one refuses rather than holding a scalar that looks like it.
class Variant {
public:
  /// \brief What a Variant can hold today. An empty Variant holds the first alternative.
  using Held = std::variant<std::monostate,
                            Boolean,
                            Integer,
                            BigInteger,
                            Decimal,
                            std::string,
                            Date,
                            Time,
                            DateTime,
                            Duration,
                            Guid,
                            RecordId,
                            DateFormula,
                            RecordInVariant>;

  /// \brief An empty Variant, which is what an unassigned one holds.
  Variant() = default;

  /// \brief Holds a value.
  /// \tparam T The AL type, which must be one of the alternatives EXACTLY.
  /// \param value The value.
  ///
  /// \note Exactly, and not merely convertible: a Duration is built from a number, so a
  ///       constructibility test would make `Variant{5}` ambiguous between Integer, BigInteger and
  ///       Duration. The type the caller wrote is the type the Variant holds.
  template <typename T>
    requires detail::InVariant<T, Held>::value
  Variant(T value) : held_(std::move(value)) {}

  /// \brief Holds a record.
  ///
  /// \tparam R The generated table class, recognised by the `kId` every one of them declares.
  /// \param record The record, which the Variant refers to and does not copy.
  template <typename R>
    requires requires {
      { R::kId } -> std::convertible_to<TableId>;
    }
  Variant(const R &record) : held_(RecordInVariant{.record = &record, .table = R::kId}) {}

  /// \brief The record this Variant refers to.
  ///
  /// \tparam R The generated table class expected.
  /// \return The record.
  /// \throws Error when the Variant holds no record, or one of another table -- refused by NUMBER,
  ///         so the message names both tables instead of a cast quietly succeeding.
  template <typename R> [[nodiscard]] const R &AsRecord() const {
    const auto *held = std::get_if<RecordInVariant>(&held_);
    if (held == nullptr) { throw Error("this Variant holds no record"); }
    if (held->table.Value() != R::kId.Value()) {
      throw Error("this Variant holds table " + std::to_string(held->table.Value()) +
                  " and table " + std::to_string(R::kId.Value()) + " was asked for");
    }
    return *static_cast<const R *>(held->record);
  }

  /// \brief Holds anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  ///
  /// \note ONE STEP AND NOT TWO. A literal, a `std::string`, a `Text` and a `Code` all reach
  ///       `std::string_view`, and C++ allows only ONE user-defined conversion on the way to a
  ///       parameter -- so without this, `Assert.AreEqual(0, X, '')` and every `Any` parameter
  ///       handed a Code would fail to compile.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view> &&
             (!detail::InVariant<T, Held>::value)
  Variant(const T &value) : held_(std::string(std::string_view(value))) {}

  /// \brief AL `Variant.IsEmpty()` -- whether nothing was ever assigned.
  /// \return True when the Variant holds no value.
  [[nodiscard]] bool IsEmpty() const { return std::holds_alternative<std::monostate>(held_); }

  /// \brief Whether the Variant holds a given AL type.
  /// \tparam T The AL type.
  /// \return True when it does.
  ///
  /// The sixty `IsX()` predicates the page lists are this one question with the type spelled into
  /// the name. They are written out below for the types that exist, because AL code calls them by
  /// those names and a reader looking for `IsDate` must find `IsDate`.
  template <typename T> [[nodiscard]] bool Is() const { return std::holds_alternative<T>(held_); }

  /// \brief AL `Variant.IsBoolean()`. \return True when it holds one.
  [[nodiscard]] bool IsBoolean() const { return Is<Boolean>(); }

  /// \brief AL `Variant.IsInteger()`. \return True when it holds one.
  [[nodiscard]] bool IsInteger() const { return Is<Integer>(); }

  /// \brief AL `Variant.IsBigInteger()`. \return True when it holds one.
  [[nodiscard]] bool IsBigInteger() const { return Is<BigInteger>(); }

  /// \brief AL `Variant.IsDecimal()`. \return True when it holds one.
  [[nodiscard]] bool IsDecimal() const { return Is<Decimal>(); }

  /// \brief AL `Variant.IsText()`. \return True when it holds one.
  [[nodiscard]] bool IsText() const { return Is<std::string>(); }

  /// \brief AL `Variant.IsCode()`. \return True when it holds one.
  /// \note A Code and a Text are one alternative here, because AL's Code IS a Text with a
  ///       normalisation rule, and a Variant carries the VALUE rather than the rule.
  [[nodiscard]] bool IsCode() const { return Is<std::string>(); }

  /// \brief AL `Variant.IsDate()`. \return True when it holds one.
  [[nodiscard]] bool IsDate() const { return Is<Date>(); }

  /// \brief AL `Variant.IsTime()`. \return True when it holds one.
  [[nodiscard]] bool IsTime() const { return Is<Time>(); }

  /// \brief AL `Variant.IsDateTime()`. \return True when it holds one.
  [[nodiscard]] bool IsDateTime() const { return Is<DateTime>(); }

  /// \brief AL `Variant.IsDuration()`. \return True when it holds one.
  [[nodiscard]] bool IsDuration() const { return Is<Duration>(); }

  /// \brief AL `Variant.IsGuid()`. \return True when it holds one.
  [[nodiscard]] bool IsGuid() const { return Is<Guid>(); }

  /// \brief AL `Variant.IsRecordId()`. \return True when it holds one.
  [[nodiscard]] bool IsRecordId() const { return Is<RecordId>(); }

  /// \brief AL `Variant.IsDateFormula()`. \return True when it holds one.
  [[nodiscard]] bool IsDateFormula() const { return Is<DateFormula>(); }

  /// \brief AL `Variant.IsAction()`. Indicates whether an AL variant contains an Action variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsAction() const {
    throw Error("Variant.IsAction() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsAutomation()`. Indicates whether an AL variant contains an Automation
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsAutomation() const {
    throw Error("Variant.IsAutomation() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsBinary()`. Indicates whether an AL variant contains a Binary variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsBinary() const {
    throw Error("Variant.IsBinary() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsByte()`. Indicates whether an AL variant contains a Byte data type
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsByte() const {
    throw Error("Variant.IsByte() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsChar()`. Indicates whether an AL variant contains a Char variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsChar() const {
    throw Error("Variant.IsChar() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsClientType()`. Indicates whether an AL variant contains a ClientType
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsClientType() const {
    throw Error("Variant.IsClientType() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsCodeunit()`. Indicates whether an AL variant contains a Codeunit
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsCodeunit() const {
    throw Error("Variant.IsCodeunit() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsDataClassification()`. Indicates whether an AL variant contains a
  /// DataClassification variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsDataClassification() const {
    throw Error("Variant.IsDataClassification() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsDataClassificationType()`. Indicates whether an AL variant contains a
  /// DataClassification variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsDataClassificationType() const {
    throw Error(
        "Variant.IsDataClassificationType() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsDefaultLayout()`. Indicates whether an AL variant contains a
  /// DefaultLayout variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsDefaultLayout() const {
    throw Error("Variant.IsDefaultLayout() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsDictionary()`. Indicates whether an AL variant contains a Dictionary
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsDictionary() const {
    throw Error("Variant.IsDictionary() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsDotNet()`. Indicates whether an AL variant contains a DotNet variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsDotNet() const {
    throw Error("Variant.IsDotNet() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsExecutionMode()`. Indicates whether an AL variant contains an
  /// ExecutionMode variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsExecutionMode() const {
    throw Error("Variant.IsExecutionMode() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsFieldRef()`. Indicates whether an AL variant contains a FieldRef
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsFieldRef() const {
    throw Error("Variant.IsFieldRef() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsFile()`. Indicates whether an AL variant contains a File variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsFile() const {
    throw Error("Variant.IsFile() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsFilterPageBuilder()`. Indicates whether an AL variant contains a
  /// FilterPageBuilder variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsFilterPageBuilder() const {
    throw Error("Variant.IsFilterPageBuilder() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsInStream()`. Indicates whether an AL variant contains an InStream
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsInStream() const {
    throw Error("Variant.IsInStream() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsJsonArray()`. Indicates whether an AL variant contains a JsonArray
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsJsonArray() const {
    throw Error("Variant.IsJsonArray() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsJsonObject()`. Indicates whether an AL variant contains a JsonObject
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsJsonObject() const {
    throw Error("Variant.IsJsonObject() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsJsonToken()`. Indicates whether an AL variant contains a JsonToken
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsJsonToken() const {
    throw Error("Variant.IsJsonToken() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsJsonValue()`. Indicates whether an AL variant contains a JsonValue
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsJsonValue() const {
    throw Error("Variant.IsJsonValue() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsList()`. Indicates whether an AL variant contains a List variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsList() const {
    throw Error("Variant.IsList() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsNotification()`. Indicates whether an AL variant contains a Notification
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsNotification() const {
    throw Error("Variant.IsNotification() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsObjectType()`. Indicates whether an AL variant contains an ObjectType
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsObjectType() const {
    throw Error("Variant.IsObjectType() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsOption()`. Indicates whether an AL variant contains an Option variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsOption() const {
    throw Error("Variant.IsOption() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsOutStream()`. Indicates whether an AL variant contains an OutStream
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsOutStream() const {
    throw Error("Variant.IsOutStream() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsPromptMode()`. Indicates whether an AL variant contains a PromptMode
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsPromptMode() const {
    throw Error("Variant.IsPromptMode() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsRecord()`. Indicates whether an AL variant contains a Record variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsRecord() const { return std::holds_alternative<RecordInVariant>(held_); }

  /// \brief AL `Variant.IsRecordRef()`. Indicates whether an AL variant contains a RecordRef
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsRecordRef() const {
    throw Error("Variant.IsRecordRef() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsReportFormat()`. Indicates whether an AL variant contains a ReportFormat
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsReportFormat() const {
    throw Error("Variant.IsReportFormat() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsSecurityFiltering()`. Indicates whether an AL variant contains a
  /// SecurityFiltering variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsSecurityFiltering() const {
    throw Error("Variant.IsSecurityFiltering() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsTableConnectionType()`. Indicates whether an AL variant contains a
  /// TableConnectionType variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTableConnectionType() const {
    throw Error("Variant.IsTableConnectionType() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsTestPermissions()`. Indicates whether an AL variant contains a
  /// TestPermissions variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTestPermissions() const {
    throw Error("Variant.IsTestPermissions() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsTextBuilder()`. Indicates whether an AL variant contains a TextBuilder
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTextBuilder() const {
    throw Error("Variant.IsTextBuilder() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsTextConstant()`. Indicates whether an AL variant contains a Text
  /// constant.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTextConstant() const {
    throw Error("Variant.IsTextConstant() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsTextEncoding()`. Indicates whether an AL variant contains a TextEncoding
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTextEncoding() const {
    throw Error("Variant.IsTextEncoding() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsTransactionType()`. Indicates whether an AL variant contains a
  /// TransactionType variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsTransactionType() const {
    throw Error("Variant.IsTransactionType() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsWideChar()`. Indicates whether an AL variant contains a WideChar
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsWideChar() const {
    throw Error("Variant.IsWideChar() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlAttribute()`. Indicates whether an AL variant contains an XmlAttribute
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlAttribute() const {
    throw Error("Variant.IsXmlAttribute() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlAttributeCollection()`. Indicates whether an AL variant contains an
  /// XmlAttributeCollection variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlAttributeCollection() const {
    throw Error(
        "Variant.IsXmlAttributeCollection() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlCData()`. Indicates whether an AL variant contains an XmlCData
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlCData() const {
    throw Error("Variant.IsXmlCData() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlComment()`. Indicates whether an AL variant contains an XmlComment
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlComment() const {
    throw Error("Variant.IsXmlComment() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlDeclaration()`. Indicates whether an AL variant contains an
  /// XmlDeclaration variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlDeclaration() const {
    throw Error("Variant.IsXmlDeclaration() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlDocument()`. Indicates whether an AL variant contains an XmlDocument
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlDocument() const {
    throw Error("Variant.IsXmlDocument() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlDocumentType()`. Indicates whether an AL variant contains an
  /// XmlDocumentType variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlDocumentType() const {
    throw Error("Variant.IsXmlDocumentType() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlElement()`. Indicates whether an AL variant contains an XmlElement
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlElement() const {
    throw Error("Variant.IsXmlElement() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlNamespaceManager()`. Indicates whether an AL variant contains an
  /// XmlNamespaceManager variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlNamespaceManager() const {
    throw Error("Variant.IsXmlNamespaceManager() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlNameTable()`. Indicates whether an AL variant contains an XmlNameTable
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlNameTable() const {
    throw Error("Variant.IsXmlNameTable() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlNode()`. Indicates whether an AL variant contains an XmlNode variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlNode() const {
    throw Error("Variant.IsXmlNode() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlNodeList()`. Indicates whether an AL variant contains an XmlNodeList
  /// variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlNodeList() const {
    throw Error("Variant.IsXmlNodeList() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlProcessingInstruction()`. Indicates whether an AL variant contains an
  /// XmlProcessingInstruction variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlProcessingInstruction() const {
    throw Error(
        "Variant.IsXmlProcessingInstruction() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlReadOptions()`. Indicates whether an AL variant contains an
  /// XmlReadOptions variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlReadOptions() const {
    throw Error("Variant.IsXmlReadOptions() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlText()`. Indicates whether an AL variant contains an XmlText variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlText() const {
    throw Error("Variant.IsXmlText() is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Variant.IsXmlWriteOptions()`. Indicates whether an AL variant contains an
  /// XmlWriteOptions variable.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlWriteOptions() const {
    throw Error("Variant.IsXmlWriteOptions() is declared and not implemented yet (board:0035)");
  }

  /// \brief The value, if the Variant holds that type.
  ///
  /// \tparam T The AL type.
  /// \return The value.
  /// \throws Error when the Variant holds something else, naming both types.
  ///
  /// \warning IT DOES NOT CONVERT. AL assigns a Variant to a typed variable and the platform
  ///          refuses a mismatch; a `Get<Date>()` that read an Integer as a day number would turn a
  ///          type error into a wrong date, silently.
  template <typename T> [[nodiscard]] const T &Get() const {
    const T *value = std::get_if<T>(&held_);
    if (value == nullptr) { Refuse(); }
    return *value;
  }

  /// \brief Reads as text, when that is what it holds.
  ///
  /// \return The stored text.
  ///
  /// \throws Error when the Variant holds something else.
  ///
  /// \note AL HANDS AN `Any` TO A `Text` PARAMETER AND THE PLATFORM UNWRAPS IT -- `Assert.AreEqual`
  ///       does it on every call. The conversion is implicit because AL's is, and it RAISES on a
  ///       mismatch because AL's does: what it must not do is hand back an empty string for an
  ///       Integer, which is the wrong answer wearing the right type.
  operator std::string_view() const {
    const std::string *text = std::get_if<std::string>(&held_);
    if (text == nullptr) { Refuse(); }
    return *text;
  }

  /// \brief Reads as one of its alternatives, when that is what it holds.
  ///
  /// \tparam T The AL type wanted.
  /// \return The value.
  /// \throws Error when the Variant holds something else.
  ///
  /// \note THE SAME UNWRAP THE TEXT ONE DOES, and for the same reason: AL hands an `Any` to a typed
  ///       parameter and the platform unwraps it, raising on a mismatch. What it must not do is
  ///       hand back a zero for a Text, which is the wrong answer wearing the right type.
  template <typename T>
    requires detail::InVariant<T, Held>::value
  operator T() const {
    const T *value = std::get_if<T>(&held_);
    if (value != nullptr) { return *value; }
    if constexpr (std::is_same_v<T, Decimal>) {
      if (const Integer *narrow = std::get_if<Integer>(&held_); narrow != nullptr) {
        return Decimal{*narrow};
      }
      if (const BigInteger *wide = std::get_if<BigInteger>(&held_); wide != nullptr) {
        return Decimal{*wide};
      }
    }
    if constexpr (std::is_same_v<T, BigInteger>) {
      if (const Integer *narrow = std::get_if<Integer>(&held_); narrow != nullptr) {
        return BigInteger{*narrow};
      }
    }
    Refuse();
  }

  /// \brief Compares two Variants.
  /// \param o The other.
  /// \return True when they hold the same type and the same value.
  [[nodiscard]] bool operator==(const Variant &o) const = default;

private:
  [[noreturn]] static void Refuse();

  Held held_;
};

// NOLINTEND(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,readability-magic-numbers,modernize-use-nodiscard)

}
