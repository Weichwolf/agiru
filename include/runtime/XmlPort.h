/// \file
/// \brief AL `XMLPORT` -- the platform object reached by number, and the base of every generated
///        xmlport (board:0065).
///
/// AN XMLPORT IS A PAGE WITH A SCHEMA, the way a report is a page with a dataset: the parser reads
/// `schema` into `al::PageObject::dataset`, the generated class derives from `XmlPort<Derived>`,
/// which derives from `Page<Derived>` so the request page is served by the page machinery, and
/// the generator emits `Export_()` and `Import_()` from the element tree -- `tableelement`s as
/// record loops, `fieldelement`s and `textelement`s as values, `Unbound` elements as loops that
/// `BreakUnbound` ends. What is written and what is read goes through `XmlPortOutput` and
/// `XmlPortInput`, which know the three formats (`devenv-format-property.md`): `Xml`,
/// `VariableText` (fields between separators, records between separators) and `FixedText`.
#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/test/Handlers.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/TextEncoding.h"
#include "type/Variant.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace agiru {

/// \brief What the runtime knows about a generated xmlport: its number, its name and its
///        `XmlPortDef`.
/// \tparam T The xmlport's generated class.
template <typename T> struct XmlPortTraits;

/// \brief The `Format` property, `devenv-format-property.md`.
enum class XmlPortFormat : std::uint8_t {
  Xml,          ///< `Xml`, the default.
  VariableText, ///< `VariableText`: separated fields and records.
  FixedText,    ///< `FixedText`: fields of a declared `Width`.
};

/// \brief The `Direction` property.
enum class XmlPortDirection : std::uint8_t {
  Both,   ///< `Both`, the default.
  Import, ///< `Import`.
  Export, ///< `Export`.
};

/// \brief What an xmlport declares, `constexpr` in the generated definitions unit.
struct XmlPortDef {
  XmlPortId id{};                                 ///< The AL number.
  std::string_view name{};                        ///< The AL name.
  XmlPortFormat format = XmlPortFormat::Xml;      ///< `Format`.
  XmlPortDirection direction = XmlPortDirection::Both; ///< `Direction`.
  TextEncoding encoding = TextEncoding::MSDos;    ///< `TextEncoding`, MSDOS by default.
  std::string_view fieldSeparator = "<TAB>";      ///< `FieldSeparator`.
  std::string_view recordSeparator = "<NewLine>"; ///< `RecordSeparator`.
  std::string_view fieldDelimiter = "<None>";     ///< `FieldDelimiter`.
  std::string_view tableSeparator = "<NewLine><NewLine>"; ///< `TableSeparator`.
  bool useRequestPage = true;                     ///< `UseRequestPage`.
  bool formatEvaluateXml = false;                 ///< `FormatEvaluate = Xml`.
  std::string_view rootName{};                    ///< The root element's `XmlName`.
};

/// \brief `currXMLport.Break()` in flight: ends the current table element's loop.
struct XmlPortBreak {};

/// \brief `currXMLport.Skip()` in flight: leaves the current record out.
struct XmlPortSkip {};

/// \brief `currXMLport.Quit()` in flight: ends the port.
struct XmlPortQuit {};

/// \brief `currXMLport.BreakUnbound()` in flight: ends an `Unbound` element's loop.
struct XmlPortBreakUnbound {};

/// \brief What an export writes: the element tree for `Xml`, the lines for the text formats.
class XmlPortOutput {
public:
  /// \brief Starts a run. \param def The declaration. \param fieldSeparator The separator in
  ///        force. \param recordSeparator The record separator. \param fieldDelimiter The
  ///        delimiter. \param tableSeparator The table separator.
  void Start(const XmlPortDef &def,
             std::string_view fieldSeparator,
             std::string_view recordSeparator,
             std::string_view fieldDelimiter,
             std::string_view tableSeparator);

  /// \brief Opens a container element (a `textelement` with children). \param name Its XmlName.
  void BeginGroup(std::string_view name);

  /// \brief Closes it. \param name Its XmlName.
  void EndGroup(std::string_view name);

  /// \brief Opens a table element's record. \param name The XmlName.
  void BeginRecord(std::string_view name);

