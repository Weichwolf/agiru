// Generated from Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include "LineNumberBuffer.h"

namespace agiru::app::tables {
class LineNumberBuffer_Table;
} // namespace agiru::app::tables

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Foundation::ExtendedText {

class TransferOldExtTextLines_Codeunit;

class TransferOldExtTextLines_Codeunit : public Codeunit<TransferOldExtTextLines_Codeunit> {
public:
  TransferOldExtTextLines_Codeunit() = default;
  using Codeunit<TransferOldExtTextLines_Codeunit>::operator=;

  void OnRun();

  Integer GetNewLineNumber(Integer OldLineNo);
  void ClearLineNumbers();
  Integer TransferExtendedText(Integer OldLineNo, Integer NewLineNo, Integer AttachedLineNo);
  void GetLineNoBuffer(::agiru::app::tables::LineNumberBuffer_Table &OutTempLineNumberBuffer);

  void ClearAll();

private:
  Instance<Temporary<::agiru::app::tables::LineNumberBuffer_Table>> TempLineNumberBuffer;

  void InsertLineNumbers(Integer OldLineNo, Integer NewLineNo);
  void OnBeforeTransferExtendedText(Integer OldLineNo,
                                    Integer NewLineNo,
                                    Integer AttachedLineNo,
                                    Integer &Result,
                                    ::agiru::Boolean &IsHandled);
};

extern const CodeunitDef kTransferOldExtTextLinesCodeunit;

} // namespace agiru::Foundation::ExtendedText

template <>
struct agiru::CodeunitTraits<agiru::Foundation::ExtendedText::TransferOldExtTextLines_Codeunit> {
  static constexpr CodeunitId kId{379};
  static constexpr std::string_view kName{"Transfer Old Ext. Text Lines"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit =
      agiru::Foundation::ExtendedText::kTransferOldExtTextLinesCodeunit;
};
