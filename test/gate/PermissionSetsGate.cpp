#include "meta/PermissionSetDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PermissionSets.h"

#include "Check.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace {

using Level = agiru::PermissionLevel;
using Rights = agiru::PermissionEntry::Rights;
using Identity = agiru::PermissionSetIdentity;
using Set = agiru::PermissionSetDef;
using Operation = agiru::PermissionOperation;
using Kind = agiru::PermissionObject;
constexpr auto kNone = Level::None;
constexpr auto kDirect = Level::Direct;
constexpr auto kIndirect = Level::Indirect;
constexpr std::string_view kApp = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kOtherApp = "00000000-0000-0000-0000-000000000002";
constexpr Identity kRoot{.app = kApp, .role = "ROOT"};
constexpr Identity kLeaf{.app = kApp, .role = "LEAF"};
constexpr Identity kRemove{.app = kApp, .role = "REMOVE"};
constexpr Identity kParent{.app = kApp, .role = "PARENT"};
constexpr Rights kAll{kDirect, kDirect, kDirect, kDirect, kDirect};
constexpr Rights kRead{kDirect};
constexpr std::int32_t kObject = 18;
constexpr std::uint8_t kUnknownOperation = static_cast<std::uint8_t>(Operation::Execute) + 1;

class Catalog final : public agiru::PermissionSetCatalog {
public:
  explicit Catalog(std::span<const Set> definitions) : definitions_(definitions) {}

  const Set *Find(const Identity &identity) const override {
    ++lookups_;
    const auto found = std::ranges::find(definitions_, identity, &Set::identity);
    return found == definitions_.end() ? nullptr : &*found;
  }

  [[nodiscard]] std::size_t Lookups() const { return lookups_; }

private:
  std::span<const Set> definitions_;
  mutable std::size_t lookups_ = 0;
};

class MismatchedCatalog final : public agiru::PermissionSetCatalog {
public:
  explicit MismatchedCatalog(const Set &definition) : definition_(definition) {}

  const Set *Find([[maybe_unused]] const Identity &identity) const override { return &definition_; }

private:
  const Set &definition_;
};

Level Resolve(const Catalog &catalog,
              std::span<const Identity> assigned,
              Operation operation = Operation::Read,
              agiru::PermissionSetLimits limits = {}) {
  return agiru::ResolvePermission(catalog, assigned, kObject, Kind::TableData, operation, limits);
}

template <typename Body>
void Refuses(std::string_view claim,
             Body body,
             std::string_view code = "PermissionPolicy",
             std::string_view text = {}) {
  bool refused = false;
  try {
    body();
  } catch (const agiru::Error &error) {
    refused = error.Code() == code && std::string_view(error.what()).contains(text);
  }
  CHECK_TRUE(claim, refused);
}

void DocumentedComposition(const Rights &own,
                           const Rights &other,
                           bool exclude,
                           const Rights &expected) {
  const std::array entries{agiru::PermissionEntry{.rights = own, .object = kObject}};
  const std::array inherited{agiru::PermissionEntry{.rights = other, .object = kObject}};
  constexpr std::array references{kLeaf};
  const std::array definitions{Set{.identity = kRoot,
                                   .permissions = entries,
                                   .included = exclude ? std::span<const Identity>{} : references,
                                   .excluded = exclude ? references : std::span<const Identity>{}},
                               Set{.identity = kLeaf, .permissions = inherited}};
  const Catalog catalog(definitions);
  constexpr std::array assigned{kRoot};
  for (std::size_t index = 0; index < expected.size(); ++index) {
    CHECK_TRUE("every documented include/exclude operation matches the BC truth table",
               Resolve(catalog, assigned, static_cast<Operation>(index)) == expected[index]);
  }
}

void LevelAlgebra() {
  DocumentedComposition({kDirect, kDirect},
                        {kNone, kIndirect, kDirect, kDirect},
                        false,
                        {kDirect, kDirect, kDirect, kDirect});
  DocumentedComposition({kDirect, kIndirect},
                        {kNone, kDirect, kDirect, kDirect},
                        false,
                        {kDirect, kDirect, kDirect, kDirect});
  DocumentedComposition({kDirect, kDirect, kDirect, kDirect},
                        {kNone, kIndirect, kDirect, kDirect},
                        true,
                        {kDirect, kDirect});
  DocumentedComposition(
      {kDirect, kIndirect, kDirect, kDirect}, {kNone, kDirect, kDirect, kDirect}, true, {kDirect});
  for (const auto included : {kNone, kIndirect, kDirect}) {
    for (const auto excluded : {kNone, kIndirect, kDirect}) {
      const Level expected = excluded == kDirect || included == excluded ? kNone : included;
      DocumentedComposition({included}, {excluded}, true, {expected});
    }
  }
}

