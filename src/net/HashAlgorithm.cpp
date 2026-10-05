#include "dotnet/HashAlgorithm.h"

#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "ByteArray.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include <openssl/evp.h>

namespace agiru::dotnet {

namespace {

constexpr std::size_t kInputBlockBytes = 4096;

std::string LowerAscii(std::string_view name) {
  std::string result(name);
  for (char &unit : result) {
    if (unit >= 'A' && unit <= 'Z') { unit = static_cast<char>(unit + ('a' - 'A')); }
  }
  return result;
}

std::string DigestName(std::string_view name) {
  std::string normalized = LowerAscii(name);
  constexpr std::string_view prefix = "system.security.cryptography.";
  const bool qualified = normalized.starts_with(prefix);
  if (qualified) { normalized.erase(0, prefix.size()); }
  if (normalized.starts_with("hmac") || normalized == "keyedhashalgorithm") {
    throw Error("HashAlgorithm.Create: keyed algorithms are not rebuilt (board:0035)");
  }
  if (qualified && normalized == "hashalgorithm") { return "SHA1"; }
  if (qualified && normalized.ends_with("cryptoserviceprovider")) {
    normalized.resize(normalized.size() - std::string_view("cryptoserviceprovider").size());
  } else if (qualified && normalized.ends_with("managed") && normalized != "md5managed") {
    normalized.resize(normalized.size() - std::string_view("managed").size());
  }
  if (normalized == "md5") { return "MD5"; }
  if (normalized == "sha1" || (!qualified && normalized == "sha")) { return "SHA1"; }
  if (normalized == "sha256" || (!qualified && normalized == "sha-256")) { return "SHA256"; }
  if (normalized == "sha384" || (!qualified && normalized == "sha-384")) { return "SHA384"; }
  if (normalized == "sha512" || (!qualified && normalized == "sha-512")) { return "SHA512"; }
  return {};
}

void UpdateDigest(EVP_MD_CTX &context, const Array &bytes, Integer offset, Integer count) {
  std::array<unsigned char, kInputBlockBytes> block{};
  Integer consumed = 0;
  while (consumed < count) {
    const auto length = std::min(static_cast<std::size_t>(count - consumed), block.size());
    detail::ReadByteBlock(bytes, offset + consumed, std::span(block).first(length));
    if (EVP_DigestUpdate(&context, block.data(), length) != 1) {
      throw Error("HashAlgorithm.ComputeHash: digest update failed");
    }
    consumed += static_cast<Integer>(length);
  }
}

}

struct HashAlgorithm::State {
  std::unique_ptr<EVP_MD, decltype(&EVP_MD_free)> digest{nullptr, EVP_MD_free};
};

HashAlgorithm HashAlgorithm::Create(std::string_view name) {
  HashAlgorithm result;
  const std::string digest = DigestName(name);
  if (digest.empty()) { return result; }
  result.state_ = std::make_shared<State>();
  result.state_->digest.reset(EVP_MD_fetch(nullptr, digest.c_str(), nullptr));
  if (result.state_->digest == nullptr) {
    throw Error("HashAlgorithm.Create: digest provider is unavailable");
  }
  return result;
}

HashAlgorithm HashAlgorithm::Create() {
  throw Error("HashAlgorithm.Create(): default cryptographic factory is not supported");
}

Array HashAlgorithm::ComputeHash(const Array &bytes) const {
  return ComputeHash(bytes, 0, bytes.Length());
}

Array HashAlgorithm::ComputeHash(const Array &bytes, Integer offset, Integer count) const {
  if (state_ == nullptr) { throw Error("HashAlgorithm.ComputeHash: reference is null"); }
  if (state_->digest == nullptr) { throw Error("HashAlgorithm.ComputeHash: object is disposed"); }
  detail::ValidateByteRegion(bytes, offset, count);
  const std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context{EVP_MD_CTX_new(),
                                                                        EVP_MD_CTX_free};
  if (context == nullptr || EVP_DigestInit_ex(context.get(), state_->digest.get(), nullptr) != 1) {
    throw Error("HashAlgorithm.ComputeHash: digest initialization failed");
  }
  UpdateDigest(*context, bytes, offset, count);
  std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
  unsigned int length = 0;
  if (EVP_DigestFinal_ex(context.get(), digest.data(), &length) != 1 || length > digest.size()) {
    throw Error("HashAlgorithm.ComputeHash: digest finalization failed");
  }
  Array result;
  for (unsigned int index = 0; index < length; ++index) {
    result.Add(Variant{Integer{digest[index]}});
  }
  return result;
}

void HashAlgorithm::Dispose() {
  if (state_ == nullptr) { throw Error("HashAlgorithm.Dispose: reference is null"); }
  state_->digest.reset();
}

}
