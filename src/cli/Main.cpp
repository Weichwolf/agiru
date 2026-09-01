#include <cstdlib>
#include <filesystem>
#include <print>
#include <string_view>

namespace {

// DIE VIER REFERENZEN AUS CLAUDE.md, an ihren Pfaden. Fehlt eine, ist jede Aussage ueber
// AL-Semantik in diesem Lauf eine Vermutung -- deshalb sagt der Client es, statt es zu ertragen.
struct Reference {
  std::string_view what;
  std::string_view path;
};

constexpr Reference kReferences[] = {
    {"Plattform-Doku", "dynamics365smb-devitpro-pb/dev-itpro/developer"},
    {"AL-Quelltext", "BCApps/src"},
    {"Anwender-Doku", "dynamics365smb-docs"},
    {"Vorgaenger", "openerp"},
};

std::filesystem::path GitRoot() {
  const char* home = std::getenv("HOME");
  return std::filesystem::path(home ? home : ".") / "Git";
}

} // namespace

int main() {
  const std::filesystem::path git = GitRoot();
  std::println("agiru -- AL nach C++, Transpiler und Runtime");
  for (const Reference& r : kReferences) {
    const std::filesystem::path p = git / r.path;
    std::println("  {:<16} {:<52} {}", r.what, p.string(),
                 std::filesystem::exists(p) ? "da" : "FEHLT");
  }
  return 0;
}
