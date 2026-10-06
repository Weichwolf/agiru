#pragma once

#include <string>
#include <string_view>

/// \file
/// \brief Private-provider cryptography for opaque client credentials and handles.

namespace agiru {

/// \brief Generates 256 random bits using the system OpenSSL private CSPRNG.
/// \return Sixty-four lowercase hexadecimal characters; no UUID or simulation PRNG.
/// \throws Error if secure entropy is unavailable; never falls back to weak randomness.
/// \note Native provider only; browser/WASM entropy remains unqualified.
[[nodiscard]] std::string GenerateSecureToken();

/// \brief SHA-256 of exact bytes for storing high-entropy credential verifiers.
/// \param secret Exact bytes, including NUL; no normalization or text conversion.
/// \return Sixty-four lowercase hexadecimal digest characters.
/// \throws Error if the digest provider fails.
/// \warning Not password hashing. Caller must supply independently generated secure secrets.
[[nodiscard]] std::string SecureTokenDigest(std::string_view secret);

}