  /// \brief Closes the record: a line in the text formats. \param name The XmlName.
  void EndRecord(std::string_view name);

  /// \brief A leaf value. \param name The XmlName. \param text The value. \param attribute
  ///        Whether it is an attribute of the open element. \param width The `Width` for
  ///        `FixedText`, 0 for none.
  void Value(std::string_view name, std::string_view text, bool attribute, std::int32_t width);

  /// \brief The whole output, in the port's encoding. \return The bytes.
  [[nodiscard]] std::string Finish() const;

private:
  struct Node {
    std::string name;
    std::string text;
    bool hasText = false;
    std::vector<std::pair<std::string, std::string>> attributes;
    std::vector<Node> children;
  };
  void Serialize(const Node &node, std::string &into, int depth) const;
  Node *Open_();
  XmlPortDef def_{};
  std::string fieldSeparator_;
  std::string recordSeparator_;
  std::string fieldDelimiter_;
  std::string tableSeparator_;
  Node root_;
  std::vector<std::size_t> path_;
  std::string lines_;
  std::vector<std::string> fields_;
  int records_ = 0;
  bool inRecord_ = false;
};

/// \brief What an import reads: a cursor over the document, `Enter`ed and `Left` the way the
///        generated `Import_()` walks the schema. A text format serves the same cursor -- a
///        record level is a line, a leaf level a field -- so one walk reads all three.
class XmlPortInput {
public:
  /// \brief Loads the document. \param bytes The bytes read from the source stream.
  /// \param def The declaration. \param fieldSeparator The separator in force.
  /// \param recordSeparator The record separator. \param fieldDelimiter The delimiter.
  void Load(std::string_view bytes,
            const XmlPortDef &def,
            std::string_view fieldSeparator,
            std::string_view recordSeparator,
            std::string_view fieldDelimiter);

  /// \brief Descends into the next child named so. \param name The XmlName.
  /// \return Whether there was one.
  [[nodiscard]] bool Enter(std::string_view name);

  /// \brief Ascends.
  void Leave();

  /// \brief The current element's text. \return The text.
  [[nodiscard]] std::string Text() const;

  /// \brief An attribute of the current element. \param name Its XmlName. \return The value, empty
  ///        when absent.
  [[nodiscard]] std::string Attribute(std::string_view name) const;

private:
  struct Node {
    std::string name;
    std::string text;
    std::vector<std::pair<std::string, std::string>> attributes;
    std::vector<Node> children;
  };
  struct Level {
    const Node *node;
    std::size_t next;
  };
  void ParseXml(std::string_view text);
  void ParseLines(std::string_view text);
  XmlPortDef def_{};
  Node root_;
  std::vector<Level> levels_;
  bool textFormat_ = false;
  std::vector<std::vector<std::string>> lines_;
  std::size_t line_ = 0;
  std::size_t field_ = 0;
  int depth_ = 0;
};

namespace detail {
/// \brief The filter group a `LinkFields` narrows in, `record-filtergroup-method.md`'s "Link".
inline constexpr std::int32_t kElementLinkGroup = 4;
}

/// \brief What `Xmlport.Run/Export/Import(Number, ...)` hands the generated entry.
struct XmlPortRequest {
  bool import = false;              ///< Whether to import rather than export.
  bool requestPage = false;         ///< Whether the request page is shown.
  OutStream *destination = nullptr; ///< Where an export goes.
  InStream *source = nullptr;       ///< Where an import reads from.
  const void *record = nullptr;     ///< The `var Record` argument, or `nullptr`.
  const TableDef *table = nullptr;  ///< Its declaration.
};

/// \brief What the runtime knows about a generated xmlport: its number, its name and how to run.
struct XmlPortEntry {
  XmlPortId id;          ///< The AL number.
  std::string_view name; ///< The AL name.
  /// \brief Constructs the xmlport and runs the request. \param request The request.
  void (*run)(const XmlPortRequest &request);
};

/// \brief Puts an xmlport in the catalogue at load time. \param entry The entry, which lives on.
void RegisterXmlPortEntry(const XmlPortEntry *entry);

/// \brief Finds an xmlport by number. \param id The number. \return The entry, or `nullptr`.
[[nodiscard]] const XmlPortEntry *FindXmlPort(XmlPortId id);

