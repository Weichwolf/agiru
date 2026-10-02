// Generated from Host.Page.al. Do not edit.

#include "CollisionHost.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"


namespace agiru::Fixture {

constexpr std::array<ControlDef, 2> kCollisionHost_C1{{
    ControlDef{.kind = ControlKind::Part, .name = "Dispatch Work", .page = ::agiru::PageId{50102}},
    ControlDef{.kind = ControlKind::Part, .name = "Dispatch-Work", .page = ::agiru::PageId{50102}},
}};

constexpr std::array<ControlDef, 1> kCollisionHostLayout{{
    ControlDef{.kind = ControlKind::Area, .area = AreaKind::Content, .children = kCollisionHost_C1},
}};

constexpr PageDef kCollisionHostPage{
    .id = CollisionHost_Page::kId,
    .name = CollisionHost_Page::kName,
    .caption = CollisionHost_Page::kName,
    .type = PageType::Card,
    .layout = kCollisionHostLayout,
};

static_assert(Depth(kCollisionHostLayout) == 2,
              "a page's layout is a TREE and the generator keeps it -- a flattened one is one level deep (board:0553)");

} // namespace agiru::Fixture
namespace agiru::Fixture {

namespace {
namespace CollisionHost_unit {
const RegisterPage<CollisionHost_Page> kInPageCatalogue;
} // namespace CollisionHost_unit
} // namespace

} // namespace agiru::Fixture
