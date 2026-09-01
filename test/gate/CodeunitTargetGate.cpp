#include "Check.h"
#include "LineNumberBuffer.h"
#include "TransferOldExtTextLines.h"
#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Integer.h"

#include <string>
#include <type_traits>

using agiru::CodeunitId;
using agiru::Error;
using agiru::app::LineNumberBuffer;
using agiru::app::TransferOldExtTextLines;

namespace {

// WHAT THE COMPILER CAN DECIDE IS A static_assert AND NEVER A CASE.
constexpr int kAlNumber = 379;
static_assert(agiru::CodeunitTraits<TransferOldExtTextLines>::kId == CodeunitId{kAlNumber});
static_assert(agiru::CodeunitTraits<TransferOldExtTextLines>::kName ==
              "Transfer Old Ext. Text Lines");
static_assert(std::is_standard_layout_v<LineNumberBuffer>,
              "a temporary record is the SAME table, so it must stay addressable by offsetof");

/// The codeunit carries its number and name, and the class itself says neither -- the same split a
/// table makes between its class and its TableDef.
void TheCodeunitKnowsWhatAlNamedIt() {
  const TransferOldExtTextLines codeunit{};
  CHECK_TRUE("the number comes from AL", codeunit.Id() == CodeunitId{kAlNumber});
  CHECK_TEXT("and so does the name", std::string(codeunit.Name()), "Transfer Old Ext. Text Lines");
}

/// `Codeunit.Run` OPENS A TRANSACTION BOUNDARY on the session's own pinned connection, so outside
/// a session there is nothing to open. That is a defect in the HOST rather than in AL code -- an AL
/// statement can only run inside a session -- which is why it raises rather than reporting false.
void WhatIsNotBuiltYetRefusesRatherThanPretending() {
  TransferOldExtTextLines codeunit;
  std::string said;
  try {
    (void)codeunit.Run();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("Codeunit.Run outside a session refuses", !said.empty());
  CHECK_TRUE("saying it is the session that is missing", said.find("session") != std::string::npos);

  said.clear();
  try {
    codeunit.ClearLineNumbers();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a temporary record refuses too", !said.empty());
  CHECK_TRUE("and names its own item", said.find("board:0020") != std::string::npos);
  CHECK_TRUE("saying which AL method it was", said.find("DeleteAll") != std::string::npos);
}

/// The negative control for the refusals: what IS built must not refuse. A procedure that touches
/// no record runs, which proves the codeunit is a working class and not a wall.
void WhatIsBuiltDoesNotRefuse() {
  TransferOldExtTextLines codeunit;
  std::string said;
  try {
    codeunit.OnRun();
  } catch (const Error &e) { said = e.what(); }
  CHECK_SILENT("an empty OnRun runs", said);

  // And an event publisher with no subscribers does nothing, which is correct rather than a stub:
  // AL fires subscribers at the CALL SITE, so the publisher's own body has nothing to do.
  constexpr agiru::Integer kMarker = 7;
  agiru::Temporary<LineNumberBuffer> buffer;
  buffer.OldLineNumber = kMarker;
  said.clear();
  try {
    codeunit.GetLineNoBuffer(buffer);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("reaching a temporary record refuses wherever it happens", !said.empty());
  CHECK_TRUE("and the record it refused on is untouched", buffer.OldLineNumber == kMarker);
}

} // namespace

int main() {
  return gate::Run("CodeunitTarget", [] {
    TheCodeunitKnowsWhatAlNamedIt();
    WhatIsNotBuiltYetRefusesRatherThanPretending();
    WhatIsBuiltDoesNotRefuse();
  });
}
