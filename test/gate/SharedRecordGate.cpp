#include "runtime/RecordRef.h"

#include "Check.h"
#include "FailingAllocation.h"

#include <new>
#include <utility>

using gate::failAllocation;

namespace {
int released = 0;

void ReleaseFixture([[maybe_unused]] void *record) {
  ++released;
}
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
  owner = {};
  CHECK_TRUE("reusing a moved owner cannot prematurely release the transferred record",
             released == 0 && owner.Get() == nullptr && moved.Get() == &record);
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
