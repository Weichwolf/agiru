#pragma once

#include "dotnet/Refused.h"
#include "type/Stream.h"
#include "type/Text.h"

#include <concepts>
#include <type_traits>

namespace agiru::dotnet {

/// \brief .NET `System.IO.BinaryReader` over an AL `InStream`, the reader of what `BinaryWriter`
///        writes: `BinReader := BinReader.BinaryReader(InStream); Note := BinReader.ReadString()`.
class BinaryReader {
public:
  /// \brief `BinaryReader.BaseStream`, of which AL reads `Position` and `Length` to see whether
  ///        there is anything to read at all.
  class Stream {
  public:
    /// \brief The stream seen through the reader.
    /// \param input The bound stream.
    explicit Stream(const InStream &input) : input_(&input) {}
    /// \brief .NET `Stream.Position`, zero-based.
    /// \return Where the next read starts.
    [[nodiscard]] ::agiru::Integer Position() const { return input_->Position() - 1; }
    /// \brief .NET `Stream.Length`.
    /// \return The number of bytes in the stream.
    [[nodiscard]] ::agiru::Integer Length() const { return input_->Length(); }

    /// \brief Becomes the absent .NET `Stream` a wrapper codeunit stores it in: an empty stub,
    ///        whose every member refuses (board:0035).
    /// \tparam T The stub.
    /// \return An empty one.
    template <typename T>
      requires ::agiru::dotnet::IsAbsent<T>
    operator T() const {
      return T{};
    }

  private:
    const InStream *input_;
  };

  /// \param input The stream read from; it must outlive the reader.
  /// \brief The binder behind the constructor call: `BinReader := BinReader.BinaryReader(...)`.
  ///
  /// \warning IT IS A DATA MEMBER NAMED AFTER THE CLASS, the shape every absent .NET stub already
  ///          has, so the generated call `X.X(stream)` needs no rule of its own: a class with no
  ///          user-declared constructor may carry a member of its own name, and a constructor here
  ///          would forbid it.
  struct Binder {
    /// \brief Binds a reader to the stream.
    /// \param input The stream; it must outlive the reader.
    /// \return The bound reader.
    [[nodiscard]] class BinaryReader operator()(InStream &input) const;

    /// \brief The .NET constructor over a .NET stream this runtime has not rebuilt (`Stream`,
    ///        `Encoding`): a reader bound to nothing, which refuses at its first use.
    /// \tparam Arguments Whatever the wrapper codeunit hands over.
    /// \return An unbound reader.
    template <typename... Arguments>
      requires(sizeof...(Arguments) != 1 || !(std::same_as<std::remove_cvref_t<Arguments>, InStream> && ...))
    [[nodiscard]] class BinaryReader operator()(Arguments &&...) const {
      return ::agiru::dotnet::BinaryReader{};
    }
  };

  /// \brief `BinaryReader.BinaryReader(stream)`, the constructor as AL calls it.
  Binder BinaryReader;

  /// \brief `BinaryReader.BaseStream`.
  /// \return The stream's position and length.
  /// \throws Error when the reader was never bound.
  [[nodiscard]] Stream BaseStream() const;

  /// \brief `BinaryReader.ReadString()`: the length-prefixed UTF-8 string `BinaryWriter.Write` put
  ///        there.
  /// \return The text.
  /// \throws Error when the reader was never bound or the stream ends inside the string.
  [[nodiscard]] ::agiru::Text<0> ReadString();

  /// \brief `BinaryReader.Close()`: nothing to release.
  void Close() const {}

  /// \brief .NET `BinaryReader.Codeunit`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused Codeunit{{.type = "BinaryReader", .member = "Codeunit"}};
  /// \brief .NET `BinaryReader.Dispose`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused Dispose{{.type = "BinaryReader", .member = "Dispose"}};
  /// \brief .NET `BinaryReader.ReadBoolean`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadBoolean{{.type = "BinaryReader", .member = "ReadBoolean"}};
  /// \brief .NET `BinaryReader.ReadByte`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadByte{{.type = "BinaryReader", .member = "ReadByte"}};
  /// \brief .NET `BinaryReader.ReadBytes`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadBytes{{.type = "BinaryReader", .member = "ReadBytes"}};
  /// \brief .NET `BinaryReader.ReadChar`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadChar{{.type = "BinaryReader", .member = "ReadChar"}};
  /// \brief .NET `BinaryReader.ReadChars`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadChars{{.type = "BinaryReader", .member = "ReadChars"}};
  /// \brief .NET `BinaryReader.ReadDecimal`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadDecimal{{.type = "BinaryReader", .member = "ReadDecimal"}};
  /// \brief .NET `BinaryReader.ReadInt16`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadInt16{{.type = "BinaryReader", .member = "ReadInt16"}};
  /// \brief .NET `BinaryReader.ReadInt32`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadInt32{{.type = "BinaryReader", .member = "ReadInt32"}};
  /// \brief .NET `BinaryReader.ReadUInt16`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadUInt16{{.type = "BinaryReader", .member = "ReadUInt16"}};
  /// \brief .NET `BinaryReader.ReadUInt32`, named by the BaseApp's wrapper codeunit and not rebuilt: a refusal
  ///        that says so when it is called (board:0035).
  ::agiru::dotnet::Refused ReadUInt32{{.type = "BinaryReader", .member = "ReadUInt32"}};

private:
  InStream *input_ = nullptr;
};

inline class BinaryReader BinaryReader::Binder::operator()(InStream &input) const {
  class BinaryReader bound;
  bound.input_ = &input;
  return bound;
}

}