namespace detail {

/// \brief The bytes of a text in an xmlport's `TextEncoding`. \param text The text, UTF-8.
/// \param encoding The encoding. \return The bytes.
[[nodiscard]] std::string EncodeForXmlPort(std::string_view text, TextEncoding encoding);

/// \brief The text of bytes read in an xmlport's `TextEncoding`. \param bytes The bytes.
/// \param encoding The encoding. \return The text, UTF-8.
[[nodiscard]] std::string DecodeForXmlPort(std::string_view bytes, TextEncoding encoding);

/// \brief Replaces `<TAB>`, `<NewLine>`, `<None>`, `<,>` and the like in a separator property.
/// \param declared The property's text. \return The characters.
[[nodiscard]] std::string SeparatorText(std::string_view declared);

/// \brief The whole of an `InStream`, read to its end. \param stream The stream. \return Bytes.
[[nodiscard]] std::string ReadWhole(InStream &stream);

/// \brief A table element's `SourceTableView`, applied in filter group 2 without losing the
///        caller's filters; the same rule as a report dataitem's view. \param record The record.
/// \param table Its declaration. \param view The view text.
void ApplyElementView(void *record, const TableDef &table, std::string_view view);

/// \brief `ApplyElementView` over a typed record. \tparam R The table. \param record The record.
/// \param view The view text.
template <typename R> void ApplyElementView(R &record, std::string_view view) {
  ApplyElementView(static_cast<void *>(&record), TableTraits<R>::kTable, view);
}

/// \brief `SetTableView(Record)` on an xmlport: the record's filters join the table element's
///        in filter group 2. \param to The element's record. \param from The caller's.
void AdoptElementView(void *to, const void *from);

}

