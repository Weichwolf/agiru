// Generated from Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al. Do not edit.

#include "TransferOldExtTextLines.h"

#include "agiru.h"

#include "LineNumberBuffer.h"

namespace agiru::app::codeunits {

void TransferOldExtTextLines::OnRun() {}

void TransferOldExtTextLines::InsertLineNumbers(Integer OldLineNo, Integer NewLineNo) {
  TempLineNumberBuffer->OldLineNumber = OldLineNo;
  TempLineNumberBuffer->NewLineNumber = NewLineNo;
  TempLineNumberBuffer->Insert();
}

Integer TransferOldExtTextLines::GetNewLineNumber(Integer OldLineNo) {
  if (TempLineNumberBuffer->Get(OldLineNo)) { return TempLineNumberBuffer->NewLineNumber; }
  return 0;
}

void TransferOldExtTextLines::ClearLineNumbers() {
  TempLineNumberBuffer->DeleteAll();
}

Integer TransferOldExtTextLines::TransferExtendedText(Integer OldLineNo,
                                                      Integer NewLineNo,
                                                      Integer AttachedLineNo) {
  Integer Result{};
  Boolean IsHandled{};

  IsHandled = false;
  OnBeforeTransferExtendedText(OldLineNo, NewLineNo, AttachedLineNo, Result, IsHandled);
  if (IsHandled) { return Result; }
  InsertLineNumbers(OldLineNo, NewLineNo);
  if (AttachedLineNo != 0) { return GetNewLineNumber(AttachedLineNo); }
  return 0;
}

void TransferOldExtTextLines::GetLineNoBuffer(
    Temporary<tables::LineNumberBuffer> &OutTempLineNumberBuffer) {
  OutTempLineNumberBuffer.Copy(TempLineNumberBuffer, true);
}

void TransferOldExtTextLines::OnBeforeTransferExtendedText(
    Integer, Integer, Integer, Integer &, Boolean &) {}

} // namespace agiru::app::codeunits
