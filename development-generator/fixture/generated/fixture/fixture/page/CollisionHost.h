// Generated from Host.Page.al. Do not edit.
#pragma once

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "type/Integer.h"


#include "fixture/page/Child.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
class CollisionHost_Controls {
public:

  Part_Kind<::agiru::Fixture::Child_Page> DispatchWork_3;
  Part_Kind<::agiru::Fixture::Child_Page> DispatchWork_4;

  template <typename Core> void BindControls(Core &core) {
    DispatchWork_3.BindPart(core, "Dispatch Work");
    DispatchWork_4.BindPart(core, "Dispatch-Work");
  }
};

class CollisionHost_Page;

class CollisionHost_Page : public Page<CollisionHost_Page> {
public:
  static constexpr PageId kId{50101};
  static constexpr std::string_view kName{"Collision Host"};

  ::agiru::PartRef<::agiru::Fixture::Child_Page> DispatchWork_3;
  ::agiru::PartRef<::agiru::Fixture::Child_Page> DispatchWork_4;

  void *PartInstance(std::string_view name) {
    if (name == "Dispatch Work") { return &DispatchWork_3.Held(); }
    if (name == "Dispatch-Work") { return &DispatchWork_4.Held(); }
    return nullptr;
  }


  ::agiru::Integer DispatchWork();
  ::agiru::Integer DispatchWork_2();
  ::agiru::Integer Invoke();

  void ClearAll();
};

extern const PageDef kCollisionHostPage;

} // namespace agiru::Fixture

template <> struct agiru::PageTraits<agiru::Fixture::CollisionHost_Page> {
  static constexpr PageId kId{50101};
  static constexpr std::string_view kName{"Collision Host"};
  static constexpr const PageDef &kPage = agiru::Fixture::kCollisionHostPage;
  template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
  using Controls = agiru::Fixture::CollisionHost_Controls<Field_Kind, Action_Kind, Part_Kind>;
  static constexpr std::array<::agiru::ControlTrigger<agiru::Fixture::CollisionHost_Page>, 0> kControlTriggers{{}};
};
