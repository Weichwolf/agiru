// Generated from Option.Page.al. Do not edit.

#include "NativeOption.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "platform/AllObjType.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Record.h"
#include "runtime/Report.h"
#include "type/Integer.h"
#include "type/ObjectType.h"
#include "type/Option.h"



namespace agiru::Fixture {

void NativeOption_Page::OnActionSelect() {
  Rec.ObjectType = ::agiru::Option<::agiru::platform::AllObjType>{::agiru::platform::AllObjType::Table};
}

void NativeOption_Page::OnOpenPage() {
  Rec.SetRange(Rec.ObjectType, ::agiru::Option<::agiru::platform::AllObjType>{::agiru::platform::AllObjType::Report});
}

::agiru::Integer NativeOption_Page::Exercise() {
  [[maybe_unused]] ::agiru::Integer Ordinal{};

  Rec.ObjectType = ::agiru::Option<::agiru::platform::AllObjType>{::agiru::platform::AllObjType::Report};
  if (Rec.ObjectType != ::agiru::Option<::agiru::platform::AllObjType>{::agiru::platform::AllObjType::Report}) {
    ::agiru::RaiseOrCollect("Native explicit scope");
  }
  Ordinal = Rec.ObjectType;
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("Native ordinal");
  }
  if (Format(Rec.ObjectType) != "Report") {
    ::agiru::RaiseOrCollect("Native format");
  }
  Rec.SetRange(Rec.ObjectType, ::agiru::Option<::agiru::platform::AllObjType>{::agiru::platform::AllObjType::Report});
  Ordinal = Rec.GetRangeMin(Rec.ObjectType);
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("Native filter lower bound");
  }
  return 4;
}

void NativeOption_Page::ClearAll() {
}

} // namespace agiru::Fixture