/// \brief AL's `XMLPORT` object and the base of every generated xmlport.
///
/// \tparam Derived The xmlport's generated class, or `void` for the platform object AL spells
///         `XMLPORT`. A generated xmlport derives from this, which derives from `Page<Derived>`;
///         the generator adds `Export_()`, `Import_()` and `AdoptView_(table, record)`.
template <typename Derived = void> class XmlPort : public Page<Derived> {
public:
  /// \brief Marks an xmlport class for the test runner's handler dispatch.
  using IsXmlPort = void;

  /// \brief The order an xmlport runs its triggers.
  static constexpr std::string_view kTriggerOrder =
      "OnInitXmlPort, OnPreXmlPort, OnAfterInitRecord, OnBeforeInsertRecord, OnAfterInsertRecord, "
      "OnAfterGetRecord, OnBeforeModifyRecord, OnAfterModifyRecord, OnPreXmlItem, OnPostXmlPort, "
      "OnBeforePassField, OnAfterAssignField, OnBeforePassVariable, OnAfterAssignVariable";

  /// \brief The xmlport's number.
  [[nodiscard]] static constexpr XmlPortId Id() { return XmlPortTraits<Derived>::kId; }

  /// \brief The xmlport's name.
  [[nodiscard]] static constexpr std::string_view Name() { return XmlPortTraits<Derived>::kName; }

  /// \brief `currXMLport.Filename`, which AL assigns (`currXMLport.Filename := 'x.csv'`) and
  ///        reads. The name the client would download the export under; kept, shown nowhere.
  class FilenameSlot {
  public:
    /// \brief `Filename := Text`. \param text The name. \return This.
    FilenameSlot &operator=(std::string_view text) {
      name_ = std::string(text);
      return *this;
    }

    /// \brief `Filename := Text` from an AL text. \tparam N The length. \param text The name.
    /// \return This.
    template <std::size_t N> FilenameSlot &operator=(const ::agiru::Text<N> &text) {
      name_ = std::string(std::string_view(text));
      return *this;
    }

    /// \brief `Filename()` called. \return The name.
    [[nodiscard]] ::agiru::Text<0> operator()() const { return ::agiru::Text<0>{name_}; }

    /// \brief `Filename(Text)` called. \param text The name.
    void operator()(std::string_view text) { name_ = std::string(text); }

    /// \brief `Filename` read as a text. \return The name.
    operator ::agiru::Text<0>() const { return ::agiru::Text<0>{name_}; } // NOLINT(*-explicit-constructor)

    /// \brief `Filename` read as a string view. \return The name.
    operator std::string_view() const { return name_; } // NOLINT(*-explicit-constructor)

  private:
    std::string name_;
  };

  /// \brief `currXMLport.Filename`.
  FilenameSlot Filename;

  /// \brief `currXMLport.Break()`. \throws XmlPortBreak always.
  [[noreturn]] void Break() const { throw XmlPortBreak{}; }

  /// \brief `currXMLport.Skip()`. \throws XmlPortSkip always.
  [[noreturn]] void Skip() const { throw XmlPortSkip{}; }

  /// \brief `currXMLport.Quit()`. \throws XmlPortQuit always.
  [[noreturn]] void Quit() const { throw XmlPortQuit{}; }

  /// \brief `currXMLport.BreakUnbound()`. \throws XmlPortBreakUnbound always.
  [[noreturn]] void BreakUnbound() const { throw XmlPortBreakUnbound{}; }

  /// \brief `Xmlport.SetDestination(OutStream)`: where `Export` writes. \param Stream The stream.
  void SetDestination(OutStream &Stream) { destination_ = &Stream; }

  /// \brief `Xmlport.SetSource(InStream)`: where `Import` reads. \param Stream The stream.
  void SetSource(InStream &Stream) { source_ = &Stream; }

  /// \brief `Xmlport.SetTableView(Record)`: the record's view becomes the table element's on
  ///        that table. \tparam R The table. \param Record The record.
  /// \throws Error when no table element is on that table.
  template <typename R>
    requires requires { TableTraits<std::remove_cvref_t<R>>::kTable; }
  void SetTableView(const R &Record) {
    if (!Self_().AdoptView_(&TableTraits<std::remove_cvref_t<R>>::kTable,
                            static_cast<const void *>(&Record))) {
      throw Error("XmlPort.SetTableView: " + std::string(Name()) + " has no table element on " +
                  std::string(TableTraits<std::remove_cvref_t<R>>::kTable.name));
    }
  }

  /// \brief `Xmlport.TextEncoding()`. \return The encoding in force.
  [[nodiscard]] ::agiru::TextEncoding TextEncoding() const { return encoding_; }

  /// \brief `Xmlport.TextEncoding(Encoding)`. \param Encoding The encoding for this run.
  void TextEncoding(::agiru::TextEncoding Encoding) { encoding_ = Encoding; }

  /// \brief `Xmlport.FieldSeparator()`. \return The separator, as declared or set.
  [[nodiscard]] ::agiru::Text<0> FieldSeparator() const { return ::agiru::Text<0>{fieldSeparator_}; }

  /// \brief `Xmlport.FieldSeparator(Text)`. \param Separator The separator for this run.
  void FieldSeparator(std::string_view Separator) { fieldSeparator_ = std::string(Separator); }

  /// \brief `Xmlport.FieldSeparator(Text)` with one character, as `CRLF[1]` arrives. \param Separator The character.
  void FieldSeparator(Char Separator) { fieldSeparator_ = Encoded(Separator); }

  /// \brief `Xmlport.FieldSeparator(Text)` from a literal, which is neither a `Char` nor a view yet.
  /// \param Separator The text.
  void FieldSeparator(const char *Separator) { fieldSeparator_ = std::string(Separator); }

  /// \brief `Xmlport.RecordSeparator()`. \return The separator.
  [[nodiscard]] ::agiru::Text<0> RecordSeparator() const {
    return ::agiru::Text<0>{recordSeparator_};
  }

  /// \brief `Xmlport.RecordSeparator(Text)`. \param Separator The separator for this run.
  void RecordSeparator(std::string_view Separator) { recordSeparator_ = std::string(Separator); }

  /// \brief `Xmlport.RecordSeparator(Text)` with one character, as `CRLF[1]` arrives. \param Separator The character.
  void RecordSeparator(Char Separator) { recordSeparator_ = Encoded(Separator); }

  /// \brief `Xmlport.RecordSeparator(Text)` from a literal, which is neither a `Char` nor a view yet.
  /// \param Separator The text.
  void RecordSeparator(const char *Separator) { recordSeparator_ = std::string(Separator); }

  /// \brief `Xmlport.FieldDelimiter()`. \return The delimiter.
  [[nodiscard]] ::agiru::Text<0> FieldDelimiter() const { return ::agiru::Text<0>{fieldDelimiter_}; }

  /// \brief `Xmlport.FieldDelimiter(Text)`. \param Delimiter The delimiter for this run.
  void FieldDelimiter(std::string_view Delimiter) { fieldDelimiter_ = std::string(Delimiter); }

  /// \brief `Xmlport.TableSeparator()`. \return The separator.
  [[nodiscard]] ::agiru::Text<0> TableSeparator() const { return ::agiru::Text<0>{tableSeparator_}; }

  /// \brief `Xmlport.TableSeparator(Text)`. \param Separator The separator for this run.
  void TableSeparator(std::string_view Separator) { tableSeparator_ = std::string(Separator); }

  /// \brief `Xmlport.TableSeparator(Text)` with one character, as `CRLF[1]` arrives. \param Separator The character.
  void TableSeparator(Char Separator) { tableSeparator_ = Encoded(Separator); }

  /// \brief `Xmlport.TableSeparator(Text)` from a literal, which is neither a `Char` nor a view yet.
  /// \param Separator The text.
  void TableSeparator(const char *Separator) { tableSeparator_ = std::string(Separator); }

  /// \brief `Xmlport.Export()`: the schema walked over the source records into the
  ///        destination stream. \return `true`. \throws Error without a destination.
  Boolean Export() {
    Execute_(XmlPortRequest{.import = false, .requestPage = false});
    return true;
  }

  /// \brief `Xmlport.Import()`: the source stream read into the table elements' records.
  /// \return `true`. \throws Error without a source.
  Boolean Import() {
    Execute_(XmlPortRequest{.import = true, .requestPage = false});
    return true;
  }

  /// \brief `Xmlport.Run()`: the request page when declared, then `Export` or `Import` as the
  ///        `Direction` says.
  void Run() {
    Execute_(XmlPortRequest{.import = XmlPortTraits<Derived>::kPort.direction ==
                                      XmlPortDirection::Import,
                            .requestPage = UseRequestPage_()});
  }

  /// \brief `currXMLport.ImportFile`, which AL assigns (`ImportFile := false`) and tests (`if
  ///        currXMLport.ImportFile then`): whether the source is a file the client uploads. It
  ///        is a flag here -- the stream the caller set is what is read either way.
  class ImportFileSlot {
  public:
    /// \brief `ImportFile := Boolean`. \param on The value. \return This.
    ImportFileSlot &operator=(Boolean on) {
      on_ = on;
      return *this;
    }

    /// \brief `ImportFile()` read. \return The value.
    [[nodiscard]] Boolean operator()() const { return on_; }

    /// \brief `ImportFile(false)`, the generator's spelling of `ImportFile := false`.
    /// \param on The value. \return The value.
    Boolean operator()(Boolean on) {
      on_ = on;
      return on_;
    }

    /// \brief `ImportFile(FileName)`: an import from a file. \param FileName The file.
    /// \return Never. \throws Error always: a file source waits for `File` (board:0065).
    Boolean operator()(std::string_view FileName) const {
      throw Error("XmlPort.ImportFile(" + std::string(FileName) +
                  "): a file source waits for File (board:0065)");
    }

    /// \brief `if currXMLport.ImportFile then`. \return The value.
    operator Boolean() const { return on_; } // NOLINT(*-explicit-constructor)

  private:
    bool on_ = false;
  };

  /// \brief `currXMLport.ImportFile`.
  ImportFileSlot ImportFile;

  /// \brief `Xmlport.CurrentPath()`. \return The path of the element being read, empty here.
  [[nodiscard]] ::agiru::Text<0> CurrentPath() const { return {}; }

  /// \brief The output the generated `Export_()` writes into.
  [[nodiscard]] XmlPortOutput &Out_() { return output_; }

  /// \brief The input the generated `Import_()` reads from.
  [[nodiscard]] XmlPortInput &In_() { return input_; }

  /// \brief Whether `FormatEvaluate = Xml`, which formats a field with `Format(Value, 0, 9)`.
  [[nodiscard]] static constexpr bool FormatsAsXml_() {
    return XmlPortTraits<Derived>::kPort.formatEvaluateXml;
  }

  /// \brief Runs a request; the catalogue entry calls this. \param request The request.
  void Execute_(const XmlPortRequest &request) {
    Derived &self = Self_();
    if (request.record != nullptr && request.table != nullptr) {
      static_cast<void>(self.AdoptView_(request.table, request.record));
    }
    if (request.destination != nullptr) { destination_ = request.destination; }
    if (request.source != nullptr) { source_ = request.source; }
    const XmlPortDef &def = XmlPortTraits<Derived>::kPort;
    try {
      if constexpr (requires { self.OnInitXmlPort(); }) { self.OnInitXmlPort(); }
      if (request.requestPage) {
        detail::OpenPage(self, true, false);
        const TestHandler *handler =
            HandlerTable::For(HandlerKind::RequestPage, Id().Value());
        if (handler == nullptr) {
          throw Error("Unhandled UI: RequestPage " + std::string(Name()));
        }
        this->CloseWith(::agiru::Action::None);
        handler->invoke(Name(), &self);
        HandlerTable::Ran(*handler);
        detail::ClosePage(self);
        if (this->ClosedWith() != ::agiru::Action::OK) { return; }
      }
      if constexpr (requires { self.OnPreXmlPort(); }) { self.OnPreXmlPort(); }
      if (request.import) {
        if (source_ == nullptr) {
          throw Error("XmlPort.Import: " + std::string(Name()) + " has no source stream");
        }
        input_.Load(detail::DecodeForXmlPort(detail::ReadWhole(*source_), encoding_),
                    def,
                    fieldSeparator_,
                    recordSeparator_,
                    fieldDelimiter_);
        self.Import_();
      } else {
        output_.Start(def, fieldSeparator_, recordSeparator_, fieldDelimiter_, tableSeparator_);
        self.Export_();
      }
      if constexpr (requires { self.OnPostXmlPort(); }) { self.OnPostXmlPort(); }
    } catch (const XmlPortQuit &) { return; }
    if (!request.import) {
      if (destination_ == nullptr) {
        throw Error("XmlPort.Export: " + std::string(Name()) + " has no destination stream");
      }
      static_cast<void>(
          destination_->WriteBytes(detail::EncodeForXmlPort(output_.Finish(), encoding_)));
    }
  }

  /// \brief What the class declares for its separators, read once into the run's own copies.
  XmlPort()
    requires(!std::is_void_v<Derived>)
      : encoding_(XmlPortTraits<Derived>::kPort.encoding),
        fieldSeparator_(detail::SeparatorText(XmlPortTraits<Derived>::kPort.fieldSeparator)),
        recordSeparator_(detail::SeparatorText(XmlPortTraits<Derived>::kPort.recordSeparator)),
        fieldDelimiter_(detail::SeparatorText(XmlPortTraits<Derived>::kPort.fieldDelimiter)),
        tableSeparator_(detail::SeparatorText(XmlPortTraits<Derived>::kPort.tableSeparator)) {}

private:
  [[nodiscard]] Derived &Self_() { return static_cast<Derived &>(*this); }

  [[nodiscard]] static constexpr bool UseRequestPage_() {
    return XmlPortTraits<Derived>::kPort.useRequestPage;
  }

  ::agiru::TextEncoding encoding_ = ::agiru::TextEncoding::MSDos;
  std::string fieldSeparator_;
  std::string recordSeparator_;
  std::string fieldDelimiter_;
  std::string tableSeparator_;
  OutStream *destination_ = nullptr;
  InStream *source_ = nullptr;
  XmlPortOutput output_;
  XmlPortInput input_;
};

