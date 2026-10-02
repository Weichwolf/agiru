#include "type/SecretText.h"
#include "type/Text.h"

agiru::SecretText ImplicitSecret(const agiru::Text<0> &plain) {
  return plain;
}

agiru::SecretText ImplicitLiteralSecret() {
  return "secret";
}
