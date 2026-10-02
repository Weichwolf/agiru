#include "runtime/RecordRef.h"

#include "Check.h"

#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>

namespace {
thread_local bool failAllocation = false;
int released = 0;

void ReleaseFixture(void *) {
  ++released;
}
}

// Replace allocation only in this gate process to exercise the real ownership boundary.
void *operator new(std::size_t size) {
  if (std::exchange(failAllocation, false)) { throw std::bad_alloc(); }
  if (void *memory = std::malloc(size == 0 ? 1 : size)) { return memory; }
  throw std::bad_alloc();
}

void operator delete(void *memory) noexcept {
  std::free(memory);
}

void operator delete(void *memory, std::size_t) noexcept {
  std::free(memory);
}

namespace {
void FailedAdoptionReleasesTheRecord() {
  int record = 0;
  released = 0;
  bool refused = false;
  failAllocation = true;
  try {
    const agiru::detail::SharedRecord owned(&record, &ReleaseFixture);
  } catch (const std::bad_alloc &) { refused = true; }
  CHECK_TRUE("the reference-count allocation failed", refused);
  CHECK_TRUE("failed adoption releases its incoming record", released == 1);
}

void TheLastOwnerReleasesExactlyOnce() {
  int record = 0;
  released = 0;
  agiru::detail::SharedRecord owner(&record, &ReleaseFixture);
  auto copy = owner;
  agiru::detail::SharedRecord moved(std::move(owner));
  CHECK_TRUE("move construction empties its source", owner.Get() == nullptr);
  moved.Reset();
  CHECK_TRUE("another owner keeps the record alive", released == 0 && copy.Get() == &record);
  copy.Reset();
  CHECK_TRUE("the last owner releases the record", released == 1);
  copy.Reset();
  CHECK_TRUE("repeated reset does not release twice", released == 1);
}
}

int main() {
  return gate::Run("SharedRecord", [] {
    FailedAdoptionReleasesTheRecord();
    TheLastOwnerReleasesExactlyOnce();
  });
}