/// \brief Runs an xmlport of the catalogue. \tparam X The generated xmlport. \param request What.
template <typename X> void RunXmlPortEntry(const XmlPortRequest &request) {
  auto port = std::make_unique<X>();
  port->Execute_(request);
}

/// \brief The catalogue entry of a generated xmlport. \tparam X The generated xmlport.
template <typename X>
inline const XmlPortEntry kXmlPortEntry{
    .id = XmlPortTraits<X>::kId, .name = XmlPortTraits<X>::kName, .run = &RunXmlPortEntry<X>};

/// \brief Puts a generated xmlport in the catalogue by existing. \tparam X The xmlport.
template <typename X> struct RegisterXmlPort {
  RegisterXmlPort() { RegisterXmlPortEntry(&kXmlPortEntry<X>); }

  RegisterXmlPort(const RegisterXmlPort &) = delete;
  RegisterXmlPort(RegisterXmlPort &&) = delete;
  RegisterXmlPort &operator=(const RegisterXmlPort &) = delete;
  RegisterXmlPort &operator=(RegisterXmlPort &&) = delete;
  ~RegisterXmlPort() = default;
};

namespace detail {

/// \brief Reads a `var Record` argument into an xmlport request. \param request The request.
/// \param argument One argument.
template <typename A> void TakeXmlPortArgument(XmlPortRequest &request, const A &argument) {
  using V = std::remove_cvref_t<A>;
  if constexpr (requires { TableTraits<V>::kTable; }) {
    request.record = static_cast<const void *>(&argument);
    request.table = &TableTraits<V>::kTable;
  } else if constexpr (requires {
                         argument.operator->();
                         TableTraits<std::remove_cvref_t<decltype(*argument.operator->())>>::kTable;
                       }) {
    request.record = static_cast<const void *>(argument.operator->());
    request.table = &TableTraits<std::remove_cvref_t<decltype(*argument.operator->())>>::kTable;
  } else if constexpr (std::same_as<V, RecordRef>) {
    request.record = argument.RecordPointer();
    request.table = argument.TableDefinition();
  } else {
    static_cast<void>(argument);
  }
}

/// \brief Runs an xmlport by number or refuses. \param what The AL method. \param id The number.
/// \param request The request. \throws Error when the build carries no such xmlport.
inline void RunXmlPortByNumber(std::string_view what, ::agiru::Integer id, XmlPortRequest &request) {
  const XmlPortEntry *entry = FindXmlPort(XmlPortId{id});
  if (entry == nullptr) {
    throw Error("Xmlport." + std::string(what) + "(" + std::to_string(id) +
                "): this build carries no xmlport of that number (board:0065)");
  }
  entry->run(request);
}

}

