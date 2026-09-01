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

  // A TEMPORARY RECORD NEEDS NO SESSION AT ALL, which is the whole point of one: it is the same
  // table with no database behind it, so the codeunit's own buffer works outside a session where
  // every other record operation cannot.
  said.clear();
  try {
    codeunit.ClearLineNumbers();
  } catch (const Error &e) { said = e.what(); }
  CHECK_SILENT("a temporary record works with no session open", said);
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
  // AND THE CODEUNIT'S OWN BUFFER WORKS END TO END, which is what a walking skeleton is for: the
  // hand-written specification is a running object, not a shape.
  constexpr agiru::Integer kOld = 7;
  constexpr agiru::Integer kNew = 11;
  CHECK_TRUE("a line number that was never transferred is 0", codeunit.GetNewLineNumber(kOld) == 0);
  CHECK_TRUE("transferring one with no attachment answers 0",
             codeunit.TransferExtendedText(kOld, kNew, 0) == 0);
  CHECK_TRUE("and the new number can be found by the old one",
             codeunit.GetNewLineNumber(kOld) == kNew);

  agiru::Temporary<LineNumberBuffer> buffer;
  codeunit.GetLineNoBuffer(buffer);
  CHECK_TRUE("the buffer handed out shares the store", buffer.Count() == 1);
  codeunit.ClearLineNumbers();
  CHECK_TRUE("so clearing it through the codeunit empties the shared one too", buffer.Count() == 0);
}

} // namespace

int main() {
  return gate::Run("CodeunitTarget", [] {
    TheCodeunitKnowsWhatAlNamedIt();
    WhatIsNotBuiltYetRefusesRatherThanPretending();
    WhatIsBuiltDoesNotRefuse();
  });
}