void Hierarchy() {
  constexpr std::array all{agiru::PermissionEntry{.rights = kAll, .object = kObject}};
  constexpr std::array read{agiru::PermissionEntry{.rights = kRead, .object = kObject}};
  constexpr std::array leaf{kLeaf};
  constexpr std::array remove{kRemove};
  constexpr std::array root{kRoot};
  const std::array definitions{Set{.identity = kRoot, .included = leaf, .excluded = remove},
                               Set{.identity = kLeaf, .permissions = all},
                               Set{.identity = kRemove, .permissions = read},
                               Set{.identity = kParent, .permissions = read, .included = root}};
  const Catalog catalog(definitions);
  CHECK_TRUE("child exclusion removes its read grant", Resolve(catalog, root) == kNone);
  CHECK_TRUE("child exclusion preserves different operation grants",
             Resolve(catalog, root, Operation::Modify) == kDirect);
  constexpr std::array independentlyAssigned{kRoot, kLeaf};
  CHECK_TRUE("a child exclusion cannot delete a separately assigned role's rights",
             Resolve(catalog, independentlyAssigned) == kDirect);
  constexpr std::array parent{kParent};
  CHECK_TRUE("parent permissions can restore a child-local exclusion",
             Resolve(catalog, parent) == kDirect);
  CHECK_TRUE("empty assignments deny without a default SUPER", Resolve(catalog, {}) == kNone);
  CHECK_TRUE(
      "a role named SUPER has no synthetic unrestricted meaning",
      agiru::ResolvePermission(Catalog(std::array{Set{.identity = {.app = kApp, .role = "SUPER"}}}),
                               std::array{Identity{.app = kApp, .role = "SUPER"}},
                               kObject,
                               Kind::TableData,
                               Operation::Read) == kNone);
}

void WildcardsAndIdentity() {
  constexpr Identity other{.app = kOtherApp, .role = "ROOT"};
  constexpr Identity tenant{.app = kApp, .role = "ROOT", .scope = agiru::PermissionScope::Tenant};
  constexpr std::array wildcard{agiru::PermissionEntry{.rights = kAll}};
  constexpr std::array exact{agiru::PermissionEntry{.rights = kRead, .object = kObject}};
  constexpr std::array unrelated{
      agiru::PermissionEntry{.rights = kAll, .object = kObject, .kind = Kind::Page}};
  constexpr std::array remove{kRemove};
  const std::array definitions{Set{.identity = kRoot, .permissions = wildcard, .excluded = remove},
                               Set{.identity = kRemove, .permissions = exact},
                               Set{.identity = other, .permissions = exact},
                               Set{.identity = tenant, .permissions = unrelated}};
  const Catalog catalog(definitions);
  constexpr std::array root{kRoot};
  CHECK_TRUE("an exact exclusion removes the target from a wildcard grant",
             Resolve(catalog, root) == kNone);
  CHECK_TRUE("a wildcard grants a different nonexcluded object",
             agiru::ResolvePermission(
                 catalog, root, kObject + 1, Kind::TableData, Operation::Read) == kDirect);
  CHECK_TRUE("wildcards retain per-operation exclusions",
             Resolve(catalog, root, Operation::Insert) == kDirect);
  CHECK_TRUE("equal role names in different apps do not share exclusions",
             Resolve(catalog, std::array{other}) == kDirect);
  CHECK_TRUE("scope is part of the exact permission-set identity",
             Resolve(catalog, std::array{tenant}) == kNone);
  CHECK_TRUE("page and table-data numbers remain separate permission domains",
             agiru::ResolvePermission(
                 catalog, std::array{tenant}, kObject, Kind::Page, Operation::Execute) == kDirect);
  constexpr std::array wildcardRemoval{agiru::PermissionEntry{.rights = kRead}};
  const std::array reversed{Set{.identity = kRoot, .permissions = exact, .excluded = remove},
                            Set{.identity = kRemove, .permissions = wildcardRemoval}};
  CHECK_TRUE("wildcard exclusion also removes an exact grant",
             Resolve(Catalog(reversed), root) == kNone);
}