/// \brief The platform object AL spells `XMLPORT`: the static methods by number.
template <> class XmlPort<void> {
public:
  /// \brief `Xmlport.Export(Number, var OutStream [, var Record])`. \param Number The number.
  /// \param Stream Where it goes. \param arguments The record, if any. \return `true`.
  template <typename... Arguments>
  static Boolean Export(::agiru::Integer Number, OutStream &Stream, const Arguments &...arguments) {
    XmlPortRequest request{.import = false, .destination = &Stream};
    (detail::TakeXmlPortArgument(request, arguments), ...);
    detail::RunXmlPortByNumber("Export", Number, request);
    return true;
  }

  /// \brief `Xmlport.Import(Number, var InStream [, var Record])`. \param Number The number.
  /// \param Stream Where it reads. \param arguments The record, if any. \return `true`.
  template <typename... Arguments>
  static Boolean Import(::agiru::Integer Number, InStream &Stream, const Arguments &...arguments) {
    XmlPortRequest request{.import = true, .source = &Stream};
    (detail::TakeXmlPortArgument(request, arguments), ...);
    detail::RunXmlPortByNumber("Import", Number, request);
    return true;
  }

  /// \brief `Xmlport.Run(Number [, RequestWindow] [, Import] [, var Record])`.
  /// \param Number The number. \param RequestWindow Whether the request page is shown.
  /// \param Import Whether to import. \param arguments The record, if any.
  template <typename... Arguments>
  static void Run(::agiru::Integer Number,
                  Boolean RequestWindow = false,
                  Boolean Import = false,
                  const Arguments &...arguments) {
    XmlPortRequest request{.import = static_cast<bool>(Import),
                           .requestPage = static_cast<bool>(RequestWindow)};
    (detail::TakeXmlPortArgument(request, arguments), ...);
    detail::RunXmlPortByNumber("Run", Number, request);
  }
};

}
