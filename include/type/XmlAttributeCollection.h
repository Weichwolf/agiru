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
/// \brief AL `XmlAttributeCollection` -- the surface the platform documentation declares.

namespace agiru {

class XmlAttribute;

/// \brief AL `XmlAttributeCollection`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlattributecollection/` states, so a call site compiles and is CHECKED;
///          the body refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlAttributeCollection {
public:
  /// \brief AL `XmlAttributeCollection.Count()`. Gets the number of attributes in the
  /// XmlAttributeCollection.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Count();

  /// \brief AL `XmlAttributeCollection.Get(Integer, XmlAttribute)`. Gets the specified attribute.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `XmlAttribute`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(::agiru::Integer Index, ::agiru::XmlAttribute &Result);

  /// \brief AL `XmlAttributeCollection.Get(Text, Text, XmlAttribute)`. Gets the specified
  /// attribute.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \param Result The AL `XmlAttribute`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean
  Get(std::string_view LocalName, std::string_view NamespaceUri, ::agiru::XmlAttribute &Result);

  /// \brief AL `XmlAttributeCollection.Get(Text, XmlAttribute)`. Gets the specified attribute.
  /// \param Name The AL `Text`.
  /// \param Result The AL `XmlAttribute`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(std::string_view Name, ::agiru::XmlAttribute &Result);

  /// \brief AL `XmlAttributeCollection.Remove(Text)`. Removes the specified attribute from the
  /// collection.
  /// \param Name The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Remove(std::string_view Name);

  /// \brief AL `XmlAttributeCollection.Remove(Text, Text)`. Removes the specified attribute from
  /// the collection.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Remove(std::string_view LocalName, std::string_view NamespaceUri);

  /// \brief AL `XmlAttributeCollection.Remove(XmlAttribute)`. Removes the specified attribute from
  /// the collection.
  /// \param Attribute The AL `XmlAttribute`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Remove(const ::agiru::XmlAttribute &Attribute);

  /// \brief AL `XmlAttributeCollection.RemoveAll()`. Removes all attributes from the collection.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveAll();

  /// \brief AL `XmlAttributeCollection.Set(Text, Text)`. Sets the value of the specified attribute
  /// or creates it if is not part of the collection.
  /// \param Name The AL `Text`.
  /// \param Value The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Set(std::string_view Name, std::string_view Value);

  /// \brief AL `XmlAttributeCollection.Set(Text, Text, Text)`. Sets the value of the specified
  /// attribute or creates it if is not part of the collection.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \param Value The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Set(std::string_view LocalName, std::string_view NamespaceUri, std::string_view Value);
};

}
