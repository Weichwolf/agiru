#pragma once

#include <string>
#include <string_view>

/// \file
/// \brief The .NET type `System.IO.Path` -- text operations on a file path.

namespace agiru::dotnet {

/// \brief .NET `System.IO.Path`, the eleven members the BaseApp names (counted over BCApps,
///        2026-09-09: `GetDirectoryName` 4, `GetInvalidFileNameChars` 3,
///        `GetFileNameWithoutExtension` 2, the rest once each).
///
/// \note A SEPARATOR IS `\` OR `/`. BC's service tier runs on Windows, where .NET accepts both,
///       and the BaseApp's own paths are spelled both ways (`FileManagement.CombinePath`), so the
///       split points are the same on either platform.
///
/// \note `GetDirectoryName` answers empty where .NET answers null -- a path with no separator, or
///       a root -- because AL has no null text and reads `''` there.
class Path {
public:
  /// \brief A path helper; it holds nothing.
  Path() = default;

  /// \brief .NET `Path.GetFileName(path)`: what follows the last separator.
  /// \param path The path.
  /// \return The file name with its extension, or the whole path when there is no separator.
  [[nodiscard]] static std::string GetFileName(std::string_view path);

  /// \brief .NET `Path.GetFileNameWithoutExtension(path)`.
  /// \param path The path.
  /// \return The file name up to its last full stop.
  [[nodiscard]] static std::string GetFileNameWithoutExtension(std::string_view path);

  /// \brief .NET `Path.GetExtension(path)`: the last full stop of the file name and what follows.
  /// \param path The path.
  /// \return `.txt` for `a\b.txt`; empty when the name has no full stop or ends in one.
  [[nodiscard]] static std::string GetExtension(std::string_view path);

  /// \brief .NET `Path.HasExtension(path)`.
  /// \param path The path.
  /// \return True when `GetExtension` is not empty.
  [[nodiscard]] static bool HasExtension(std::string_view path);

  /// \brief .NET `Path.GetDirectoryName(path)`: everything before the last separator.
  /// \param path The path.
  /// \return The directory without its trailing separator; empty when there is none.
  [[nodiscard]] static std::string GetDirectoryName(std::string_view path);

  /// \brief .NET `Path.ChangeExtension(path, extension)`.
  /// \param path      The path.
  /// \param extension The new extension, with or without its full stop; empty removes it.
  /// \return The path with the extension replaced.
  [[nodiscard]] static std::string ChangeExtension(std::string_view path,
                                                   std::string_view extension);

  /// \brief .NET `Path.Combine(path1, path2)`.
  /// \param first  The first part.
  /// \param second The second; when it is rooted it is the whole answer.
  /// \return The two joined by one separator.
  [[nodiscard]] static std::string Combine(std::string_view first, std::string_view second);

  /// \brief .NET `Path.GetRandomFileName()`: an 8.3 name of random letters and digits.
  /// \return The name, with no directory.
  [[nodiscard]] static std::string GetRandomFileName();

  /// \brief .NET `Path.GetTempFileName()`: a new empty file in the temporary directory.
  /// \return Its full path.
  /// \throws Error when the file cannot be made.
  [[nodiscard]] static std::string GetTempFileName();

  /// \brief .NET `Path.GetInvalidFileNameChars()`, as one text.
  /// \return The characters Windows refuses in a file name: `" < > | : * ? \ /` and the controls.
  [[nodiscard]] static std::string GetInvalidFileNameChars();

  /// \brief .NET `Path.GetInvalidPathChars()`, as one text.
  /// \return The characters Windows refuses in a path: `|` and the controls.
  [[nodiscard]] static std::string GetInvalidPathChars();
};

}
