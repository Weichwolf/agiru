#include "fixture/page/NativeOption.h"
#include "fixture/table/OrdinaryOption.h"
#include "platform/AllObjType.h"
#include "type/Option.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedFieldOptions", [] {
    agiru::Fixture::OrdinaryOption_Table ordinary;
    agiru::Fixture::NativeOption_Page native;
    CHECK_TRUE("four ordinary AL checks execute", ordinary.Exercise() == 4);
    CHECK_TRUE("four native page AL checks execute", native.Exercise() == 4);
    native.Rec.SetRange(native.Rec.ObjectType);
    native.OnOpenPage();
    CHECK_TRUE("actual page trigger filters the native option",
               native.Rec.GetRangeMin(native.Rec.ObjectType) == agiru::platform::AllObjType::Report);
    native.OnActionSelect();
    CHECK_TRUE("actual page action assigns the native option",
               native.Rec.ObjectType == agiru::platform::AllObjType::Table);
  });
}
