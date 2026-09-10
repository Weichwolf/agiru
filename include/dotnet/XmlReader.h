#pragma once

#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `XmlNodeType`, held by number -- the numbers libxml2's reader answers are .NET's
///        own (`Element` 1, `Attribute` 2, `Text` 3, `CDATA` 4, `ProcessingInstruction` 7,
///        `Comment` 8, `Document` 9, `Whitespace` 13, `EndElement` 15, `XmlDeclaration` 17).
class XmlNodeType {
public:
  /// \brief `None`.
  XmlNodeType() = default;

  /// \brief A node type by number. \param number The number.
  explicit XmlNodeType(std::int32_t number) : number_(number) {}

  /// \brief `XmlNodeType.Equals(other)`. \param other The other. \return Whether the same.
  [[nodiscard]] Boolean Equals(const XmlNodeType &other) const { return number_ == other.number_; }

  /// \brief The number. \return It.
  [[nodiscard]] std::int32_t Number() const { return number_; }

  [[nodiscard]] static XmlNodeType None() { return XmlNodeType{0}; }                    ///< `None`.
  [[nodiscard]] static XmlNodeType Element() { return XmlNodeType{1}; }                 ///< `Element`.
  [[nodiscard]] static XmlNodeType Attribute() { return XmlNodeType{2}; }               ///< `Attribute`.
  [[nodiscard]] static XmlNodeType Text() { return XmlNodeType{3}; }                    ///< `Text`.
  [[nodiscard]] static XmlNodeType CDATA() { return XmlNodeType{4}; }                   ///< `CDATA`.
  [[nodiscard]] static XmlNodeType ProcessingInstruction() { return XmlNodeType{7}; }   ///< `ProcessingInstruction`.
  [[nodiscard]] static XmlNodeType Comment() { return XmlNodeType{8}; }                 ///< `Comment`.
  [[nodiscard]] static XmlNodeType Document() { return XmlNodeType{9}; }                ///< `Document`.
  [[nodiscard]] static XmlNodeType Whitespace() { return XmlNodeType{13}; }             ///< `Whitespace`.
  [[nodiscard]] static XmlNodeType SignificantWhitespace() { return XmlNodeType{14}; }  ///< `SignificantWhitespace`.
  [[nodiscard]] static XmlNodeType EndElement() { return XmlNodeType{15}; }             ///< `EndElement`.
  [[nodiscard]] static XmlNodeType XmlDeclaration() { return XmlNodeType{17}; }         ///< `XmlDeclaration`.

private:
  std::int32_t number_ = 0;
};

/// \brief .NET `DtdProcessing`: what a reader does with a DTD. Only `Ignore` and `Prohibit` are
///        named by the BaseApp, and this reader never resolves one.
class DtdProcessing {
public:
  /// \brief `Prohibit`, the .NET default.
  DtdProcessing() = default;

  [[nodiscard]] static DtdProcessing Prohibit() { return DtdProcessing{0}; } ///< `Prohibit`.
  [[nodiscard]] static DtdProcessing Ignore() { return DtdProcessing{1}; }   ///< `Ignore`.
  [[nodiscard]] static DtdProcessing Parse() { return DtdProcessing{2}; }    ///< `Parse`.

  /// \brief The number. \return It.
  [[nodiscard]] std::int32_t Number() const { return number_; }

  /// \brief The value by its number, as AL spells `DtdProcessing := 2`. \param number The value.
  /// \return The value.
  [[nodiscard]] static DtdProcessing FromNumber(std::int32_t number) { return DtdProcessing{number}; }

private:
  explicit DtdProcessing(std::int32_t number) : number_(number) {}
  std::int32_t number_ = 0;
};

/// \brief .NET `NetworkCredential` / `CredentialCache`: the BaseApp hands the default network
///        credentials to a URL resolver, and this reader resolves no URL, so they are carried empty.
class NetCredentialCache {
public:
  /// \brief `CredentialCache.DefaultNetworkCredentials`. \return An empty credential.
  [[nodiscard]] static NetCredentialCache DefaultNetworkCredentials() { return NetCredentialCache{}; }
};

/// \brief .NET `XmlUrlResolver`, carried and never asked: no external entity is fetched here.
class XmlUrlResolver {
public:
  /// \brief The binder behind `R := R.XmlUrlResolver()`.
  struct Binder {
    /// \brief `new XmlUrlResolver()`. \return A resolver.
    [[nodiscard]] class XmlUrlResolver operator()() const { return {}; }
  };

