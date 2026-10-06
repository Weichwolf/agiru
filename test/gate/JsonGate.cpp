#include "runtime/ErrorValue.h"
#include "type/BigInteger.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/JsonArray.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/List.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"

#include <limits>
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
  const agiru::JsonObject fresh;
  CHECK_TRUE("a declared object takes an Add",
             static_cast<bool>(fresh.Add("a", agiru::Integer{1})));
  const agiru::JsonArray list;
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
  const agiru::JsonObject object = AnObject();
  CHECK_TRUE("a key is added once",
             static_cast<bool>(object.Add("name", std::string_view("Widget"))));
  CHECK_TRUE("and a number beside it", static_cast<bool>(object.Add("count", agiru::Integer{7})));
  CHECK_TRUE("a key that is already there is refused rather than overwritten",
             !static_cast<bool>(object.Add("count", agiru::Integer{8})));
  CHECK_TRUE("Replace is how AL overwrites one",
             static_cast<bool>(object.Replace("count", agiru::Integer{8})));
  agiru::Text<0> written;
  CHECK_TRUE("WriteTo renders the document", static_cast<bool>(object.WriteTo(written)));
  CHECK_TEXT(
      "as JSON, in the order the members were added -- which is BC's order and Newtonsoft's, "
      "not the alphabet's",
      std::string(std::string_view(written)),
      R"({"name":"Widget","count":8})");

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
  const agiru::JsonArray array = AnArray();
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
  const agiru::JsonObject held = inner.AsObject();
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
  CHECK_TRUE("an overflowing index cannot wrap to a real element",
             !object.SelectToken("$.a.b[18446744073709551617]", token));
  CHECK_TRUE("an empty index cannot become zero", !object.SelectToken("$.a.b[]", token));
  std::string said;
  try {
    static_cast<void>(object.GetText("nope"));
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("GetText without a default refuses", !said.empty());
  CHECK_TRUE("and with one answers blank", object.GetText("nope", true).Length() == 0);
}

void ExactNumbersRemainNumbers() {
  agiru::JsonObject object;
  const std::string source =
      R"({"large":999999999999999.99,"scale":0.1234567890123456789012345678,"exponent":-1.25e+2,"max":9223372036854775807,"min":-9223372036854775808})";
  CHECK_TRUE("exact numbers parse", object.ReadFrom(source));
  CHECK_TRUE("large decimal retains every digit",
             object.GetDecimal("large") ==
                 agiru::Decimal::FromInvariantString("999999999999999.99"));
  CHECK_TRUE("scale 28 retains every digit",
             object.GetDecimal("scale") ==
                 agiru::Decimal::FromInvariantString("0.1234567890123456789012345678"));
  CHECK_TRUE("exponent is exact", object.GetDecimal("exponent") == agiru::Decimal{-125});
  agiru::JsonToken token;
  CHECK_TRUE("max Int64 found", object.Get("max", token));
  CHECK_TRUE("max Int64 is exact",
             token.AsValue().AsBigInteger() == std::numeric_limits<agiru::BigInteger>::max());
  CHECK_TRUE("min Int64 found", object.Get("min", token));
  CHECK_TRUE("min Int64 is exact",
             token.AsValue().AsBigInteger() == std::numeric_limits<agiru::BigInteger>::min());
  agiru::Text<0> written;
  CHECK_TRUE("numbers serialize", object.WriteTo(written));
  CHECK_TEXT("raw number lexemes survive without quoting", written, source);
  const agiru::JsonObject added;
  CHECK_TRUE("Decimal Add succeeds",
             added.Add("amount", agiru::Decimal::FromInvariantString("999999999999999.99")));
  CHECK_TRUE("added Decimal serializes", added.WriteTo(written));
  CHECK_TEXT("Decimal Add emits an exact number", written, R"({"amount":999999999999999.99})");
}

template <typename Operation> bool Refuses(Operation operation) {
  try {
    operation();
  } catch (const agiru::Error &) { return true; }
  return false;
}

