// Generated from Child.Page.al. Do not edit.

#include "Child.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"


namespace agiru::Fixture {

constexpr PageDef kChildPage{
    .id = Child_Page::kId,
    .name = Child_Page::kName,
    .caption = Child_Page::kName,
    .type = PageType::Card,
};


} // namespace agiru::Fixture
namespace agiru::Fixture {

namespace {
namespace Child_unit {
const RegisterPage<Child_Page> kInPageCatalogue;
} // namespace Child_unit
} // namespace

} // namespace agiru::Fixture
