// Generated from Option.Page.al. Do not edit.

#include "NativeOption.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Record.h"
#include "runtime/Report.h"
#include "type/Integer.h"
#include "type/ObjectType.h"
#include "type/Option.h"


#include "options/Types.h"

namespace agiru::Fixture {

void NativeOption_Page::OnActionSelect() {
  Rec.ObjectType = ::agiru::Option<::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701>{::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701::Table};
}

void NativeOption_Page::OnOpenPage() {
  Rec.SetRange(Rec.ObjectType, ::agiru::Option<::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701>{::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701::Report});
}

::agiru::Integer NativeOption_Page::Exercise() {
  [[maybe_unused]] ::agiru::Integer Ordinal{};

  Rec.ObjectType = ::agiru::Option<::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701>{::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701::Report};
  if (Rec.ObjectType != ::agiru::Option<::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701>{::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701::Report}) {
    ::agiru::RaiseOrCollect("Native explicit scope");
  }
  Ordinal = Rec.ObjectType;
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("Native ordinal");
  }
  if (Format(Rec.ObjectType) != "Report") {
    ::agiru::RaiseOrCollect("Native format");
  }
  Rec.SetRange(Rec.ObjectType, ::agiru::Option<::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701>{::agiru::options::OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701::Report});
  Ordinal = Rec.GetRangeMin(Rec.ObjectType);
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("Native filter lower bound");
  }
  return 4;
}

void NativeOption_Page::ClearAll() {
}

} // namespace agiru::Fixture