void InexactAndOverflowConversionsRefuse() {
  agiru::JsonObject object;
  CHECK_TRUE(
      "conversion controls parse",
      object.ReadFrom(
          R"({"fraction":1.25,"narrow":2147483648,"wide":9223372036854775808,"scale":0.12345678901234567890123456789,"range":1e100,"integer":12e2,"bad":"1..2","text":"7","zero":-0e200})"));
  CHECK_TRUE("Integer does not truncate a fraction",
             Refuses([&] { static_cast<void>(object.GetInteger("fraction")); }));
  CHECK_TRUE("Integer does not wrap overflow",
             Refuses([&] { static_cast<void>(object.GetInteger("narrow")); }));
  agiru::JsonToken token;
  CHECK_TRUE("Int64 overflow found", object.Get("wide", token));
  CHECK_TRUE("Int64 overflow refuses",
             Refuses([&] { static_cast<void>(token.AsValue().AsBigInteger()); }));
  CHECK_TRUE("Decimal does not round excess precision",
             Refuses([&] { static_cast<void>(object.GetDecimal("scale")); }));
  CHECK_TRUE("Decimal exponent overflow refuses",
             Refuses([&] { static_cast<void>(object.GetDecimal("range")); }));
  CHECK_TRUE("exact integral exponent converts", object.GetInteger("integer") == 1200);
  CHECK_TRUE("a malformed numeric string refuses",
             Refuses([&] { static_cast<void>(object.GetDecimal("bad")); }));
  CHECK_TRUE("Integer requires a number",
             Refuses([&] { static_cast<void>(object.GetInteger("text")); }));
  CHECK_TRUE("Boolean does not silently accept a number",
             Refuses([&] { static_cast<void>(object.GetBoolean("integer")); }));
  CHECK_TRUE("Boolean does not silently accept unrelated text",
             Refuses([&] { static_cast<void>(object.GetBoolean("text")); }));
  CHECK_TRUE("signed exponent zero remains exact", object.GetDecimal("zero") == agiru::Decimal{});
  CHECK_TRUE("NaN is invalid JSON", !object.ReadFrom(R"({"n":NaN})"));
  CHECK_TRUE("Infinity is invalid JSON", !object.ReadFrom(R"({"n":Infinity})"));
  CHECK_TRUE("trailing data is invalid JSON", !object.ReadFrom("{} trailing"));
}

void RetainedNodesSurviveGrowthAndRemoval() {
  agiru::JsonToken held;
  {
    agiru::JsonObject object;
    CHECK_TRUE("alias source parses", object.ReadFrom(R"({"inner":{"n":1}})"));
    CHECK_TRUE("child retained", object.Get("inner", held));
    constexpr agiru::Integer siblingCount = 128;
    for (agiru::Integer index = 0; index < siblingCount; ++index) {
      CHECK_TRUE("sibling appended", object.Add(std::to_string(index), index));
    }
    CHECK_TRUE("alias survives container growth", held.AsObject().GetInteger("n") == 1);
    CHECK_TRUE("alias write remains shared", held.AsObject().Replace("n", agiru::Integer{7}));
    CHECK_TRUE("parent observes alias write", object.GetObject("inner").GetInteger("n") == 7);
    CHECK_TRUE("child removed", object.Remove("inner"));
    CHECK_TRUE("removed child remains valid", held.AsObject().GetInteger("n") == 7);
    CHECK_TRUE("removed child is absent", !object.Contains("inner"));
  }
  CHECK_TRUE("retained child outlives parent variable", held.AsObject().GetInteger("n") == 7);
  agiru::JsonArray array;
  CHECK_TRUE("array source parses", array.ReadFrom("[10,20,30]"));
  CHECK_TRUE("array child retained", array.Get(1, held));
  CHECK_TRUE("earlier child removed", array.RemoveAt(0));
  CHECK_TRUE("alias tracks identity rather than former index", held.AsValue().AsInteger() == 20);
  CHECK_TRUE("retained child removed", array.RemoveAt(0));
  CHECK_TRUE("detached array child remains valid", held.AsValue().AsInteger() == 20);
}

