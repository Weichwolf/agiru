// Generated from Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al. Do not edit.

#pragma once

#include "LineNumberBuffer.h"
#include "agiru.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::app {

class TransferOldExtTextLines : public Codeunit<TransferOldExtTextLines> {
public:
  void OnRun();

  Integer GetNewLineNumber(Integer OldLineNo);
  void ClearLineNumbers();
  Integer TransferExtendedText(Integer OldLineNo, Integer NewLineNo, Integer AttachedLineNo);
  void GetLineNoBuffer(Temporary<LineNumberBuffer> &OutTempLineNumberBuffer);

private:
  Temporary<LineNumberBuffer> TempLineNumberBuffer;

  void InsertLineNumbers(Integer OldLineNo, Integer NewLineNo);
  void OnBeforeTransferExtendedText(Integer OldLineNo,
                                    Integer NewLineNo,
                                    Integer AttachedLineNo,
                                    Integer &Result,
                                    Boolean &IsHandled);
};

} // namespace agiru::app

template <> struct agiru::CodeunitTraits<agiru::app::TransferOldExtTextLines> {
  static constexpr CodeunitId kId{379};
  static constexpr std::string_view kName{"Transfer Old Ext. Text Lines"};
};
