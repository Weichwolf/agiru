#include "SessionRandom.h"

#include "type/Integer.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace agiru::detail {

SessionRandom::SessionRandom(Integer seed) {
  Reseed(seed);
}

void SessionRandom::Reseed(Integer seed) {
  const std::int64_t subtraction = seed == std::numeric_limits<Integer>::min()
                                       ? std::numeric_limits<Integer>::max()
                                       : std::abs(static_cast<std::int64_t>(seed));
  std::int64_t mj = kSeed - subtraction;
  state_[kLag] = mj;
  std::int64_t mk = 1;
  for (std::size_t index = 1; index < kLag; ++index) {
    const std::size_t slot = (kStride * index) % kLag;
    state_[slot] = mk;
    mk = mj - mk;
    if (mk < 0) { mk += kBig; }
    mj = state_[slot];
  }
  for (int round = 1; round < kWarmups; ++round) {
    for (std::size_t index = 1; index < kSlots; ++index) {
      state_[index] -= state_[1 + ((index + kSecondLag) % kLag)];
      if (state_[index] < 0) { state_[index] += kBig; }
    }
  }
  next_ = 0;
  nextp_ = kStride;
}

Integer SessionRandom::Bounded(Integer maximum) {
  const double drawn = static_cast<double>(Sample()) * (1.0 / static_cast<double>(kBig));
  return static_cast<Integer>(drawn * static_cast<double>(maximum)) + 1;
}

std::int64_t SessionRandom::Sample() {
  std::size_t at = next_ + 1;
  if (at >= kSlots) { at = 1; }
  std::size_t other = nextp_ + 1;
  if (other >= kSlots) { other = 1; }
  std::int64_t drawn = state_[at] - state_[other];
  if (drawn == kBig) { drawn -= 1; }
  if (drawn < 0) { drawn += kBig; }
  state_[at] = drawn;
  next_ = at;
  nextp_ = other;
  return drawn;
}

}
