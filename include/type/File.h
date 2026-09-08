#pragma once

#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
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
#include "type/StringValue.h"
#include "type/TextEncoding.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

/// \file
/// \brief AL `File` -- the surface the platform documentation declares.

namespace agiru {

class BigText;

/// \brief AL `File`.
///
/// \note IT HOLDS THE WHOLE FILE AND NOT A HANDLE, and that is a decision rather than an
///       omission. AL's `File` is opened, streamed through and closed; a `Blob` is what this
///       runtime already streams through, so `Open` reads the file into one and `Close` writes it
///       back. The cost is the file's size in memory, and the population says that is the right
///       trade here: every `File` the BaseApp opens is a document, a report layout or an import,
///       none of them the 100-million-row table the streaming rule was written for (board:0045).
///
/// \warning WHAT NEEDS A CLIENT STILL REFUSES. `Download`, `Upload` and `View` move a file
///          between the server and a browser, and there is no browser (board:0030). They are the
///          only members here that still say so.
class File {
public:
  /// \brief A file variable that names nothing yet.
  File() = default;

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
  ::agiru::Boolean Create(std::string_view Name, const ::agiru::TextEncoding &Encoding = {});

  /// \brief AL `File.CreateInStream(InStream)`. Creates an InStream object for a file. This enables
  /// you to import or read data from the file.
  /// \param InStream The AL `InStream`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateInStream(::agiru::InStream &InStream);

  /// \brief AL `File.CreateInStream(InStream, TextEncoding)` -- the encoding the file is read in.
  /// \param InStream The stream.
  /// \param Encoding The encoding.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateInStream(::agiru::InStream &InStream, const ::agiru::TextEncoding &Encoding);

  /// \brief AL `File.CreateOutStream(OutStream)`. Creates an OutStream object for a file. This
  /// enables you to export or write data to the file.
  /// \param OutStream The AL `OutStream`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateOutStream(::agiru::OutStream &OutStream);

  /// \brief AL `File.CreateOutStream(OutStream, TextEncoding)` -- the encoding the file is written
  ///        in.
  /// \param OutStream The stream.
  /// \param Encoding  The encoding.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void CreateOutStream(::agiru::OutStream &OutStream, const ::agiru::TextEncoding &Encoding);

  /// \brief AL `File.CreateTempFile(TextEncoding)`. Creates a temporary file. This enables you to
  /// save data of any format to a temporary file. This file has a unique name and will be stored in
  /// a temporary file folder.
  /// \param Encoding The AL `TextEncoding`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `File.CreateTempFile()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] File.CreateTempFile([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean CreateTempFile();

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
                                   ::agiru::Text<0> &ToFile);

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
                                             ::agiru::Text<0> &ToFile);

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
  /// \brief AL `File.GetStamp` without its optional argument(s), which the
  ///        documentation brackets.
  /// \return The value.
  /// \throws Error always -- the surface is declared, the behaviour is not.
  /// \brief AL `File.GetStamp` without its optional argument(s), which the
  ///        documentation brackets.
  /// \return The value.
  /// \throws Error always -- the surface is declared, the behaviour is not.
  static ::agiru::Boolean GetStamp(std::string_view Name);

  static ::agiru::Boolean GetStamp(std::string_view Name, ::agiru::Date &Date);

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
  ::agiru::Boolean Open(std::string_view Name, const ::agiru::TextEncoding &Encoding = {});

  /// \brief AL `File.Pos()`. Gets the current position of the file pointer in an ASCII or binary
  /// file.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Integer Pos() const;

  /// \brief AL `File.Read(Any)`. Reads from an MS-DOS encoded file or binary file.
  /// \param Read The AL `Any`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Read(::agiru::Variant &Read);

  /// \brief AL `File.Read(var Text)` -- a line in text mode.
  /// \tparam T The text's type, which must assign from a `std::string_view`.
  /// \param Read Receives the line.
  /// \return How many bytes were read, the line break counted.
  template <typename T>
    requires requires(T &into) { into = std::string_view{}; } &&
             (!std::is_same_v<std::remove_cvref_t<T>, ::agiru::Variant>)::agiru::Integer
  Read(T &Read) {
    ::agiru::Variant held;
    const ::agiru::Integer read = this->Read(held);
    Read = std::string_view(held.Get<std::string>());
    return read;
  }

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
  static ::agiru::Boolean
  SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time = {});

  /// \brief AL `File.TextMode(Boolean)`. Sets whether a file should be opened as an ASCII file or a
  /// binary file. Gets the current setting of this option for a file.
  /// \param Mode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `File.TextMode()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] File.TextMode([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Boolean TextMode() const;

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
                                 ::agiru::Text<0> &ToFile);

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
                                           ::agiru::Text<0> &FromFile,
                                           ::agiru::InStream &InStream);

  /// \brief AL `File.View(Text, Boolean)`. Opens a file from server computer on the client computer
  /// in preview mode. The client computer is the computer that is running the browser that accesses
  /// the web client.
  /// \param FromFile The AL `Text`.
  /// \param AllowDownloadAndPrint The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean View(std::string_view FromFile,
                               ::agiru::Boolean AllowDownloadAndPrint = {});

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
                                         ::agiru::Boolean AllowDownloadAndPrint = {});

  /// \brief What a `File` reads as when it reaches an `Any`.
  /// \return Never.
  /// \throws Error always -- a file is not a value AL renders.
  ///
  /// \note IT EXISTS SO THAT `Variant := File` COMPILES AND REFUSES, which is what AL's `Any`
  ///       allows and what `WorkflowEngineUT` writes on purpose inside an `asserterror`.
  [[nodiscard]] std::string ToText() const;

  /// \brief AL `File.Write(Value)` for a TEXT value.
  ///
  /// \tparam T The value's type, which must read as a `std::string_view`.
  /// \param Value The value.
  ///
  /// \note IT IS TWO CONSTRAINED TEMPLATES AND NOT SEVENTEEN OVERLOADS, and the reason is an
  ///       ambiguity rather than brevity: `Guid` converts from a `Text`, so `File.Write(SomeText)`
  ///       matched the text overload and the GUID one equally well and the call had no answer.
  ///       The documented seventeen are one question -- text, or the platform's binary layout --
  ///       and the constraint is that question.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  void Write(const T &Value) {
    WriteLine(std::string_view(Value));
  }

  /// \brief AL `File.Write(Value)` for a value that is not text.
  /// \tparam T The value's type.
  /// \param Value The value.
  /// \throws Error always.
  /// \warning REFUSED. A typed `Write` puts the platform's own BINARY layout into the file, and
  ///          inventing one would produce a file that reads back wrong wherever BC reads it.
  template <typename T>
    requires(!std::convertible_to<const T &, std::string_view>)
  void Write(const T &Value) {
    static_cast<void>(Value);
    RefuseTyped();
  }

  /// \brief AL `File.WriteMode(Boolean)`. Use this method before you use OPEN method (File)] to set
  /// or test whether you can write to a file in later calls.
  /// \param Mode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `File.WriteMode()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] File.WriteMode([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Boolean WriteMode() const;

  ::agiru::Boolean WriteMode(::agiru::Boolean Mode);

private:
  void Bind(std::string_view name, bool truncate);

  void WriteLine(std::string_view text);

  [[noreturn]] static void RefuseTyped();

  ::agiru::Blob held_;
  std::string name_;
  std::size_t position_ = 0;
  bool open_ = false;
  bool textMode_ = true;
  bool writeMode_ = true;
};

}
