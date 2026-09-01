#include <array>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <print>
#include <string_view>
#include <system_error>

namespace {

// DIE VIER REFERENZEN AUS CLAUDE.md, an ihren Pfaden. Fehlt eine, ist jede Aussage ueber
// AL-Semantik in diesem Lauf eine Vermutung -- deshalb sagt der Client es, statt es zu ertragen.
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
  // NOLINTNEXTLINE(concurrency-mt-unsafe): einmal in main, bevor der erste Thread existiert.
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
  // EIN FEHLSCHLAG IST LAUT. `main` darf nichts entkommen lassen, sonst endet der Prozess ueber
  // std::terminate ohne die Meldung, die sagt, was fehlschlug. Der Handler selbst darf nicht
  // werfen -- std::println kann es, std::fputs nicht.
  try {
    return Report();
  } catch (const std::exception &e) {
    std::fputs("agiru: ", stderr);
    std::fputs(e.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  } catch (...) {
    // Nicht geschluckt, sondern gemeldet: was nicht von std::exception kommt, traegt keinen Text,
    // aber einen Rueckgabewert. Diese Stelle zaehlt unten in der Baseline der stillen Stellen.
    std::fputs("agiru: unbekannte Ausnahme\n", stderr);
    return 1;
  }
}
