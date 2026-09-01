#include <array>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <print>
#include <string_view>
#include <system_error>

namespace {
struct Reference {
  std::string_view what;
  std::string_view path;
};

constexpr std::array kReferences = {
    Reference{.what = "Plattform-Doku", .path = "dynamics365smb-devitpro-pb/dev-itpro/developer"},
    Reference{.what = "AL-Quelltext", .path = "BCApps/src"},
    Reference{.what = "Anwender-Doku", .path = "dynamics365smb-docs"},
    Reference{.what = "Vorgaenger", .path = "openerp"},
};

std::filesystem::path GitRoot() {
  // NOLINTNEXTLINE(concurrency-mt-unsafe): once in main, before the first thread exists.
  const char *const home = std::getenv("HOME");
  return std::filesystem::path(home != nullptr ? home : ".") / "Git";
}

int Report() {
  const std::filesystem::path git = GitRoot();
  std::println("agiru -- AL nach C++, Transpiler und Runtime");
  for (const Reference &r : kReferences) {
    const std::filesystem::path p = git / r.path;
    std::error_code ec;
    std::println(
        "  {:<16} {:<52} {}", r.what, p.string(), std::filesystem::exists(p, ec) ? "da" : "FEHLT");
  }
  return 0;
}

} // namespace

int main() {
  try {
    return Report();
  } catch (const std::exception &e) {
    std::fputs("agiru: ", stderr);
    std::fputs(e.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  } catch (...) {
    std::fputs("agiru: unbekannte Ausnahme\n", stderr);
    return 1;
  }
}