void ExtensionsAndFilters() {
  constexpr std::array indirect{agiru::PermissionEntry{.rights = {kIndirect}, .object = kObject}};
  constexpr std::array direct{agiru::PermissionEntry{.rights = kRead, .object = kObject}};
  constexpr std::array leaf{kLeaf};
  constexpr std::array remove{kRemove};
  const std::array extensions{
      agiru::PermissionSetExtensionDef{.permissions = indirect, .included = leaf}};
  const std::array definitions{
      Set{.identity = kRoot, .extensions = extensions},
      Set{.identity = kLeaf, .permissions = direct},
      Set{.identity = kParent, .excluded = remove, .extensions = extensions},
      Set{.identity = kRemove, .permissions = direct}};
  const Catalog catalog(definitions);
  CHECK_TRUE("extension includes participate in direct-over-indirect composition",
             Resolve(catalog, std::array{kRoot}) == kDirect);
  CHECK_TRUE("owner exclusions apply to extension contributions at the same level",
             Resolve(catalog, std::array{kParent}) == kNone);
  CHECK_TRUE("indirect stays indirect and never becomes an execution grant",
             Resolve(Catalog(std::array{Set{.identity = kRoot, .permissions = indirect}}),
                     std::array{kRoot}) == kIndirect);
  constexpr std::array filtered{
      agiru::PermissionEntry{.rights = kRead, .object = kObject, .securityFiltered = true}};
  const std::array policies{Set{.identity = kRoot, .permissions = filtered},
                            Set{.identity = kLeaf, .permissions = direct, .excluded = remove},
                            Set{.identity = kRemove, .permissions = filtered}};
  const Catalog filteredCatalog(policies);
  Refuses(
      "security filters never become unrestricted table grants",
      [&] { static_cast<void>(Resolve(filteredCatalog, std::array{kRoot})); },
      "PermissionFilterUnsupported");
  CHECK_TRUE("excluded-set filters are not interpreted as negative row predicates",
             Resolve(filteredCatalog, leaf) == kNone);
  CHECK_TRUE("a filter on a different operation does not fabricate a grant",
             Resolve(filteredCatalog, std::array{kRoot}, Operation::Modify) == kNone);
  const std::array zero{Set{.identity = kRoot}};
  CHECK_TRUE("a later call cannot inherit a cached user's earlier grants",
             Resolve(Catalog(zero), std::array{kRoot}) == kNone);
}

void MissingAndCycles() {
  constexpr std::array leaf{kLeaf};
  constexpr std::array root{kRoot};
  const std::array missing{Set{.identity = kRoot, .included = leaf}};
  Refuses("missing include refuses rather than silently shrinking the policy",
          [&] { static_cast<void>(Resolve(Catalog(missing), root)); });
  const std::array missingExclude{Set{.identity = kRoot, .excluded = leaf}};
  Refuses("missing exclusion refuses rather than broadening the policy",
          [&] { static_cast<void>(Resolve(Catalog(missingExclude), root)); });
  const std::array cycle{Set{.identity = kRoot, .included = leaf},
                         Set{.identity = kLeaf, .included = root}};
  Refuses(
      "include cycles refuse explicitly",
      [&] { static_cast<void>(Resolve(Catalog(cycle), root)); },
      "PermissionPolicy",
      "cyclic permission-set composition");
  const std::array excludeCycle{Set{.identity = kRoot, .excluded = leaf},
                                Set{.identity = kLeaf, .included = root}};
  Refuses(
      "exclusion cycles refuse explicitly",
      [&] { static_cast<void>(Resolve(Catalog(excludeCycle), root)); },
      "PermissionPolicy",
      "cyclic permission-set composition");
  const std::array ownCycle{Set{.identity = kRoot, .included = root}};
  Refuses(
      "self-inclusion refuses explicitly",
      [&] { static_cast<void>(Resolve(Catalog(ownCycle), root)); },
      "PermissionPolicy",
      "cyclic permission-set composition");
  const Catalog empty({});
  Refuses("missing root refuses rather than a synthetic SUPER",
          [&] { static_cast<void>(Resolve(empty, root)); });
  const Set wrong{.identity = kLeaf};
  Refuses("catalogue cannot substitute a different app/role/scope declaration", [&] {
    static_cast<void>(agiru::ResolvePermission(
        MismatchedCatalog(wrong), root, kObject, Kind::TableData, Operation::Read));
  });
}

