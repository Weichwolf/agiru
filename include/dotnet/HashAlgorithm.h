#pragma once

#include "type/Boolean.h"
#include "type/Integer.h"

#include <memory>
#include <string_view>

namespace agiru::dotnet {

class Array;

/// \brief Named .NET MD5/SHA1/SHA256/SHA384/SHA512 byte-array hashing through private OpenSSL.
/// Copies retain reference identity; disposing one invalidates every alias. Each successful
/// computation returns independently owned bytes and resets the digest for the next call.
/// \warning Keyed hashes, stream/transform APIs, configurable CryptoConfig mappings and
/// null-versus-empty System.Array identity remain separate gaps. WASM is not qualified.
class HashAlgorithm {
public:
  /// \brief A declared, unbound .NET reference; no provider allocation.
  HashAlgorithm() = default;

  /// \brief Create a named algorithm using the built-in, case-insensitive .NET aliases.
  /// \param name Algorithm or qualified implementation name; whitespace is significant.
  /// \return A fresh reference, or null for an unknown name.
  /// \throws Error for a known unsupported keyed algorithm or unavailable provider.
  [[nodiscard]] static HashAlgorithm Create(std::string_view name);

  /// \brief The parameterless factory is unsupported on .NET 5 and later.
  /// \throws Error rather than silently choosing a digest.
  [[nodiscard]] static HashAlgorithm Create();

  /// \brief Hash all bytes, including zero bytes; never add a BOM or text conversion.
  /// \param bytes Integer byte cells in [0,255]. \return Owned digest byte cells.
  /// \throws Error for an unbound/disposed reference, nonbyte cells or provider failure.
  [[nodiscard]] Array ComputeHash(const Array &bytes) const;

  /// \brief Hash an exact zero-based region without copying the complete input.
  /// \param bytes Byte cells. \param offset First cell. \param count Number of cells.
  /// \return Owned digest byte cells; a zero-length region hashes the empty input.
  /// \throws Error for a negative/out-of-range region or any ComputeHash failure.
  [[nodiscard]] Array ComputeHash(const Array &bytes, Integer offset, Integer count) const;

  /// \brief Release the shared provider; repeated disposal is harmless, hashing then fails.
  /// \throws Error for an unbound reference; does not change null identity on a bound reference.
  void Dispose();

  /// \brief Whether no algorithm reference is bound; a disposed reference is not null.
  /// \return True for an unbound reference or an unknown factory result.
  [[nodiscard]] Boolean IsNullObject() const { return state_ == nullptr; }

private:
  struct State;
  std::shared_ptr<State> state_;
};

}
