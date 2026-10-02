// Generated from Child.Page.al. Do not edit.
#pragma once

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "type/Integer.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
class Child_Controls {
public:

  template <typename Core> void BindControls(Core &core) {
    static_cast<void>(core);
  }
};

class Child_Page;

class Child_Page : public Page<Child_Page> {
public:
  static constexpr PageId kId{50102};
  static constexpr std::string_view kName{"Child"};

  ::agiru::Integer Calls;


  void Touch();
  ::agiru::Integer GetCalls();

  void ClearAll();
};

extern const PageDef kChildPage;

} // namespace agiru::Fixture

template <> struct agiru::PageTraits<agiru::Fixture::Child_Page> {
  static constexpr PageId kId{50102};
  static constexpr std::string_view kName{"Child"};
  static constexpr const PageDef &kPage = agiru::Fixture::kChildPage;
  template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
  using Controls = agiru::Fixture::Child_Controls<Field_Kind, Action_Kind, Part_Kind>;
  static constexpr std::array<::agiru::ControlTrigger<agiru::Fixture::Child_Page>, 0> kControlTriggers{{}};
};
