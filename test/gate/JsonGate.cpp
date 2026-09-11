#include "runtime/Error.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/JsonArray.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/List.h"
#include "type/Text.h"

#include "Check.h"

#include <string>
#include <string_view>

namespace {

agiru::JsonObject AnObject() {
  return {};
}

agiru::JsonArray AnArray() {
  return {};
}

/// A DECLARED VARIABLE IS ALREADY AN EMPTY DOCUMENT: `JsonObject.Add` on one that nothing was
/// read into is ordinary AL, and 106 UT cases did exactly that.
void ADeclaredVariableIsAlreadyADocument() {
  agiru::JsonObject fresh;
  CHECK_TRUE("a declared object takes an Add",
             static_cast<bool>(fresh.Add("a", agiru::Integer{1})));
  agiru::JsonArray list;
  list.Add(agiru::Integer{1});
  CHECK_TRUE("and a declared array takes one too", list.Count() == 1);
  agiru::Text<0> written;
  CHECK_TRUE("which writes as an object", static_cast<bool>(fresh.WriteTo(written)));
  CHECK_TEXT("with what was added", std::string(std::string_view(written)), R"({"a":1})");
}

/// AL'S JSON IS A REAL DOCUMENT (board:0651): `Add` refuses a key that is already there and says
/// so through its Boolean, `WriteTo` renders what was added, and `Get` hands back a token that
/// reads as the type the value has.
void AnObjectHoldsWhatWasAddedToIt() {
  agiru::JsonObject object = AnObject();
  CHECK_TRUE("a key is added once",
             static_cast<bool>(object.Add("name", std::string_view("Widget"))));
  CHECK_TRUE("and a number beside it", static_cast<bool>(object.Add("count", agiru::Integer{7})));
  CHECK_TRUE("a key that is already there is refused rather than overwritten",
             !static_cast<bool>(object.Add("count", agiru::Integer{8})));
  CHECK_TRUE("Replace is how AL overwrites one",
             static_cast<bool>(object.Replace("count", agiru::Integer{8})));
  agiru::Text<0> written;
  CHECK_TRUE("WriteTo renders the document", static_cast<bool>(object.WriteTo(written)));
  CHECK_TEXT("as JSON", std::string(std::string_view(written)), R"({"count":8,"name":"Widget"})");

  agiru::JsonToken token;
  CHECK_TRUE("Get finds the key", static_cast<bool>(object.Get("count", token)));
  CHECK_TRUE("and the token reads as the number it holds", token.AsValue().AsInteger() == 8);
  CHECK_TRUE("Contains answers for a key that is there",
             static_cast<bool>(object.Contains("name")));
  CHECK_TRUE("Keys names them", object.Keys().Count() == 2);
  CHECK_TRUE("Remove takes one out", static_cast<bool>(object.Remove("name")));

  // THE NEGATIVE CONTROL: a key that was never added is not found, is not removed, and the Get
  // leaves its token alone rather than handing back a null one that reads as zero.
  CHECK_TRUE("a key that is not there is not found", !static_cast<bool>(object.Get("nope", token)));
  CHECK_TRUE("nor removed", !static_cast<bool>(object.Remove("nope")));
  CHECK_TRUE("nor contained", !static_cast<bool>(object.Contains("nope")));
}

/// A DECIMAL SURVIVES THE ROUND TRIP AS A DECIMAL, which is what an amount in an API payload
/// depends on: `2.50` read back is `2.5` and not `2.4999999999999996`.
void ADecimalIsReadBackAsADecimal() {
  agiru::JsonObject object = AnObject();
  CHECK_TRUE("a decimal is read", static_cast<bool>(object.ReadFrom(R"({"amount":1234.56})")));
  agiru::JsonToken token;
  CHECK_TRUE("and found", static_cast<bool>(object.Get("amount", token)));
  CHECK_TRUE("as the same value",
             token.AsValue().AsDecimal() == agiru::Decimal::FromInvariantString("1234.56"));
  agiru::Text<0> written;
  CHECK_TRUE("and it writes back unchanged", static_cast<bool>(object.WriteTo(written)));
  CHECK_TEXT("byte for byte", std::string(std::string_view(written)), R"({"amount":1234.56})");

  // THE NEGATIVE CONTROL: text that is not JSON is refused rather than half-read.
  agiru::JsonObject broken = AnObject();
  CHECK_TRUE("text that is not JSON is refused", !static_cast<bool>(broken.ReadFrom("{not json")));
  CHECK_TRUE("and an array is not an object", !static_cast<bool>(broken.ReadFrom("[1,2]")));
}

/// AN ARRAY COUNTS FROM ZERO, which `jsonarray-get-method.md` states and the BaseApp relies on.
void AnArrayIsIndexedFromZero() {
  agiru::JsonArray array = AnArray();
  array.Add(std::string_view("first"));
  array.Add(std::string_view("second"));
  CHECK_TRUE("both are in", array.Count() == 2);
  agiru::JsonToken token;
  CHECK_TRUE("index 0 is the first", static_cast<bool>(array.Get(0, token)));
  CHECK_TEXT("by value", token.AsValue().AsText(), "first");
  CHECK_TRUE("index 1 is the second", static_cast<bool>(array.Get(1, token)));
  CHECK_TRUE("index 2 is past the end", !static_cast<bool>(array.Get(2, token)));
  CHECK_TRUE("and so is a negative one", !static_cast<bool>(array.Get(-1, token)));
  CHECK_TRUE("RemoveAt takes one out", static_cast<bool>(array.RemoveAt(0)));
  CHECK_TRUE("leaving one", array.Count() == 1);
}

/// A JSON VALUE IS A REFERENCE, as `jsonobject-data-type.md` says: a token from `Get` looks INTO
/// the object, so a change through it shows in what the object writes.
void ATokenLooksIntoTheDocument() {
  agiru::JsonObject object = AnObject();
  CHECK_TRUE("the object is read", static_cast<bool>(object.ReadFrom(R"({"inner":{"n":1}})")));
  agiru::JsonToken inner;
  CHECK_TRUE("and the inner object found", static_cast<bool>(object.Get("inner", inner)));
  agiru::JsonObject held = inner.AsObject();
  CHECK_TRUE("a change through the token", static_cast<bool>(held.Replace("n", agiru::Integer{9})));
  agiru::Text<0> written;
  CHECK_TRUE("shows in the outer document", static_cast<bool>(object.WriteTo(written)));
  CHECK_TEXT("which is what a reference type means",
             std::string(std::string_view(written)),
             R"({"inner":{"n":9}})");
}

/// SELECTTOKEN WALKS A PATH, the JSONPath subset the BaseApp uses: `$.a.b`, `a.b`, and `a[0]`.
void SelectTokenWalksThePath() {
  agiru::JsonObject object = AnObject();
  CHECK_TRUE("a nested document is read",
             static_cast<bool>(object.ReadFrom(R"({"a":{"b":[10,20]},"c":"x"})")));
  agiru::JsonToken token;
  CHECK_TRUE("a path with the root", static_cast<bool>(object.SelectToken("$.c", token)));
  CHECK_TEXT("finds the value", token.AsValue().AsText(), "x");
  CHECK_TRUE("a path without it", static_cast<bool>(object.SelectToken("c", token)));
  CHECK_TRUE("and an index into an array",
             static_cast<bool>(object.SelectToken("$.a.b[1]", token)) &&
                 token.AsValue().AsInteger() == 20);
  CHECK_TRUE("GetText reads a key by name", object.GetText("c") == "x");

  // THE NEGATIVE CONTROL: a path that leads nowhere answers false rather than an empty token,
  // and a key that is not there refuses unless the caller asked for a default.
  CHECK_TRUE("a path that is not there", !static_cast<bool>(object.SelectToken("$.a.b[9]", token)));
  CHECK_TRUE("nor a name that is not", !static_cast<bool>(object.SelectToken("$.nope", token)));
  std::string said;
  try {
    static_cast<void>(object.GetText("nope"));
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("GetText without a default refuses", !said.empty());
  CHECK_TRUE("and with one answers blank", object.GetText("nope", true).Length() == 0);
}

} // namespace

int main() {
  return gate::Run("Json", [] {
    ADeclaredVariableIsAlreadyADocument();
    AnObjectHoldsWhatWasAddedToIt();
    ADecimalIsReadBackAsADecimal();
    AnArrayIsIndexedFromZero();
    ATokenLooksIntoTheDocument();
    SelectTokenWalksThePath();
  });
}
