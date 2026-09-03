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
/// \brief AL `XmlNamespaceManager` -- the surface the platform documentation declares.

namespace agiru {

class XmlNameTable;

/// \brief AL `XmlNamespaceManager`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlnamespacemanager/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlNamespaceManager {
public:
  /// \brief AL `XmlNamespaceManager.AddNamespace(Text, Text)`. Adds the given namespace to the
  /// collection.
  /// \param Prefix The AL `Text`.
  /// \param Uri The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddNamespace(std::string_view Prefix, std::string_view Uri);

  /// \brief AL `XmlNamespaceManager.HasNamespace(Text)`. Gets a value indicating whether the
  /// supplied prefix has a namespace defined for the current scope.
  /// \param Prefix The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HasNamespace(std::string_view Prefix);

  /// \brief AL `XmlNamespaceManager.LookupNamespace(Text, Text)`. Gets the namespace URI for the
  /// specified prefix.
  /// \param Prefix The AL `Text`.
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean LookupNamespace(std::string_view Prefix, std::string &Result);

  /// \brief AL `XmlNamespaceManager.LookupPrefix(Text, Text)`. Finds the prefix declared for the
  /// given namespace URI.
  /// \param Uri The AL `Text`.
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean LookupPrefix(std::string_view Uri, std::string &Result);

  /// \brief AL `XmlNamespaceManager.NameTable(XmlNameTable)`. Gets or sets the XmlNameTable
  /// associated with this object.
  /// \param NewValue The AL `XmlNameTable`.
  /// \return The AL `XmlNameTable`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNameTable NameTable(const ::agiru::XmlNameTable &NewValue);

  /// \brief AL `XmlNamespaceManager.PopScope()`. Pops a namespace scope off the stack.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void PopScope();

  /// \brief AL `XmlNamespaceManager.PushScope()`. Pushes a namespace scope onto the stack.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void PushScope();

  /// \brief AL `XmlNamespaceManager.RemoveNamespace(Text, Text)`. Removes the given namespace for
  /// the given prefix.
  /// \param Prefix The AL `Text`.
  /// \param Uri The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveNamespace(std::string_view Prefix, std::string_view Uri);
};

}