void Bounds() {
  constexpr std::array read{agiru::PermissionEntry{.rights = kRead, .object = kObject}};
  constexpr std::array references{kLeaf, kLeaf, kLeaf};
  constexpr std::array root{kRoot};
  const std::array definitions{Set{.identity = kRoot, .included = references},
                               Set{.identity = kLeaf, .permissions = read}};
  const Catalog catalog(definitions);
  constexpr agiru::PermissionSetLimits exact{.sets = 2, .depth = 2, .edges = 4, .entries = 1};
  CHECK_TRUE("shared graph identities fit exact set/depth/edge/entry bounds",
             Resolve(catalog, root, Operation::Read, exact) == kDirect);
  CHECK_TRUE("shared children are loaded once per call, not exponentially", catalog.Lookups() == 2);
  auto limits = exact;
  limits.sets = 1;
  Refuses("set bound refuses before loading excess declarations",
          [&] { static_cast<void>(Resolve(catalog, root, Operation::Read, limits)); });
  limits = exact;
  limits.depth = 1;
  Refuses("depth bound refuses before expanding excess paths",
          [&] { static_cast<void>(Resolve(catalog, root, Operation::Read, limits)); });
  limits = exact;
  limits.edges = 3;
  Refuses("edge bound counts repeated cached references",
          [&] { static_cast<void>(Resolve(catalog, root, Operation::Read, limits)); });
  const std::array tooManyEntries{read.front(), read.front()};
  const std::array oversized{Set{.identity = kRoot, .permissions = tooManyEntries}};
  Refuses("entry bound refuses excess permission arrays",
          [&] { static_cast<void>(Resolve(Catalog(oversized), root, Operation::Read, exact)); });
  const std::array emptyExtensions{agiru::PermissionSetExtensionDef{},
                                   agiru::PermissionSetExtensionDef{}};
  const std::array manyExtensions{Set{.identity = kRoot, .extensions = emptyExtensions}};
  limits = exact;
  limits.edges = 2;
  Refuses("empty extension fragments also consume the admission budget", [&] {
    static_cast<void>(Resolve(Catalog(manyExtensions), root, Operation::Read, limits));
  });
  for (auto member : {&agiru::PermissionSetLimits::sets,
                      &agiru::PermissionSetLimits::depth,
                      &agiru::PermissionSetLimits::edges,
                      &agiru::PermissionSetLimits::entries}) {
    limits = exact;
    limits.*member = 0;
    Refuses("zero admission bounds refuse even with no assigned sets",
            [&] { static_cast<void>(Resolve(catalog, {}, Operation::Read, limits)); });
  }
}

void InvalidPolicy() {
  constexpr std::array root{kRoot};
  constexpr std::array invalidLevel{
      agiru::PermissionEntry{.rights = {std::bit_cast<Level>(std::uint8_t{3})}, .object = kObject}};
  const std::array malformed{Set{.identity = kRoot, .permissions = invalidLevel}};
  Refuses("unknown original permission ordinals refuse",
          [&] { static_cast<void>(Resolve(Catalog(malformed), root)); });
  constexpr std::array invalidObject{agiru::PermissionEntry{.object = -1}};
  const std::array negative{Set{.identity = kRoot, .permissions = invalidObject}};
  Refuses("negative object selectors never act as a wildcard",
          [&] { static_cast<void>(Resolve(Catalog(negative), root)); });
  constexpr std::array invalidKind{
      agiru::PermissionEntry{.kind = std::bit_cast<Kind>(std::uint8_t{2})}};
  const std::array unknown{Set{.identity = kRoot, .permissions = invalidKind}};
  Refuses("malformed declared object categories refuse",
          [&] { static_cast<void>(Resolve(Catalog(unknown), root)); });
  const Catalog empty({});
  for (const auto object : {0, -1}) {
    Refuses("clients cannot request the declaration wildcard", [&] {
      static_cast<void>(
          agiru::ResolvePermission(empty, {}, object, Kind::TableData, Operation::Read));
    });
  }
  Refuses("unknown requested object kinds refuse", [&] {
    static_cast<void>(agiru::ResolvePermission(
        empty, {}, kObject, std::bit_cast<Kind>(std::uint8_t{2}), Operation::Read));
  });
  Refuses("unknown requested operations refuse before array access", [&] {
    static_cast<void>(Resolve(empty, {}, std::bit_cast<Operation>(kUnknownOperation)));
  });
  for (const auto identity :
       {Identity{.app = "", .role = "ROOT"},
        Identity{.app = kApp, .role = ""},
        Identity{.app = kApp,
                 .role = "ROOT",
                 .scope = std::bit_cast<agiru::PermissionScope>(std::uint8_t{2})}}) {
    Refuses("blank or unknown assignment identities refuse",
            [&] { static_cast<void>(Resolve(empty, std::array{identity})); });
  }
}

}

int main() {
  return gate::Run("PermissionSets", [] {
    LevelAlgebra();
    Hierarchy();
    WildcardsAndIdentity();
    ExtensionsAndFilters();
    MissingAndCycles();
    Bounds();
    InvalidPolicy();
  });
}
