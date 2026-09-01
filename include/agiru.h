#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Codeunit.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Indexing.h"
#include "runtime/Record.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Code.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Enum.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/StringValue.h"
#include "type/Text.h"
#include "type/Time.h"

/// \file
/// \brief The door. One line, and an app has all of AL.
///
/// A GENERATED FILE INCLUDES THIS AND NOTHING ELSE FROM THE RUNTIME, because an AL file declares no
/// includes at all and the translation should not invent a list AL never wrote.
///
/// It costs nothing, and that was measured rather than assumed (2026-09-01, 200 generated table
/// headers, clang-19): with each file's own includes the pass takes 64 s and 62 s over two runs;
/// with the whole door forced into every file, 63 s and 63 s. The reason is in the preprocessor --
/// the whole door is 36 595 lines while a typical table header, which reaches for only 8 of the 17
/// type headers, already comes to 36 760. Some 36 500 of those are `<string_view>`, `<compare>`,
/// `<span>` and `<array>`, which every one of the door's headers pulls anyway. The door itself is
/// about 200 lines. There is nothing to save by including less of it.
///
/// The headers behind it stay one per AL type, and they stay reachable on their own: the runtime's
/// own sources include what they use, and a reader looking for `Date` finds `type/Date.h`.
///
/// \note The paths here are quoted rather than angled, which is what keeps the short names safe. A
///       quoted include is looked for beside the INCLUDING file first, so `type/Date.h` resolves
///       under `include/` before any `-I` on the command line is consulted -- and an app is
///       compiled with `-Iinclude -Iapps/<app>` at once.
