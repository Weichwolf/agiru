// Generated from Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al. Do not edit.

#include "TransferOldExtTextLines.h"

#include "Builtins.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Events.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include "BuiltinsWritten.h"
#include "LineNumberBuffer.h"

namespace agiru::Foundation::ExtendedText {

namespace {
namespace TransferOldExtTextLines_unit {
const RegisterCodeunit<TransferOldExtTextLines_Codeunit> kInCodeunitCatalogue;
} // namespace TransferOldExtTextLines_unit
} // namespace

void TransferOldExtTextLines_Codeunit::OnRun() {}

void TransferOldExtTextLines_Codeunit::InsertLineNumbers(Integer OldLineNo, Integer NewLineNo) {
  TempLineNumberBuffer->OldLineNumber = OldLineNo;
  TempLineNumberBuffer->NewLineNumber = NewLineNo;
  TempLineNumberBuffer->Insert();
}

Integer TransferOldExtTextLines_Codeunit::GetNewLineNumber(Integer OldLineNo) {
  if (TempLineNumberBuffer->Get(OldLineNo)) { return TempLineNumberBuffer->NewLineNumber; }
  return 0;
}

void TransferOldExtTextLines_Codeunit::ClearLineNumbers() {
  TempLineNumberBuffer->DeleteAll();
}

Integer TransferOldExtTextLines_Codeunit::TransferExtendedText(Integer OldLineNo,
                                                               Integer NewLineNo,
                                                               Integer AttachedLineNo) {
  Integer Result{};
  [[maybe_unused]] ::agiru::Boolean IsHandled{};

  IsHandled = false;
  OnBeforeTransferExtendedText(OldLineNo, NewLineNo, AttachedLineNo, Result, IsHandled);
  if (IsHandled) { return Result; }
  InsertLineNumbers(OldLineNo, NewLineNo);
  if (AttachedLineNo != 0) { return GetNewLineNumber(AttachedLineNo); }
  return 0;
}

void TransferOldExtTextLines_Codeunit::GetLineNoBuffer(
    ::agiru::app::tables::LineNumberBuffer_Table &OutTempLineNumberBuffer) {
  OutTempLineNumberBuffer.Copy(TempLineNumberBuffer, true);
}

void TransferOldExtTextLines_Codeunit::OnBeforeTransferExtendedText(Integer OldLineNo,
                                                                    Integer NewLineNo,
                                                                    Integer AttachedLineNo,
                                                                    Integer &Result,
                                                                    ::agiru::Boolean &IsHandled) {
  static constexpr std::array<std::string_view, 5> kNames{
      "OldLineNo", "NewLineNo", "AttachedLineNo", "Result", "IsHandled"};
  ::agiru::detail::RaiseEventFrom(
      static_cast<void *>(this),
      ::agiru::EventObject::Codeunit,
      ::agiru::CodeunitTraits<
          ::agiru::Foundation::ExtendedText::TransferOldExtTextLines_Codeunit>::kId.Value(),
      ::agiru::CodeunitTraits<
          ::agiru::Foundation::ExtendedText::TransferOldExtTextLines_Codeunit>::kName,
      "OnBeforeTransferExtendedText",
      {},
      kNames,
      OldLineNo,
      NewLineNo,
      AttachedLineNo,
      Result,
      IsHandled);
}

void TransferOldExtTextLines_Codeunit::ClearAll() {
  ::agiru::Clear(TempLineNumberBuffer);
}

constexpr CodeunitDef kTransferOldExtTextLinesCodeunit{
    .id = ::agiru::CodeunitTraits<TransferOldExtTextLines_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<TransferOldExtTextLines_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<TransferOldExtTextLines_Codeunit>::kSubtype,
};

} // namespace agiru::Foundation::ExtendedText
