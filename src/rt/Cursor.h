#pragma once

#include "meta/TableDef.h"
#include "runtime/Database.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

class Cursor {
public:
  Cursor(const Connection &connection,
         const std::string &select,
         std::vector<std::optional<std::string>> binds);

  Cursor(const Cursor &) = delete;
  Cursor &operator=(const Cursor &) = delete;
  Cursor(Cursor &&) = delete;
  Cursor &operator=(Cursor &&) = delete;

  ~Cursor();

  [[nodiscard]] bool Step();

  [[nodiscard]] std::optional<std::string_view> Value(std::size_t column) const;

  [[nodiscard]] std::size_t Held() const { return block_.Rows(); }

private:
  bool Fetch();

  const Connection *connection_;
  std::string name_;
  Result block_;
  std::size_t row_ = 0;
  bool spent_ = false;
};

inline constexpr std::size_t kFetchBlock = 64;

}
