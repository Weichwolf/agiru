#include "runtime/test/Handlers.h"

#include <algorithm>
#include <set>
#include <span>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

struct Standing {
  std::span<const TestHandler> handlers;
  std::vector<std::string_view> declared;
  std::set<const TestHandler *> ran;
  bool installed = false;
};

Standing &Held() {
  static thread_local Standing held;
  return held;
}

bool Declared(const Standing &held, std::string_view name) {
  return std::ranges::any_of(held.declared, [name](std::string_view declared) {
    return declared == name;
  });
}

}

void HandlerTable::Install(std::span<const TestHandler> handlers,
                           std::span<const std::string_view> declared) {
  Standing &held = Held();
  held.handlers = handlers;
  held.declared.assign(declared.begin(), declared.end());
  held.ran.clear();
  held.installed = true;
}

std::vector<std::string_view> HandlerTable::Uninstall() {
  Standing &held = Held();
  std::vector<std::string_view> missed;
  for (const std::string_view name : held.declared) {
    const auto found = std::ranges::find_if(held.handlers, [name](const TestHandler &handler) {
      return handler.name == name;
    });
    if (found == held.handlers.end()) {
      missed.push_back(name);
      continue;
    }
    if (!held.ran.contains(&*found)) { missed.push_back(name); }
  }
  held = Standing{};
  return missed;
}

const TestHandler *HandlerTable::For(HandlerKind kind, std::int32_t object) {
  const Standing &held = Held();
  if (!held.installed) { return nullptr; }
  for (const TestHandler &handler : held.handlers) {
    if (handler.kind != kind) { continue; }
    if (object != 0 && handler.object != 0 && handler.object != object) { continue; }
    if (!Declared(held, handler.name)) { continue; }
    return &handler;
  }
  return nullptr;
}

void HandlerTable::Ran(const TestHandler &handler) { Held().ran.insert(&handler); }

bool HandlerTable::Installed() { return Held().installed; }

}
