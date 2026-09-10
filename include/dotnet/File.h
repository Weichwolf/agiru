#pragma once

#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "dotnet/Regex.h"
#include "type/Stream.h"
#include "type/Text.h"

#include <string>
#include <string_view>

/// \file
/// \brief .NET `System.IO.File`, rebuilt: the static file questions the BaseApp asks beside AL's
///        own `File` type, which opens and reads a file rather than asking about one.

namespace agiru::dotnet {

/// \brief .NET `System.IO.File`.
///
/// \note IT IS THE STATIC CLASS AND NOT AL'S `File`. AL declares `SystemIOFile: DotNet File` and
///       calls `File.Exists(Name)` on it -- a question about the file system, where AL's `File`
///       is a handle on one open file. Both exist here for the same reason they exist in BC.
class File {
public:
  /// \brief The binder behind `F := F.File()`, which AL writes for any DotNet variable.
  struct Binder {
    /// \brief `new File()`. \return The value; the class is static and holds nothing.
    [[nodiscard]] class File operator()() const { return {}; }
  };

  /// \brief The constructor AL calls as a member.
  Binder File;

  /// \brief `File.Exists(path)`. \param path The file. \return Whether it is there.
  [[nodiscard]] static Boolean Exists(std::string_view path);

  /// \brief `File.Delete(path)`, which does nothing when the file is not there.
  /// \param path The file.
  static void Delete(std::string_view path);

  /// \brief `File.Copy(from, to, overwrite)`: .NET's three-argument form, which refuses when
  ///        the target is there and `overwrite` is false.
  /// \param from The source. \param to The target. \param overwrite Whether it may replace it.
  /// \throws Error when the copy fails, the existing target included.
  static void Copy(std::string_view from, std::string_view to, Boolean overwrite);

  /// \brief `File.Copy(from, to)`. \param from The source. \param to The target.
  /// \throws Error when the source cannot be read or the target cannot be written.
  static void Copy(std::string_view from, std::string_view to);

  /// \brief `File.Move(from, to)`. \param from The source. \param to The target.
  /// \throws Error when the move fails.
  static void Move(std::string_view from, std::string_view to);

  /// \brief `File.ReadAllText(path)`. \param path The file. \return Its contents.
  /// \throws Error when the file cannot be read.
  [[nodiscard]] static ::agiru::Text<0> ReadAllText(std::string_view path);

  /// \brief `File.WriteAllText(path, text)`. \param path The file. \param text What goes in it.
  /// \throws Error when the file cannot be written.
  static void WriteAllText(std::string_view path, std::string_view text);

  /// \brief `File.ReadAllLines(path)`: the file's lines, which a test walks to check an export.
  /// \param path The file. \return The lines as .NET hands them back, an `Array` of text.
  /// \throws Error when the file cannot be read.
  /// \note IT IS AN `Array` AND NOT A `List`, because AL declares what it assigns to as
  ///       `DotNet Array` and reads it with `GetValue`.
  [[nodiscard]] static ::agiru::dotnet::Array ReadAllLines(std::string_view path);

  /// \brief `File.AppendAllText(path, text)`. \param path The file. \param text What is added.
  /// \throws Error when the file cannot be written.
  static void AppendAllText(std::string_view path, std::string_view text);

  /// \brief `File.ReadAllBytes(path)`, refused: the array of bytes AL would hold is not rebuilt
  ///        (board:0035).
  ::agiru::dotnet::Refused ReadAllBytes{{.type = "File", .member = "ReadAllBytes"}};

  /// \brief `File.WriteAllBytes(path, bytes)`, refused for the same reason.
  ::agiru::dotnet::Refused WriteAllBytes{{.type = "File", .member = "WriteAllBytes"}};

  /// \brief `File.Open(path [, mode])`, refused: it hands back a .NET Stream (board:0035).
  ::agiru::dotnet::Refused Open{{.type = "File", .member = "Open"}};

  /// \brief `File.OpenRead(path)`, refused: it hands back a .NET Stream, and the AL side reads
  ///        through `InStream` instead (board:0035).
  ::agiru::dotnet::Refused OpenRead{{.type = "File", .member = "OpenRead"}};

  /// \brief `File.OpenText(path)`, refused for the same reason.
  ::agiru::dotnet::Refused OpenText{{.type = "File", .member = "OpenText"}};
};

}
