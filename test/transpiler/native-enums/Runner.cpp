#include "type/Enum.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/codeunit/NativeConsumerUT.h"
#include "fixture/table/NativeRecord.h"
#include "system/fixture/enum/NativeSparse.h"
#include "system/fixture/interface/NativeBase.h"
#include "system/fixture/interface/NativeContract.h"

namespace {

using Sparse = agiru::System::Fixture::NativeSparse_Enum;
constexpr agiru::Integer kChosen = 10;
constexpr agiru::Integer kExtended = 70;

}

int main() {
  return gate::Run("Generated Native Enums", [] {
    agiru::Fixture::NativeConsumerUT_Codeunit unit;
    agiru::Enum<Sparse> value = Sparse::Chosen;
    const auto returned = unit.Echo(value);
    CHECK_TRUE("native return and local retain the original sparse ordinal",
               returned.AsInteger() == kChosen);
    CHECK_TRUE("native var parameter receives the source extension",
               value.AsInteger() == kExtended);
    CHECK_TRUE("numeric enum identity names the same type", unit.Numeric(returned) == kChosen);
    CHECK_TEXT("native source caption survives", returned.Caption(), "Chosen value");
    CHECK_TEXT("extension caption survives", value.Caption(), "Extended value");
    agiru::Fixture::NativeRecord_Table record;
    record.Choice = returned;
    CHECK_TRUE("a generated field uses the native type", record.Choice.AsInteger() == kChosen);
    CHECK_TRUE("all base and extension values remain counted",
               agiru::EnumTraits<Sparse>::kValues.size() == 3);
    CHECK_TRUE("source enum ID survives", agiru::EnumTraits<Sparse>::kObjectID == 50240);
    CHECK_TRUE("source extensibility survives", agiru::EnumTraits<Sparse>::kExtensible);
    CHECK_TEXT("source enum scope survives", agiru::EnumTraits<Sparse>::kScope, "Cloud");
    agiru::System::Fixture::NativeBase_Interface &base = unit;
    CHECK_TRUE("native interface inheritance retains numeric enum parameters",
               base.Numeric(returned) == kChosen);
    agiru::System::Fixture::NativeContract_Interface &face = unit;
    value = Sparse::Chosen;
    CHECK_TRUE("native interface dispatch retains a typed return",
               face.Echo(value).AsInteger() == kChosen);
    CHECK_TRUE("native interface dispatch retains var effects", value.AsInteger() == kExtended);
    value = Sparse::Chosen;
    CHECK_TRUE("AL interface assignment dispatches through the native declaration",
               unit.Dispatch(value).AsInteger() == kChosen);
    CHECK_TRUE("AL interface assignment preserves var effects", value.AsInteger() == kExtended);
    unit.Kept();
  });
}
