#include "meta/Ids.h"
#include "runtime/Report.h"

#include "Check.h"

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace {

constexpr int kArgumentCount = 3;

int ReportNumber(std::string_view text) {
  int number = 0;
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), number);
  if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || number <= 0) {
    throw std::invalid_argument("expected a positive report number");
  }
  return number;
}

}

int main(int argc, char **argv) {
  return gate::Run("Shared Report Registry", [argc, argv] {
    if (argc != kArgumentCount) {
      throw std::invalid_argument("expected report number and original name");
    }
    const auto *entry = agiru::FindReport(agiru::ReportId{ReportNumber(argv[1])});
    CHECK_TRUE("registry-only lookup finds the linked native report", entry != nullptr);
    if (entry == nullptr) { return; }
    CHECK_TEXT("shared registration retains the source name", entry->name, argv[2]);
    CHECK_TRUE("shared registration retains the generated entrypoint", entry->run != nullptr);
  });
}
