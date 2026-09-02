#pragma once

#include "runtime/Error.h"
#include "runtime/RecordRef.h"
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
/// \brief AL `File` -- the surface the platform documentation declares.

namespace agiru {

class BigText;

/// \brief AL `File`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/file/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class File {
public:
  /// \brief AL `File.Close()`. Closes a file that has been opened by the OPEN method (File).
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Close();

  /// \brief AL `File.Copy(Text, Text)`. Copies a file.
  /// \param FromName The AL `Text`.
  /// \param ToName The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Copy(std::string_view FromName, std::string_view ToName);

  /// \brief AL `File.Create(Text, TextEncoding)`. Creates an Automation object.
  /// \param Name The AL `Text`.
  /// \param Encoding The AL `TextEncoding`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Create(std::string_view Name, const ::agiru::TextEncoding &Encoding);

  /// \brief AL `File.CreateInStream(InStream)`. Creates an InStream object for a file. This enables
  /// you to import or read data from the file.
  /// \param InStream The AL `InStream`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateInStream(const ::agiru::InStream &InStream);

  /// \brief AL `File.CreateOutStream(OutStream)`. Creates an OutStream object for a file. This
  /// enables you to export or write data to the file.
  /// \param OutStream The AL `OutStream`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateOutStream(const ::agiru::OutStream &OutStream);

  /// \brief AL `File.CreateTempFile(TextEncoding)`. Creates a temporary file. This enables you to
  /// save data of any format to a temporary file. This file has a unique name and will be stored in
  /// a temporary file folder.
  /// \param Encoding The AL `TextEncoding`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean CreateTempFile(const ::agiru::TextEncoding &Encoding);

