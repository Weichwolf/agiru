#pragma once

#include "dotnet/Refused.h"
#include "type/Stream.h"
#include "runtime/Error.h"
#include "type/Text.h"

#include <concepts>
#include <string_view>
#include <type_traits>

namespace agiru::dotnet {

/// \brief .NET `System.IO.BinaryWriter` over an AL `OutStream`, which the Record Link module uses
///        to write a note: `BinWriter := BinWriter.BinaryWriter(OutStream); BinWriter.Write(Note)`.
///
/// \note A .NET STRING IS LENGTH-PREFIXED. `Write(string)` puts the UTF-8 byte count first, seven
///       bits per byte with the high bit carrying on, then the bytes -- and `BinaryReader.ReadString`
///       reads that back. A note written by BC is read by this reader and the other way round, so the
///       layout is .NET's and not this runtime's.
class BinaryWriter {
public:
  /// \param output The stream written to; it must outlive the writer.
  /// \brief The binder behind the constructor call: `BinWriter := BinWriter.BinaryWriter(...)`.
  ///
  /// \warning IT IS A DATA MEMBER NAMED AFTER THE CLASS, the shape every absent .NET stub already
  ///          has, so the generated call `X.X(stream)` needs no rule of its own: a class with no
  ///          user-declared constructor may carry a member of its own name, and a constructor here
  ///          would forbid it.
  struct Binder {
    /// \brief Binds a writer to the stream.
    /// \param output The stream; it must outlive the writer.
    /// \return The bound writer.
    [[nodiscard]] class BinaryWriter operator()(OutStream &output) const;

    /// \brief The .NET constructor over a .NET stream this runtime has not rebuilt (`Stream`,
    ///        `Encoding`): a writer bound to nothing, which refuses at its first use.
    /// \tparam Arguments Whatever the wrapper codeunit hands over.
    /// \return An unbound writer.
    template <typename... Arguments>
      requires(sizeof...(Arguments) != 1 || !(std::same_as<std::remove_cvref_t<Arguments>, OutStream> && ...))
    [[nodiscard]] class BinaryWriter operator()(Arguments &&...) const {
      return ::agiru::dotnet::BinaryWriter{};
    }
  };

  /// \brief `BinaryWriter.BinaryWriter(stream)`, the constructor as AL calls it.
  Binder BinaryWriter;

  /// \brief `BinaryWriter.Write(string)`: a length-prefixed UTF-8 string.
  /// \param text The text.
  /// \throws Error when the writer was never bound.
  void Write(std::string_view text);

  /// \brief `BinaryWriter.Write(string)` for an AL `Text`.
  /// \tparam N The declared length.
  /// \param text The text.
  template <std::size_t N> void Write(const ::agiru::Text<N> &text) { Write(text.Value()); }

  /// \brief `BinaryWriter.Write` of any other .NET type (`Boolean`, `Decimal`, `Int32`, bytes):
  ///        the platform's binary layout, which this runtime does not carry.
  /// \tparam T The value's type.
  /// \throws Error always (board:0035).
  template <typename T>
    requires(!std::convertible_to<const T &, std::string_view>)
  void Write(const T &) {
    throw Error("BinaryWriter.Write of this type is declared and not implemented yet (board:0035)");
  }

  /// \brief `BinaryWriter.Close()`: nothing is buffered here, so nothing is left to flush.
  void Close() const {}

  /// \brief .NET `BinaryWriter.Codeunit`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused Codeunit{{.type = "BinaryWriter", .member = "Codeunit"}};
  /// \brief .NET `BinaryWriter.Dispose`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused Dispose{{.type = "BinaryWriter", .member = "Dispose"}};
  /// \brief .NET `BinaryWriter.Flush`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused Flush{{.type = "BinaryWriter", .member = "Flush"}};
  /// \brief .NET `BinaryWriter.Seek`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused Seek{{.type = "BinaryWriter", .member = "Seek"}};
  /// \brief .NET `BinaryWriter.BaseStream`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused BaseStream{{.type = "BinaryWriter", .member = "BaseStream"}};

private:
  OutStream *output_ = nullptr;
};

inline class BinaryWriter BinaryWriter::Binder::operator()(OutStream &output) const {
  class BinaryWriter bound;
  bound.output_ = &output;
  return bound;
}

}