  /// \brief `R.XmlUrlResolver()`, the constructor as AL calls it.
  Binder XmlUrlResolver;

  /// \brief `XmlUrlResolver.Credentials := c`, accepted and unused.
  /// \tparam Credential Whatever was handed over.
  template <typename Credential> void Credentials(const Credential &) {}
};

/// \brief .NET `StringReader`: text held for a reader to read.
class StringReader {
public:
  /// \brief The binder behind `S := S.StringReader(text)`.
  struct Binder {
    /// \brief `new StringReader(text)`. \param text The text. \return The reader.
    [[nodiscard]] class StringReader operator()(std::string_view text) const;
  };

  /// \brief `S.StringReader(text)`, the constructor as AL calls it.
  Binder StringReader;

  /// \brief The text held. \return It.
  [[nodiscard]] std::string_view Text() const { return text_; }

  /// \brief `StringReader.Close()`: lets the text go.
  void Close() { text_.clear(); }

private:
  std::string text_;
};

/// \brief .NET `XmlReaderSettings`: the BaseApp sets `DtdProcessing` and `XmlResolver`, and this
///        reader ignores DTDs and resolves nothing whatever they say.
class XmlReaderSettings {
public:
  /// \brief The binder behind `S := S.XmlReaderSettings()`.
  struct Binder {
    /// \brief `new XmlReaderSettings()`. \return Settings.
    [[nodiscard]] class XmlReaderSettings operator()() const { return {}; }
  };

  /// \brief `S.XmlReaderSettings()`, the constructor as AL calls it.
  Binder XmlReaderSettings;

  /// \brief `Settings.DtdProcessing := d`. \param processing The value.
  void DtdProcessing(const dotnet::DtdProcessing &processing) { dtd_ = processing; }

  /// \brief `Settings.DtdProcessing := 2`, the enum by its number. \param number The value.
  void DtdProcessing(::agiru::Integer number) { dtd_ = dotnet::DtdProcessing::FromNumber(number); }

  /// \brief `Settings.DtdProcessing`, read. \return The value.
  [[nodiscard]] dotnet::DtdProcessing DtdProcessing() const { return dtd_; }

  /// \brief `Settings.XmlResolver := r`, accepted and unused. \tparam Resolver The resolver's type.
  template <typename Resolver> void XmlResolver(const Resolver &) {}

private:
  dotnet::DtdProcessing dtd_;
};

/// \brief .NET `XmlReader`, rebuilt over libxml2's `xmlTextReader`: a forward-only walk over
///        the nodes of a document, which `XML Buffer Writer` uses to fill the XML Buffer and
///        `XML DOM Management` to read a document from a stream (11 UT cases refused at
///        `XmlReaderSettings.XmlReaderSettings`, 2026-09-10).
///
/// \note THE MEMBERS ARE THE ONES THE BASEAPP NAMES: `Create` over a path, a stream or a
///       `StringReader`; `Read`, `Close`, `Depth`, `Name`, `Value`, `NodeType`,
///       `MoveToFirstAttribute`, `MoveToNextAttribute`. Whitespace between elements is reported
///       as .NET reports it with `IgnoreWhitespace = false` -- the default, which the BaseApp
///       does not change -- so a walk sees the same node sequence.
class XmlReader {
public:
  /// \brief `XmlReader.Create(path, settings)`. \param path A file path. \param settings The
  ///        settings. \return A reader over the file.
  /// \throws Error when the file cannot be read or is not XML.
  static XmlReader Create(std::string_view path, const XmlReaderSettings &settings);

  /// \brief `XmlReader.Create(path)` with the default settings. \param path The file.
  /// \return The reader. \throws Error when the file cannot be opened.
  static XmlReader Create(std::string_view path) { return Create(path, XmlReaderSettings{}); }

  /// \brief `XmlReader.Create(stream)` with the default settings. \param stream The stream.
  /// \return The reader.
  static XmlReader Create(const ::agiru::InStream &stream) {
    return Create(stream, XmlReaderSettings{});
  }

  /// \brief `XmlReader.Create(reader)` with the default settings. \param reader The text.
  /// \return The reader.
  static XmlReader Create(const StringReader &reader) { return Create(reader, XmlReaderSettings{}); }

  /// \brief `XmlReader.Create(stream, settings)`. \param stream The stream, read whole.
  ///        \param settings The settings. \return A reader over the stream's bytes.
  /// \throws Error when the bytes are not XML.
  static XmlReader Create(const ::agiru::InStream &stream, const XmlReaderSettings &settings);

