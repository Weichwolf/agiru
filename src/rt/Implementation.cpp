#include "runtime/Implementation.h"

#include "runtime/Error.h"

#include <cctype>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace agiru::detail {

namespace {

std::string Folded(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return out;
}

std::string ValueKey(std::string_view enumName, std::int32_t ordinal, std::string_view face) {
  return Folded(enumName) + "|" + std::to_string(ordinal) + "|" + Folded(face);
}

std::map<std::string, ForeignImplementation> &ByValue() {
  static std::map<std::string, ForeignImplementation> registry;
  return registry;
}

std::map<std::pair<std::type_index, std::string>, ForeignImplementation> &ByType() {
  static std::map<std::pair<std::type_index, std::string>, ForeignImplementation> registry;
  return registry;
}

}

void RegisterImplementation(std::string_view enumName,
                            std::int32_t ordinal,
                            std::string_view interfaceName,
                            const std::type_info &held,
                            ForeignImplementation made) {
  ByValue().insert_or_assign(ValueKey(enumName, ordinal, interfaceName), made);
  ByType().insert_or_assign({std::type_index(held), Folded(interfaceName)}, made);
}

const ForeignImplementation *FindImplementation(std::string_view enumName,
                                                std::int32_t ordinal,
                                                std::string_view interfaceName) {
  const auto &registry = ByValue();
  const auto found = registry.find(ValueKey(enumName, ordinal, interfaceName));
  return found == registry.end() ? nullptr : &found->second;
}

void *CloneForeign(const std::type_info &held,
                   std::string_view interfaceName,
                   const void *instance) {
  const auto &registry = ByType();
  const auto found = registry.find({std::type_index(held), Folded(interfaceName)});
  if (found == registry.end()) {
    throw Error("no app registered " + std::string(held.name()) + " as an implementation of " +
                std::string(interfaceName));
  }
  return found->second.clone(instance);
}

}
