// Generated from Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al. Do not edit.

#pragma once

#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

namespace agiru::app::tables {
class LineNumberBuffer_Table;
using LineNumberBuffer = LineNumberBuffer_Table;
} // namespace agiru::app::tables

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::app::codeunits {

class TransferOldExtTextLines_Codeunit;
using TransferOldExtTextLines = TransferOldExtTextLines_Codeunit;

class TransferOldExtTextLines_Codeunit : public Codeunit<TransferOldExtTextLines_Codeunit> {
public:
  void OnRun();

  Integer GetNewLineNumber(Integer OldLineNo);
  void ClearLineNumbers();
  Integer TransferExtendedText(Integer OldLineNo, Integer NewLineNo, Integer AttachedLineNo);
  void GetLineNoBuffer(Temporary<tables::LineNumberBuffer> &OutTempLineNumberBuffer);

private:
  Instance<Temporary<tables::LineNumberBuffer>> TempLineNumberBuffer;

  void InsertLineNumbers(Integer OldLineNo, Integer NewLineNo);
  void OnBeforeTransferExtendedText(Integer OldLineNo,
                                    Integer NewLineNo,
                                    Integer AttachedLineNo,
                                    Integer &Result,
                                    Boolean &IsHandled);
};

} // namespace agiru::app::codeunits

template <> struct agiru::CodeunitTraits<agiru::app::codeunits::TransferOldExtTextLines> {
  static constexpr CodeunitId kId{379};
  static constexpr std::string_view kName{"Transfer Old Ext. Text Lines"};
  static constexpr Subtype kSubtype{Subtype::Normal};
};
