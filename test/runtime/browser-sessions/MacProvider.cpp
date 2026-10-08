#include <cstddef>
#include <cstdlib>
#include <string_view>

#include <openssl/evp.h>

extern "C" unsigned char *EVP_Q_mac(OSSL_LIB_CTX *libctx,
                                    const char *name,
                                    const char *propq,
                                    const char *subalg,
                                    const OSSL_PARAM *params,
                                    const void *key,
                                    std::size_t keylen,
                                    const unsigned char *data,
                                    std::size_t datalen,
                                    unsigned char *out,
                                    std::size_t outsize,
                                    std::size_t *outlen) {
  if (libctx != nullptr || name == nullptr || std::string_view(name) != "HMAC" ||
      propq != nullptr || subalg == nullptr || std::string_view(subalg) != "SHA256" ||
      params != nullptr) {
    std::abort();
  }
  static_cast<void>(key);
  static_cast<void>(keylen);
  static_cast<void>(data);
  static_cast<void>(datalen);
  if (out != nullptr && outsize > 0) { out[0] = 0; }
  if (outlen != nullptr) { *outlen = 0; }
  return nullptr;
}
