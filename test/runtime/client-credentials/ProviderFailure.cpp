#include <cstddef>

#include <openssl/evp.h>
#include <openssl/rand.h>

extern "C" int RAND_priv_bytes(unsigned char *buf, int num) {
  if (buf != nullptr && num > 0) { buf[0] = 0; }
  return 0;
}

extern "C" int EVP_Digest(const void *data,
                          std::size_t count,
                          unsigned char *md,
                          unsigned *size,
                          const EVP_MD *type,
                          ENGINE *impl) {
  static_cast<void>(data);
  static_cast<void>(count);
  if (md != nullptr) { md[0] = 0; }
  if (size != nullptr) { *size = 0; }
  static_cast<void>(type);
  static_cast<void>(impl);
  return 0;
}
