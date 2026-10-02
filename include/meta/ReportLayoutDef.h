#pragma once

#include "meta/Ids.h"

#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief Immutable AL rendering declarations; not installed assets or a rendering capability.

namespace agiru {

/// \brief Lexical kinds retained in layout property values, including localization metadata.
enum class ReportLayoutTokenKind : std::uint8_t {
  Identifier,       ///< An unquoted AL name.
  QuotedIdentifier, ///< A quoted AL name, decoded without its delimiters.
  String,           ///< An AL string, with doubled apostrophes decoded.
  Integer,          ///< An integer spelling.
  Decimal,          ///< A decimal spelling, never converted to binary floating point.
  DateTime,         ///< An AL date/time spelling.
  Punctuation,      ///< A delimiter or operator in a property value.
  Directive,        ///< A retained compiler directive.
};

/// \brief One decoded property token. The kind distinguishes strings from names and numbers.
struct ReportLayoutTokenDef {
  ReportLayoutTokenKind kind{}; ///< The original lexical kind.
  std::string_view text{};      ///< The decoded AL token text.
};

/// \brief One source property; retention does not imply its behaviour is implemented.
struct ReportLayoutPropertyDef {
  std::string_view name{};                       ///< The original AL property spelling.
  std::string_view text{};                       ///< The parser's decoded value text.
  std::span<const ReportLayoutTokenDef> value{}; ///< All value/localization tokens in source order.
};

/// \brief Declaring object/app provenance, distinct from the target report after merging.
struct ReportLayoutOriginDef {
  ReportId report{};             ///< The declaring report, or zero for an extension declaration.
  ReportExtensionId extension{}; ///< The declaring extension, or zero for a report declaration.
  std::string_view name{};       ///< The declaring object's AL name.
  std::string_view nameSpace{};  ///< The declaring object's namespace.
  std::string_view appId{};  ///< The declaring app GUID from its manifest; empty if unavailable.
  std::string_view source{}; ///< The source path relative to the AL input root.
};

/// \brief A named report/extension rendering declaration, in source/merge order.
/// \note Type, Subtype, LayoutFile and localized labels remain properties, not inferred defaults.
struct ReportLayoutDef {
  std::string_view name{};                               ///< The decoded AL layout name.
  std::span<const ReportLayoutPropertyDef> properties{}; ///< All declared properties, in order.
  ReportLayoutOriginDef origin{}; ///< Exactly one declaring object ID is nonzero.
};

}
