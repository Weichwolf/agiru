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
#include "type/ObjectType.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/WebServiceActionResultCode.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `WebServiceActionContext` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `WebServiceActionContext`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/webserviceactioncontext/` states, so a call site compiles and is CHECKED;
///          the body refuses by name rather than returning a plausible wrong answer (board:0035).
class WebServiceActionContext {
public:
  /// \brief AL `WebServiceActionContext.AddEntityKey(Integer, Any)`. Add a new \<fieldId, value\>
  /// pair to the collection of entity keys.
  /// \param FieldId The AL `Integer`.
  /// \param FieldValue The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddEntityKey(::agiru::Integer FieldId, const ::agiru::Variant &FieldValue);

  /// \brief AL `WebServiceActionContext.GetObjectId()`. Gets the object ID.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer GetObjectId();

  /// \brief AL `WebServiceActionContext.GetObjectType()`. Gets the object type.
  /// \return The AL `ObjectType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::ObjectType GetObjectType();

  /// \brief AL `WebServiceActionContext.GetResultCode()`. Gets the web service action result status
  /// code.
  /// \return The AL `WebServiceActionResultCode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::WebServiceActionResultCode GetResultCode();

  /// \brief AL `WebServiceActionContext.SetObjectId(Integer)`. Sets the object ID.
  /// \param ObjectId The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetObjectId(::agiru::Integer ObjectId);

  /// \brief AL `WebServiceActionContext.SetObjectType(ObjectType)`. Sets the object type.
  /// \param ObjectType The AL `ObjectType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetObjectType(const ::agiru::ObjectType &ObjectType);

  /// \brief AL `WebServiceActionContext.SetResultCode(WebServiceActionResultCode)`. Sets the web
  /// service action result status code.
  /// \param ResultCode The AL `WebServiceActionResultCode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetResultCode(const ::agiru::WebServiceActionResultCode &ResultCode);
};

} // namespace agiru
