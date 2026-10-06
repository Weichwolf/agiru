#include "runtime/SecureToken.h"

#include "runtime/ErrorValue.h"

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace agiru {

namespace {

constexpr std::size_t kSecretBytes = 32;
constexpr std::string_view kHex = "0123456789abcdef";
constexpr unsigned kNibbleMask = 0x0F;

class SecretBytes {
public:
  ~SecretBytes() { OPENSSL_cleanse(bytes_.data(), bytes_.size()); }

  std::span<unsigned char> Bytes() { return bytes_; }

private:
  std::array<unsigned char, kSecretBytes> bytes_{};
};

std::string Hex(std::span<const unsigned char> bytes) {
  std::string result;
  result.reserve(bytes.size() * 2);
  for (const auto byte : bytes) {
    result += kHex[byte >> 4];
    result += kHex[byte & kNibbleMask];
  }
  return result;
}

}

std::string GenerateSecureToken() {
  SecretBytes storage;
  const auto bytes = storage.Bytes();
  if (RAND_priv_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1) {
    throw Error("secure token entropy is unavailable", "SecureTokenProvider");
  }
  return Hex(bytes);
}

std::string SecureTokenDigest(std::string_view secret) {
  std::array<unsigned char, kSecretBytes> digest{};
  unsigned length = 0;
  const int status =
      EVP_Digest(secret.data(), secret.size(), digest.data(), &length, EVP_sha256(), nullptr);
  if (status != 1 || length != digest.size()) {
    throw Error("secure token digest is unavailable", "SecureTokenProvider");
  }
  return Hex(digest);
}

}
