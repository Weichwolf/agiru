#include "runtime/ErrorValue.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/codeunit/IDCaller.h"
#include "fixture/page/IDPage.h"
#include "fixture/report/IDReport.h"
#include "fixture/table/IDOwner.h"

#include <string_view>

namespace {

constexpr agiru::Integer kDeclaredID = 2000000821;
constexpr agiru::Integer kPageID = 50303;

template <typename Owner> void DeclaredConstants() {
  Owner owner;
  CHECK_TRUE("native ID follows the selected original declaration",
             owner.NativeID() == kDeclaredID);
  CHECK_TRUE("qualified native ID retains its declared namespace",
             owner.QualifiedID() == kDeclaredID);
}

template <typename Call> void Refuses(Call call, std::string_view identity) {
  bool refused = false;
  try {
    static_cast<void>(call());
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).contains(identity);
  }
  CHECK_TRUE("unbound behaviour remains an explicit named refusal", refused);
}

}

int main() {
  return gate::Run("Generated Native Table IDs", [] {
    DeclaredConstants<agiru::Fixture::IDCaller_Codeunit>();
    DeclaredConstants<agiru::Fixture::IDOwner_Table>();
    DeclaredConstants<agiru::Fixture::IDPage_Page>();
    DeclaredConstants<agiru::Fixture::IDReport_Report>();
    agiru::Fixture::IDCaller_Codeunit caller;
    CHECK_TRUE("other object kinds keep their own identity", caller.OtherKind() == kPageID);
    CHECK_TRUE("a local Option shadows the Database keyword", caller.Shadowed() == 1);
    Refuses([&caller] { return caller.WrongNamespace(); }, "Other.Declared Only");
    Refuses([&caller] { return caller.MissingID(); }, "Missing");
    Refuses([&caller] { return caller.NativeRead(); }, "DeclaredOnly.FindFirst");
  });
}
