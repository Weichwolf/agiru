// Generated from NativePage.Page.al. Do not edit.
#pragma once

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "platform/ODataEdmType.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "type/Code.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru {

template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
class NativePage_Controls {
public:

  template <typename Core> void BindControls(Core &core) {
    static_cast<void>(core);
  }
};

class NativePage_Page;

class NativePage_Page : public Page<NativePage_Page> {
public:
  static constexpr PageId kId{50272};
  static constexpr std::string_view kName{"NativePage"};


  ::agiru::Code<50> Read(::agiru::platform::ODataEdmType &Row);

  void ClearAll();
};

extern const PageDef kNativePagePage;

} // namespace agiru

template <> struct agiru::PageTraits<agiru::NativePage_Page> {
  static constexpr PageId kId{50272};
  static constexpr std::string_view kName{"NativePage"};
  static constexpr const PageDef &kPage = agiru::kNativePagePage;
  template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
  using Controls = agiru::NativePage_Controls<Field_Kind, Action_Kind, Part_Kind>;
  static constexpr std::array<::agiru::ControlTrigger<agiru::NativePage_Page>, 0> kControlTriggers{{}};
};
