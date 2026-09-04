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
  /// \brief AL `SessionSettings.Company()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] SessionSettings.Company([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] std::string Company();

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
  /// \brief AL `SessionSettings.LanguageId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] SessionSettings.LanguageId([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Integer LanguageId();

  ::agiru::Integer LanguageId(::agiru::Integer NewLanguageId);

  /// \brief AL `SessionSettings.LocaleId(Integer)`. Gets or sets the locale ID property in a
  /// SessionSettings object.
  /// \param NewLocaleId The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `SessionSettings.LocaleId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] SessionSettings.LocaleId([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Integer LocaleId();

  ::agiru::Integer LocaleId(::agiru::Integer NewLocaleId);

  /// \brief AL `SessionSettings.ProfileAppId(Guid)`. Gets or sets the ID of an extension, which
  /// provides a profile, in a SessionSettings object.
  /// \param NewProfileAppId The AL `Guid`.
  /// \return The AL `Guid`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `SessionSettings.ProfileAppId()` -- the READING form, which the documentation's
  /// syntax block brackets: `[X := ] SessionSettings.ProfileAppId([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Guid ProfileAppId();

  ::agiru::Guid ProfileAppId(::agiru::Guid NewProfileAppId);

  /// \brief AL `SessionSettings.ProfileId(Text)`. Gets or sets the profile ID property in a
  /// SessionSettings object.
  /// \param NewProfileId The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `SessionSettings.ProfileId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] SessionSettings.ProfileId([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] std::string ProfileId();

  std::string ProfileId(std::string_view NewProfileId);

  /// \brief AL `SessionSettings.ProfileSystemScope(Boolean)`. Gets or sets the profile scope
  /// property in a SessionSettings object.
  /// \param NewProfileScope The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `SessionSettings.ProfileSystemScope()` -- the READING form, which the
  /// documentation's syntax block brackets: `[X := ] SessionSettings.ProfileSystemScope([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Boolean ProfileSystemScope();

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
  /// \brief AL `SessionSettings.TimeZone()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] SessionSettings.TimeZone([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] std::string TimeZone();

  std::string TimeZone(std::string_view NewTimeZone);
};

}
