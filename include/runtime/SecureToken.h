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

/// \brief HMAC-SHA-256 of exact bytes with an exact secret key, using the private provider.
/// \param secret Independent high-entropy key bytes; never a password or public identifier.
/// \param domain Exact domain/message bytes, including NUL; no normalization.
/// \return Sixty-four lowercase hexadecimal characters; no weak provider fallback.
/// \throws Error if the MAC provider fails; native backend only, WASM remains unqualified.
[[nodiscard]] std::string SecureTokenMac(std::string_view secret, std::string_view domain);

/// \brief Compares equal-length secret bytes without content-dependent early exits.
/// \param expected Trusted exact bytes. \param supplied Untrusted exact bytes.
/// \return False for different lengths; otherwise the private provider's constant-time equality.
/// \note Length is public; no normalization. Native provider, WASM remains unqualified.
[[nodiscard]] bool SecureTokenEqual(std::string_view expected, std::string_view supplied);

}
