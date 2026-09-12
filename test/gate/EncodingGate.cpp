#include "dotnet/Encoding.h"
#include "type/Integer.h"
#include "type/Text.h"

#include "Check.h"

#include <string>

using agiru::dotnet::Encoding;

namespace {

/// `GetEncoding(0)` IS THE SERVER'S ANSI PAGE, 1252, AND `Default` IS UTF-8. The BaseApp writes
/// them as a pair: an xmlport declared `TextEncoding = WINDOWS` and a `StreamReader(InStream,
/// Encoding.GetEncoding(0))` reading its output back (Payment Export XMLPort UT). Under UTF-8 the
/// three Scandinavian letters came back as three question marks.
void CodePageZeroIsTheAnsiPage() {
  const Encoding ansi = Encoding::GetEncoding(agiru::Integer{0});
  CHECK_TEXT("bytes E6 F8 E5 read as Latin letters",
             ansi.Decode("\xE6\xF8\xE5"),
             "\xC3\xA6\xC3\xB8\xC3\xA5");
  CHECK_TEXT("and the letters write as the same bytes",
             ansi.Encode("\xC3\xA6\xC3\xB8\xC3\xA5"),
             "\xE6\xF8\xE5");
  CHECK_TRUE("the same page by name",
             Encoding::GetEncoding("windows-1252").Decode("\xE6") == ansi.Decode("\xE6"));
  CHECK_TEXT("Default stays UTF-8", Encoding::Default().Decode("\xC3\xA6"), "\xC3\xA6");
  CHECK_TEXT("and UTF-8 round-trips",
             Encoding::GetEncoding(agiru::Integer{65001}).Encode("\xC3\xA6"),
             "\xC3\xA6");
}

}

int main() {
  return gate::Run("Encoding", [] { CodePageZeroIsTheAnsiPage(); });
}