void SetValueDisconnectsAndUsesItsDeclaredRepresentation() {
  agiru::JsonObject object;
  CHECK_TRUE("setter source parses", object.ReadFrom(R"({"n":1})"));
  agiru::JsonToken token;
  CHECK_TRUE("setter child found", object.Get("n", token));
  agiru::JsonValue value = token.AsValue();
  constexpr agiru::Integer replacement = 9;
  value.SetValue(replacement);
  CHECK_TRUE("SetValue disconnects from its containing tree", object.GetInteger("n") == 1);
  CHECK_TRUE("other aliases retain the previous node", token.AsValue().AsInteger() == 1);
  CHECK_TRUE("setter variable contains the new value", value.AsInteger() == replacement);
  value.SetValue(agiru::Decimal::FromInvariantString("999999999999999.99"));
  const agiru::JsonObject wrapped;
  CHECK_TRUE("explicit JsonValue can be added", wrapped.Add("decimal", value));
  value.SetValue(std::numeric_limits<agiru::BigInteger>::max());
  CHECK_TRUE("explicit BigInteger JsonValue can be added", wrapped.Add("integer", value));
  agiru::Text<0> written;
  CHECK_TRUE("setter representations serialize", wrapped.WriteTo(written));
  CHECK_TEXT("SetValue Decimal/BigInteger stores strings, unlike Add Decimal",
             written,
             R"({"decimal":"999999999999999.99","integer":"9223372036854775807"})");
}

void ConstHandlesPreserveSharedMutationAndIndependentRebinding() {
  agiru::JsonObject object;
  const agiru::JsonObject alias = object;
  const agiru::Decimal amount =
      agiru::Decimal::FromInvariantString("0.1234567890123456789012345678");
  const auto maximum = std::numeric_limits<agiru::BigInteger>::max();
  CHECK_TRUE("a const object handle can add to its referenced node", alias.Add("amount", amount));
  CHECK_TRUE("an object alias retains exact Decimal digits", object.GetDecimal("amount") == amount);
  CHECK_TRUE("a const handle can replace the referenced value", alias.Replace("amount", maximum));
  agiru::JsonToken token;
  CHECK_TRUE("the replaced exact Int64 can be retrieved", object.Get("amount", token));
  const agiru::JsonToken retained = token;
  const agiru::JsonValue scalar = retained.AsValue();
  CHECK_TRUE("a const scalar handle retains the exact Int64", scalar.AsBigInteger() == maximum);
  const agiru::JsonArray array;
  array.Add(amount);
  agiru::JsonArray arrayAlias = array;
  const agiru::JsonArray &constArrayAlias = arrayAlias;
  CHECK_TRUE("a const array alias can insert a referenced value",
             constArrayAlias.Insert(0, retained));
  CHECK_TRUE("the original array observes exact shared insertion",
             array.Get(0, token) && token.AsValue().AsBigInteger() == maximum);
  CHECK_TRUE("the original array retains the exact shifted Decimal",
             array.Get(1, token) && token.AsValue().AsDecimal() == amount);
  CHECK_TRUE("a const array handle can replace an element", constArrayAlias.Set(1, maximum));
  CHECK_TRUE("the original array observes exact shared replacement",
             array.Get(1, token) && token.AsValue().AsBigInteger() == maximum);
  CHECK_TRUE("a const array handle can remove an element", constArrayAlias.RemoveAt(0));
  CHECK_TRUE("the original array observes shared removal", array.Count() == 1);
  CHECK_TRUE("ReadFrom rebinds only the array alias", arrayAlias.ReadFrom("[]"));
  CHECK_TRUE("the original array survives its alias being rebound", array.Count() == 1);
  CHECK_TRUE("ReadFrom rebinds the original object variable", object.ReadFrom("{}"));
  CHECK_TRUE("the rebound original is empty", object.Keys().Count() == 0);
  CHECK_TRUE("its alias retains the previous object node",
             alias.Get("amount", token) && token.AsValue().AsBigInteger() == maximum);
  CHECK_TRUE("a const object alias can remove a property", alias.Remove("amount"));
  CHECK_TRUE("a retained child survives alias removal and owner rebinding",
             scalar.AsBigInteger() == maximum);
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
    ExactNumbersRemainNumbers();
    InexactAndOverflowConversionsRefuse();
    RetainedNodesSurviveGrowthAndRemoval();
    SetValueDisconnectsAndUsesItsDeclaredRepresentation();
    ConstHandlesPreserveSharedMutationAndIndependentRebinding();
  });
}