  /// \brief `XmlReader.Create(outStream, settings)`: an OutStream cannot be read back, and the
  ///        platform refuses too. \param stream The stream. \param settings The settings.
  ///        \return Never. \throws Error always.
  static XmlReader Create(const ::agiru::OutStream &stream, const XmlReaderSettings &settings);

  /// \brief `XmlReader.Create(stringReader, settings)`. \param reader The text.
  ///        \param settings The settings. \return A reader over the text.
  /// \throws Error when the text is not XML.
  static XmlReader Create(const StringReader &reader, const XmlReaderSettings &settings);

  /// \brief `XmlReader.Read()`: moves to the next node. \return Whether there was one.
  [[nodiscard]] Boolean Read();

  /// \brief `XmlReader.Close()`: ends the walk.
  void Close();

  /// \brief `XmlReader.Depth`. \return The current node's depth, the root element at 0.
  [[nodiscard]] Integer Depth() const;

  /// \brief `XmlReader.Name`. \return The current node's qualified name; empty for text.
  [[nodiscard]] ::agiru::Text<0> Name() const;

  /// \brief `XmlReader.Value`. \return The current node's text value; empty for an element.
  [[nodiscard]] ::agiru::Text<0> Value() const;

  /// \brief `XmlReader.NodeType`. \return The current node's type.
  [[nodiscard]] XmlNodeType NodeType() const;

  /// \brief `XmlReader.MoveToFirstAttribute()`. \return Whether the element has one.
  [[nodiscard]] Boolean MoveToFirstAttribute();

  /// \brief `XmlReader.MoveToNextAttribute()`. \return Whether there was another.
  [[nodiscard]] Boolean MoveToNextAttribute();

  /// \brief `XmlReader.MoveToElement()`: back from an attribute to its element.
  /// \return Whether the reader stood on an attribute.
  [[nodiscard]] Boolean MoveToElement();

  /// \brief `XmlReader.IsEmptyElement`. \return Whether the element is `<a/>`.
  [[nodiscard]] Boolean IsEmptyElement() const;

  /// \brief `XmlReader.EOF`, spelled `Eof` because `EOF` is a macro every C++ translation unit
  ///        carries from `<cstdio>` -- the one door name that cannot be AL's (recorded here).
  /// \return Whether the walk is over.
  [[nodiscard]] Boolean Eof() const;

  /// \brief The whole document the reader reads, for a consumer that takes it at once.
  /// \return The text, empty before `Create`.
  [[nodiscard]] std::string_view Source() const {
    return text_ == nullptr ? std::string_view{} : std::string_view(*text_);
  }

private:
  static XmlReader Over(std::string text);
  std::shared_ptr<void> reader_;
  std::shared_ptr<std::string> text_;
  bool over_ = false;
  bool declarationPending_ = false;
  bool atDeclaration_ = false;
  std::string declaration_;
};

/// \brief .NET `XmlTextReader`: the same reader under the name `XML DOM Management` uses.
class XmlTextReader : public XmlReader {
public:
  /// \brief The binder behind `R := R.XmlTextReader(...)`.
  struct Binder {
    /// \brief `new XmlTextReader(path)`. \param path The file. \return The reader.
    [[nodiscard]] class XmlTextReader operator()(std::string_view path) const {
      return XmlTextReader::Over(XmlReader::Create(path));
    }

    /// \brief `new XmlTextReader(reader)`. \param reader The text. \return The reader.
    [[nodiscard]] class XmlTextReader operator()(const StringReader &reader) const {
      return XmlTextReader::Over(XmlReader::Create(reader));
    }

    /// \brief `new XmlTextReader(stream)`. \param stream The stream. \return The reader.
    [[nodiscard]] class XmlTextReader operator()(const ::agiru::InStream &stream) const {
      return XmlTextReader::Over(XmlReader::Create(stream));
    }
  };

  /// \brief The constructor AL calls as a member.
  Binder XmlTextReader;

  /// \brief `XmlTextReader.Create(...)`, the same overloads. \tparam Arguments As `XmlReader`.
  /// \param arguments As `XmlReader`. \return The reader.
  template <typename... Arguments> static class XmlTextReader Create(Arguments &&...arguments) {
    return Over(XmlReader::Create(std::forward<Arguments>(arguments)...));
  }

private:
  static class XmlTextReader Over(XmlReader base) {
    class XmlTextReader out;
    static_cast<XmlReader &>(out) = std::move(base);
    return out;
  }
};

}
