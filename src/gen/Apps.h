#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace agiru::gen {

/// One BC app: what it is called, where its AL lives, and what it may see.
struct App {
  std::string name;
  std::string source;
  std::vector<std::string> depends;
};

/// Reads apps.json. The apps come back in DECLARATION order, which is dependency order, and the
/// transpiler relies on that: an app resolves a name against itself and everything already read,
/// so a later app is invisible to an earlier one -- the direction AL declares, enforced by the
/// order of the loop rather than by a second check.
std::vector<App> ReadApps(const std::filesystem::path &path);

/// What of BC this tree translates, as a WHITELIST over AL namespaces.
///
/// A namespace is in scope when the LONGEST matching prefix -- case-insensitive, on a dot boundary
/// -- comes from `include`; an `exclude` entry wins over a shorter `include` prefix, which is what
/// makes a carve-out possible. No match at all is out of scope.
///
/// A file that declares NO namespace is decided by the top-level folder it sits in instead. W1's
/// test suite is 91 % namespace-less, so the folder is the only signal it has.
struct TranspileScope {
  std::vector<std::string> include;           ///< Namespaces this tree translates.
  std::vector<std::string> exclude;           ///< Carve-outs inside an included namespace.
  std::vector<std::string> areaExclude;       ///< Top-level folders of namespace-less sources.
  std::vector<std::string> areaExcludeSuffix; ///< Folder suffixes, `-Internal` among them.
};

/// Whether a declared namespace is in scope.
/// \param scope     The scope.
/// \param nameSpace The AL `namespace`, empty when the file declares none.
/// \return True when it is translated.
[[nodiscard]] bool Holds(const TranspileScope &scope, std::string_view nameSpace);

/// Whether a namespace-less source's folder is in scope.
/// \param scope The scope.
/// \param area  The top-level folder under the app's source root.
/// \return True when it is translated.
[[nodiscard]] bool HoldsArea(const TranspileScope &scope, std::string_view area);

/// Reads scope.json.
/// \param path The file.
/// \return The scope; an empty include list means everything is out of scope, which is refused.
TranspileScope ReadScope(const std::filesystem::path &path);

} // namespace agiru::gen
