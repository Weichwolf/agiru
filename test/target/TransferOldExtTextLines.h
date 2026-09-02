// Generated from Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al. Do not edit.

#pragma once

#include "LineNumberBuffer.h"
#include "agiru.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::app::codeunits {

class TransferOldExtTextLines : public Codeunit<TransferOldExtTextLines> {
public:
  void OnRun();

  Integer GetNewLineNumber(Integer OldLineNo);
  void ClearLineNumbers();
  Integer TransferExtendedText(Integer OldLineNo, Integer NewLineNo, Integer AttachedLineNo);
  void GetLineNoBuffer(Temporary<tables::LineNumberBuffer> &OutTempLineNumberBuffer);

private:
  Temporary<tables::LineNumberBuffer> TempLineNumberBuffer;

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
};
