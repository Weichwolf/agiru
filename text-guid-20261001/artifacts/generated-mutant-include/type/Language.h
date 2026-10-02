#pragma once

#include "type/Integer.h"

/// \file
/// \brief The language a session formats in, as the value layer sees it.

namespace agiru {

/// \brief AL `GlobalLanguage` on the VALUE side: the Windows language id a value renders in.
///
/// From `system-globallanguage-method.md`: "the current global language setting" decides how a
/// value shows -- a `DateFormula`'s letters (`dateformula-data-type.md`: `1Y` is `1J` in a German
/// session), a `CultureInfo.CurrentCulture`, a number's separators. The value layer sees nothing
/// but the standard library, so the language it formats in is ITS OWN `thread_local`, written by
/// the session that owns the thread and read here without naming the session.
///
/// \note IT IS PER THREAD BECAUSE A SESSION IS PER THREAD. A service tier runs ten thousand
///       sessions in one process, and a process-wide language would be one session's answering
///       for every other (\see Session::Language). The session writes it when it opens, when
///       `GlobalLanguage(Integer)` moves it, and gives the thread's previous session its language
///       back when it closes.
class Language {
public:
  /// \brief `en-US`, the language the BaseApp's captions are written in and the one a session
  ///        opens in.
  static constexpr Integer kEnglishUnitedStates = 1033;

  /// \brief The language this thread formats in.
  /// \return The Windows language id; `kEnglishUnitedStates` until a session says otherwise.
  [[nodiscard]] static Integer Current();

  /// \brief Moves this thread's formatting language.
  /// \param id The Windows language id; 0 is read as `kEnglishUnitedStates`.
  static void MakeCurrent(Integer id);
};

}
