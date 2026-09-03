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
/// \brief AL `SessionSettings` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `SessionSettings`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/sessionsettings/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class SessionSettings {
public:
  /// \brief AL `SessionSettings.Company(Text)`. Gets or sets the company property in a
  /// SessionSettings object.
  /// \param NewCompanyName The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Company(std::string_view NewCompanyName);

  /// \brief AL `SessionSettings.Init()`. Populates the instance of a SessionsSettings with the
  /// current client user's personalization properties (such as Profile ID and Company) that are
  /// stored in the database.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Init();

  /// \brief AL `SessionSettings.LanguageId(Integer)`. Gets or sets the language ID property in a
  /// SessionSettings object.
  /// \param NewLanguageId The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer LanguageId(::agiru::Integer NewLanguageId);

  /// \brief AL `SessionSettings.LocaleId(Integer)`. Gets or sets the locale ID property in a
  /// SessionSettings object.
  /// \param NewLocaleId The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer LocaleId(::agiru::Integer NewLocaleId);

  /// \brief AL `SessionSettings.ProfileAppId(Guid)`. Gets or sets the ID of an extension, which
  /// provides a profile, in a SessionSettings object.
  /// \param NewProfileAppId The AL `Guid`.
  /// \return The AL `Guid`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Guid ProfileAppId(::agiru::Guid NewProfileAppId);

  /// \brief AL `SessionSettings.ProfileId(Text)`. Gets or sets the profile ID property in a
  /// SessionSettings object.
  /// \param NewProfileId The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ProfileId(std::string_view NewProfileId);

  /// \brief AL `SessionSettings.ProfileSystemScope(Boolean)`. Gets or sets the profile scope
  /// property in a SessionSettings object.
  /// \param NewProfileScope The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ProfileSystemScope(::agiru::Boolean NewProfileScope);

  /// \brief AL `SessionSettings.RequestSessionUpdate(Boolean)`. Passes a SessionSettings object to
  /// the client to request a new session that uses the user personalization properties that are set
  /// in the object. The current client session is abandoned and a new session is started.
  /// \param saveSettings The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RequestSessionUpdate(::agiru::Boolean saveSettings);

  /// \brief AL `SessionSettings.TimeZone(Text)`. Gets or sets the time zone property in a
  /// SessionSettings object.
  /// \param NewTimeZone The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string TimeZone(std::string_view NewTimeZone);
};

}
