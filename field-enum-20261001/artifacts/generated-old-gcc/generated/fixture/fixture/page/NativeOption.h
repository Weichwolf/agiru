// Generated from Option.Page.al. Do not edit.
#pragma once

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "platform/AllObjWithCaption.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "type/Integer.h"
#include "type/ObjectType.h"
#include "type/Option.h"


#include "platform/AllObjWithCaption.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
class NativeOption_Controls {
public:
  Field_Kind ObjectType{"ObjectType"};

  Action_Kind Select{"Select"};
  Field_Kind ALNamespace{"al namespace"};
  Field_Kind AppID{"app id"};
  Field_Kind AppPackageID{"app package id"};
  Field_Kind AppRuntimePackageID{"app runtime package id"};
  Field_Kind ObjectCaption{"object caption"};
  Field_Kind ObjectID{"object id"};
  Field_Kind ObjectName{"object name"};
  Field_Kind ObjectSubtype{"object subtype"};
  Field_Kind SystemCreatedAt{"systemcreatedat"};
  Field_Kind SystemCreatedBy{"systemcreatedby"};
  Field_Kind SystemId{"systemid"};
  Field_Kind SystemModifiedAt{"systemmodifiedat"};
  Field_Kind SystemModifiedBy{"systemmodifiedby"};

  template <typename Core> void BindControls(Core &core) {
    ObjectType.Bind(core);
    Select.Bind(core);
    ALNamespace.Bind(core);
    AppID.Bind(core);
    AppPackageID.Bind(core);
    AppRuntimePackageID.Bind(core);
    ObjectCaption.Bind(core);
    ObjectID.Bind(core);
    ObjectName.Bind(core);
    ObjectSubtype.Bind(core);
    SystemCreatedAt.Bind(core);
    SystemCreatedBy.Bind(core);
    SystemId.Bind(core);
    SystemModifiedAt.Bind(core);
    SystemModifiedBy.Bind(core);
  }
};

class NativeOption_Page;

class NativeOption_Page : public Page<NativeOption_Page> {
public:
  static constexpr PageId kId{50221};
  static constexpr std::string_view kName{"Native Option"};

  ::agiru::platform::AllObjWithCaption Rec;


  void OnActionSelect();

  void OnOpenPage();
  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const PageDef kNativeOptionPage;

} // namespace agiru::Fixture

template <> struct agiru::PageTraits<agiru::Fixture::NativeOption_Page> {
  static constexpr PageId kId{50221};
  static constexpr std::string_view kName{"Native Option"};
  static constexpr const PageDef &kPage = agiru::Fixture::kNativeOptionPage;
  template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
  using Controls = agiru::Fixture::NativeOption_Controls<Field_Kind, Action_Kind, Part_Kind>;
  static constexpr std::array<::agiru::ControlTrigger<agiru::Fixture::NativeOption_Page>, 1> kControlTriggers{{
      {.control = "Select", .validate = nullptr, .action = &agiru::Fixture::NativeOption_Page::OnActionSelect, .drillDown = nullptr, .assistEdit = nullptr, .lookup = nullptr, .visible = nullptr, .enabled = nullptr, .editable = nullptr}
  }};
};