  /// \brief AL `File.Download(Text, Text, Text, Text, Text)`. Sends a file from a server computer
  /// to the client computer. The client computer is the computer that is running the Windows client
  /// or the computer that is running a browser that accesses the web client.
  /// \param FromFile The AL `Text`.
  /// \param DialogTitle The AL `Text`.
  /// \param ToFolder The AL `Text`.
  /// \param ToFilter The AL `Text`.
  /// \param ToFile The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Download(std::string_view FromFile,
                                   std::string_view DialogTitle,
                                   std::string_view ToFolder,
                                   std::string_view ToFilter,
                                   std::string &ToFile);

  /// \brief AL `File.DownloadFromStream(InStream, Text, Text, Text, Text)`. Sends a file from
  /// server computer to the client computer. The client computer is the computer that is running
  /// the Windows client or the computer that is running the browser that accesses the web client.
  /// \param InStream The AL `InStream`.
  /// \param DialogTitle The AL `Text`.
  /// \param ToFolder The AL `Text`.
  /// \param ToFilter The AL `Text`.
  /// \param ToFile The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean DownloadFromStream(const ::agiru::InStream &InStream,
                                             std::string_view DialogTitle,
                                             std::string_view ToFolder,
                                             std::string_view ToFilter,
                                             std::string &ToFile);

  /// \brief AL `File.Erase(Text)`. Deletes a file.
  /// \param Name The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Erase(std::string_view Name);

  /// \brief AL `File.Exists(Text)`. Determines whether a file exists.
  /// \param Name The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Exists(std::string_view Name);

  /// \brief AL `File.GetStamp(Text, Date, Time)`. Gets the exact time that a file was last written
  /// to.
  /// \param Name The AL `Text`.
  /// \param Date The AL `Date`.
  /// \param Time The AL `Time`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time);

  /// \brief AL `File.IsPathTemporary(Text)`. Validates whether the given path is located in the
  /// current users temporary folder within the current service.
  /// \param Name The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsPathTemporary(std::string_view Name);

  /// \brief AL `File.Len()`. Gets the length of an ASCII or binary file.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Len();

  /// \brief AL `File.Name()`. Gets the name of an ASCII or binary file.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Name();

  /// \brief AL `File.Open(Text, TextEncoding)`. Opens an ASCII or binary file. This method does not
  /// create the file if it does not exist.
  /// \param Name The AL `Text`.
  /// \param Encoding The AL `TextEncoding`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Open(std::string_view Name, const ::agiru::TextEncoding &Encoding);

  /// \brief AL `File.Pos()`. Gets the current position of the file pointer in an ASCII or binary
  /// file.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Pos();

  /// \brief AL `File.Read(Any)`. Reads from an MS-DOS encoded file or binary file.
  /// \param Read The AL `Any`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Read(::agiru::Variant &Read);

  /// \brief AL `File.Rename(Text, Text)`. Renames an ASCII or binary file.
  /// \param OldName The AL `Text`.
  /// \param NewName The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Rename(std::string_view OldName, std::string_view NewName);

  /// \brief AL `File.Seek(Integer)`. Sets a file pointer to a new position in an ASCII or binary
  /// file.
  /// \param Position The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Seek(::agiru::Integer Position);

  /// \brief AL `File.SetStamp(Text, Date, Time)`. Sets a timestamp for a file.
  /// \param Name The AL `Text`.
  /// \param Date The AL `Date`.
  /// \param Time The AL `Time`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time);

  /// \brief AL `File.TextMode(Boolean)`. Sets whether a file should be opened as an ASCII file or a
  /// binary file. Gets the current setting of this option for a file.
  /// \param Mode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean TextMode(::agiru::Boolean Mode);

  /// \brief AL `File.Trunc()`. Truncate an ASCII or binary file to the current position of the file
  /// pointer.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Trunc();

  /// \brief AL `File.Upload(Text, Text, Text, Text, Text)`. Sends a file from the client computer
  /// to the server computer. The client computer is the computer that is running the Windows client
  /// or the computer that is running a browser that accesses the web client.
  /// \param DialogTitle The AL `Text`.
  /// \param FromFolder The AL `Text`.
  /// \param FromFilter The AL `Text`.
  /// \param FromFile The AL `Text`.
  /// \param ToFile The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Upload(std::string_view DialogTitle,
                                 std::string_view FromFolder,
                                 std::string_view FromFilter,
                                 std::string_view FromFile,
                                 std::string &ToFile);

  /// \brief AL `File.UploadIntoStream(Text, InStream)`. Sends a file from the client computer to
  /// the corresponding server. The client computer is the computer that is running a browser that
  /// accesses the web client.
  /// \param FromFilter The AL `Text`.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean UploadIntoStream(std::string_view FromFilter,
                                           ::agiru::InStream &InStream);

  /// \brief AL `File.UploadIntoStream(Text, Text, Text, Text, InStream)`. Sends a file from the
  /// client computer to the corresponding server. The client computer is the computer that is
  /// running the Windows client or the computer that is running a browser that accesses the web
  /// client.
  /// \param DialogTitle The AL `Text`.
  /// \param FromFolder The AL `Text`.
  /// \param FromFilter The AL `Text`.
  /// \param FromFile The AL `Text`.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean UploadIntoStream(std::string_view DialogTitle,
                                           std::string_view FromFolder,
                                           std::string_view FromFilter,
                                           std::string &FromFile,
                                           ::agiru::InStream &InStream);

  /// \brief AL `File.View(Text, Boolean)`. Opens a file from server computer on the client computer
  /// in preview mode. The client computer is the computer that is running the browser that accesses
  /// the web client.
  /// \param FromFile The AL `Text`.
  /// \param AllowDownloadAndPrint The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean View(std::string_view FromFile, ::agiru::Boolean AllowDownloadAndPrint);

  /// \brief AL `File.ViewFromStream(InStream, Text, Boolean)`. Opens a file from the server on the
  /// client computer in preview mode. The client computer is defined as the machine running the
  /// browser accessing the web client.
  /// \param InStream The AL `InStream`.
  /// \param FileName The AL `Text`.
  /// \param AllowDownloadAndPrint The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean ViewFromStream(const ::agiru::InStream &InStream,
                                         std::string_view FileName,
                                         ::agiru::Boolean AllowDownloadAndPrint);

  /// \brief AL `File.Write(BigInteger)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::BigInteger Value);

  /// \brief AL `File.Write(BigText)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `BigText`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(const ::agiru::BigText &Value);

  /// \brief AL `File.Write(Boolean)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Boolean Value);

  /// \brief AL `File.Write(Byte)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Byte`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Byte Value);

  /// \brief AL `File.Write(Char)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Char`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Char Value);

  /// \brief AL `File.Write(Code)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Code`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(std::string_view Value);

  /// \brief AL `File.Write(Date)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Date Value);

  /// \brief AL `File.Write(DateFormula)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `DateFormula`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::DateFormula Value);

  /// \brief AL `File.Write(DateTime)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::DateTime Value);

  /// \brief AL `File.Write(Decimal)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Decimal Value);

  /// \brief AL `File.Write(Duration)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Duration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Duration Value);

  /// \brief AL `File.Write(Guid)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Guid`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Guid Value);

  /// \brief AL `File.Write(Integer)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Integer Value);

  /// \brief AL `File.Write(Any)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(const ::agiru::Variant &Value);

  /// \brief AL `File.Write(RecordId)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `RecordId`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::RecordId Value);

  /// \brief AL `File.Write(Record)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Record`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(const ::agiru::RecordRef &Value);

  /// \brief AL `File.Write(Time)`. Writes to an MS-DOS encoded file or binary file.
  /// \param Value The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Write(::agiru::Time Value);

  /// \brief AL `File.WriteMode(Boolean)`. Use this method before you use OPEN method (File)] to set
  /// or test whether you can write to a file in later calls.
  /// \param Mode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteMode(::agiru::Boolean Mode);
};

} // namespace agiru
