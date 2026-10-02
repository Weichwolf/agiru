#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Stream.h"
#include "type/TextEncoding.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `FileUpload` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `FileUpload`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/fileupload/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class FileUpload {
public:
  /// \brief AL `FileUpload.CreateInStream(InStream)`. Creates an InStream object for a file. This
  /// enables you to import or read data from the file.
  /// \param InStream The AL `InStream`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateInStream(const ::agiru::InStream &InStream);

  /// \brief AL `FileUpload.CreateInStream(InStream, TextEncoding)`. Creates an InStream object for
  /// a file. This enables you to import or read data from the file.
  /// \param InStream The AL `InStream`.
  /// \param Encoding The AL `TextEncoding`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateInStream(const ::agiru::InStream &InStream, const ::agiru::TextEncoding &Encoding);

  /// \brief AL `FileUpload.FileName()`. Gets the file name.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string FileName();
};

}
