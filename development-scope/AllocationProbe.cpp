#include "Apps.h"
#include "Scope.h"

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>

namespace {
std::atomic<std::size_t> allocations{};
}

void *operator new(std::size_t size) {
  allocations.fetch_add(1, std::memory_order_relaxed);
  if (void *held = std::malloc(size == 0 ? 1 : size)) { return held; }
  throw std::bad_alloc{};
}
void *operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void *held) noexcept { std::free(held); }
void operator delete[](void *held) noexcept { std::free(held); }
void operator delete(void *held, std::size_t) noexcept { std::free(held); }
void operator delete[](void *held, std::size_t) noexcept { std::free(held); }

int main() {
  const auto rules = agiru::gen::ReadScope("scope.json");
  const auto scope = agiru::gen::Scope::FromFile("scope.json");
  constexpr std::size_t repeats = 1000;
  const auto before = allocations.load(std::memory_order_relaxed);
  std::size_t admitted = 0;
  for (std::size_t at = 0; at < repeats; ++at) {
    admitted += agiru::gen::Holds(rules, "MICROSOFT.Finance.GeneralLedger");
    admitted += scope.Contains("MICROSOFT.Finance.GeneralLedger");
  }
  const auto measured = allocations.load(std::memory_order_relaxed) - before;
  std::printf("namespace checks: %zu/%zu; allocations: %zu\n", admitted, 2 * repeats, measured);
  return admitted == 2 * repeats && measured == 0 ? 0 : 1;
}
