#pragma once

#include "type/Integer.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace agiru::detail {

class SessionRandom {
public:
  explicit SessionRandom(Integer seed);
  void Reseed(Integer seed);
  [[nodiscard]] Integer Bounded(Integer maximum);

private:
  [[nodiscard]] std::int64_t Sample();
  static constexpr std::int64_t kBig = 2147483647;
  static constexpr std::int64_t kSeed = 161803398;
  static constexpr std::size_t kLag = 55;
  static constexpr std::size_t kSecondLag = 30;
  static constexpr std::size_t kStride = 21;
  static constexpr std::size_t kSlots = kLag + 1;
  static constexpr int kWarmups = 5;
  std::array<std::int64_t, kSlots> state_{};
  std::size_t next_ = 0;
  std::size_t nextp_ = 0;
};

}
