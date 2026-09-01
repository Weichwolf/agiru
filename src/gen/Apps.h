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

} // namespace agiru::gen
