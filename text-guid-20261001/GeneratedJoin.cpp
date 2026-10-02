#include "fixture/codeunit/TextGuidJoin.h"
#include "type/StringValue.h"

#include "Check.h"

#include <string>

int main() {
  return gate::Run("GeneratedTextGuidJoin", [] {
    agiru::Fixture::TextGuidJoin_Codeunit fixture;
    CHECK_TRUE("all sixteen generated AL checks execute", fixture.Exercise() == 16);
    std::string diagnostic;
    try {
      static_cast<void>(fixture.Limited());
    } catch (const agiru::StringError &error) { diagnostic = error.what(); }
    CHECK_TEXT("generated destination assignment keeps the BC diagnostic",
               diagnostic.substr(0, diagnostic.find(". Value")),
               "The length of the string is 40, but it must be less than or equal to 3 characters");
  });
}
