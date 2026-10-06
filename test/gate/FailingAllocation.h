#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>

#include <dlfcn.h>

namespace gate {

inline thread_local bool failAllocation = false;

namespace {

template <typename Operator> Operator NextAllocationOperator(const char *name) {
  auto operation = reinterpret_cast<Operator>(dlsym(RTLD_NEXT, name));
  if (operation == nullptr) { std::abort(); }
  return operation;
}

}

}

void *operator new(std::size_t size) {
  if (std::exchange(gate::failAllocation, false)) { throw std::bad_alloc(); }
  static const auto allocate = gate::NextAllocationOperator<void *(*)(std::size_t)>("_Znwm");
  return allocate(size);
}

void operator delete(void *memory) noexcept {
  static const auto release = gate::NextAllocationOperator<void (*)(void *)>("_ZdlPv");
  release(memory);
}

void operator delete(void *memory, std::size_t size) noexcept {
  static const auto release =
      gate::NextAllocationOperator<void (*)(void *, std::size_t)>("_ZdlPvm");
  release(memory, size);
}
