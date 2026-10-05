#include "NativeSource.h"

#include <cctype>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include <yyjson.h>

namespace agiru::gen {

bool IsAppGuid(std::string_view value) {
  constexpr std::size_t kGuidLength = 36;
  if (value.size() != kGuidLength) { return false; }
  for (std::size_t at = 0; at < value.size(); ++at) {
    const bool dash = at == 8 || at == 13 || at == 18 || at == 23;
    if (dash ? value[at] != '-' : std::isxdigit(static_cast<unsigned char>(value[at])) == 0) {
      return false;
    }
  }
  return true;
}

namespace {

std::string IdentityField(yyjson_val *root, std::string_view name, bool required = true) {
  yyjson_obj_iter iterator = yyjson_obj_iter_with(root);
  yyjson_val *found = nullptr;
  while (yyjson_val *key = yyjson_obj_iter_next(&iterator)) {
    if (std::string_view{yyjson_get_str(key), yyjson_get_len(key)} != name) { continue; }
    if (found != nullptr) {
      throw std::runtime_error("app.json has duplicate identity field " + std::string(name));
    }
    found = yyjson_obj_iter_get_val(key);
  }
  if (found == nullptr && !required) { return {}; }
  if (found == nullptr || !yyjson_is_str(found) || yyjson_get_len(found) == 0) {
    throw std::runtime_error("app.json lacks nonempty string identity field " + std::string(name));
  }
  return {yyjson_get_str(found), yyjson_get_len(found)};
}

}

NativeAppIdentity ParseAppIdentity(std::string_view text) {
  const std::unique_ptr<yyjson_doc, decltype(&yyjson_doc_free)> document{
      yyjson_read(text.data(), text.size(), YYJSON_READ_NOFLAG), &yyjson_doc_free};
  if (document == nullptr || !yyjson_is_obj(yyjson_doc_get_root(document.get()))) {
    throw std::runtime_error("app.json is not a valid JSON object");
  }
  auto *const root = yyjson_doc_get_root(document.get());
  NativeAppIdentity identity{.id = IdentityField(root, "id"),
                             .name = IdentityField(root, "name"),
                             .publisher = IdentityField(root, "publisher"),
                             .version = IdentityField(root, "version"),
                             .minimumRuntime = IdentityField(root, "runtime", false)};
  if (!IsAppGuid(identity.id)) { throw std::runtime_error("app.json has invalid app id"); }
  return identity;
}

}
