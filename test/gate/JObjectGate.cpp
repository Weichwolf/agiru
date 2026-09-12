#include "dotnet/Generic.h"
#include "dotnet/JObject.h"
#include "runtime/Error.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <string>

using agiru::Variant;
using agiru::dotnet::GenericIEnumerable1;
using agiru::dotnet::GenericIEnumerator1;
using agiru::dotnet::JArray;
using agiru::dotnet::JObject;
using agiru::dotnet::JProperty;
using agiru::dotnet::JToken;
using agiru::dotnet::JValue;

namespace {

std::string T(const agiru::Text<0> &text) {
  return std::string(std::string_view(text));
}

/// NEWTONSOFT'S `JObject` FAMILY IS REBUILT ON THE JSON TREE (board:0714): a token is a REFERENCE
/// into a tree, so a property read out of an object and written through changes the object; a
/// `JObject` variable's constructor is spelled `JObject.JObject()` the way AL spells it, `Parse`
/// reads text, `Add` refuses a second member of one name the way Newtonsoft does, and
/// `ToString()` is the INDENTED form Newtonsoft renders with `\r\n` line breaks.
void AnObjectIsARefereceIntoATreeAndRendersIndented() {
  JObject object;
  object = object.JObject();
  CHECK_TRUE("a new object is not null", !agiru::IsNull(object));
  object.Add("unitCode", Variant(std::string("PCS")));
  object.Add("qty", Variant(agiru::Integer{3}));
  CHECK_TEXT("ToString is indented with CRLF",
             T(object.ToString()),
             "{\r\n  \"unitCode\": \"PCS\",\r\n  \"qty\": 3\r\n}");
  bool refused = false;
  try {
    object.Add("qty", Variant(agiru::Integer{4}));
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("a second member of one name is refused", refused);

  JProperty property = object.Property("unitCode");
  CHECK_TEXT("a property carries its name", T(property.Name()), "unitCode");
  CHECK_TEXT("and its value renders as the bare text", T(property.Value().ToString()), "PCS");
  JValue replacement;
  replacement = replacement.JValue(Variant(std::string("BOX")));
  property.Value(replacement);
  CHECK_TEXT("writing through the property changes the object",
             T(object.SelectToken("unitCode").ToString()),
             "BOX");
  JToken missing = object.SelectToken("nothing.here");
  CHECK_TRUE("a path that names nothing is null", agiru::IsNull(missing));

  JObject parsed;
  parsed = parsed.Parse("{\"a\": {\"b\": [10, 20]}, \"flag\": true}");
  CHECK_TEXT("a nested path", T(parsed.SelectToken("a.b[1]").ToString()), "20");
  CHECK_TEXT("a boolean renders the way .NET renders one",
             T(parsed.SelectToken("flag").ToString()),
             "True");
  JToken token;
  CHECK_TRUE("TryGetValue finds a member", parsed.TryGetValue("a", token));
  CHECK_TRUE("and not a missing one", !parsed.TryGetValue("z", token));
  CHECK_TRUE("Count is the member count", parsed.Count() == 2);
}

/// A VARIANT CARRIES A TOKEN BY REFERENCE, which is how `JSON Management` hands a property's
/// value to `Format`: `Value := JProperty.Value; Text := Format(Value)` reads the bare text of a
/// string leaf and the indented JSON of an object.
void AVariantCarriesATokenAndFormatRendersIt() {
  JObject object;
  object = object.Parse("{\"name\": \"Bicycle\", \"price\": 2800.5}");
  const Variant held = object.Property("name").Value();
  CHECK_TEXT("Format of a held string leaf", T(agiru::Format(held)), "Bicycle");
  JObject again;
  again = held;
  CHECK_TRUE("a Variant hands its token back", !agiru::IsNull(again));
  CHECK_TEXT("a decimal leaf keeps its digits",
             T(agiru::Format(Variant(object.Property("price").Value()))),
             "2800.5");
  const Variant property(object.Property("name"));
  CHECK_TEXT("a property renders as name and value",
             T(agiru::Format(property)),
             "\"name\": \"Bicycle\"");
}

/// AN ARRAY ENUMERATES AND SELECTS WITH A FILTER: `JSON Management` walks
/// `$[?(@.<name> == '<value>')]` to find the object whose member matches, and a `JObject` variable
/// takes what the enumerator's `Current` hands it.
void AnArrayEnumeratesAndFiltersItsObjects() {
  JArray array;
  array = array.Parse("[{\"id\": \"A\", \"n\": 1}, {\"id\": \"B\", \"n\": 2}]");
  CHECK_TRUE("Count is the item count", array.Count() == 2);
  JObject second;
  second = array.Item(Variant(agiru::Integer{1}));
  CHECK_TEXT("Item by index, seen as an object", T(second.SelectToken("id").ToString()), "B");

  GenericIEnumerable1 matches = array.SelectTokens("$[?(@.id == 'B')]", false);
  GenericIEnumerator1 walker = matches.GetEnumerator();
  CHECK_TRUE("the filter finds the one object", walker.MoveNext());
  JObject found;
  found = walker.Current();
  CHECK_TEXT("and it is the right one", T(found.SelectToken("n").ToString()), "2");
  CHECK_TRUE("and no other", !walker.MoveNext());

  int walked = 0;
  for (const JObject &item : array) {
    walked += static_cast<int>(item.Count());
  }
  CHECK_TRUE("foreach walks the items as objects", walked == 4);

  JArray more;
  more = more.Parse("[{\"id\": \"C\"}]");
  array.Merge(more);
  CHECK_TRUE("Merge appends the other's items", array.Count() == 3);
  JObject copy;
  copy = array.DeepClone();
  more.Add(Variant(agiru::Integer{7}));
  CHECK_TRUE("a deep clone is its own tree", copy.Count() == 3);
}

/// THE PROPERTIES OF AN OBJECT ARE ENUMERATED IN ORDER, and `Replace` on one swaps its name and
/// value inside the object (`JSON Management.ReplaceOrAddJPropertyInJObject`).
void PropertiesEnumerateAndReplace() {
  JObject object;
  object = object.Parse("{\"first\": 1, \"second\": 2}");
  GenericIEnumerator1 walker = object.Properties().GetEnumerator();
  std::string names;
  while (walker.MoveNext()) {
    JProperty property;
    property = walker.Current();
    names += T(property.Name()) + ";";
  }
  CHECK_TEXT("the names in document order", names, "first;second;");
  JProperty renamed;
  renamed = renamed.JProperty("third", Variant(agiru::Integer{3}));
  object.Property("second").Replace(renamed);
  CHECK_TEXT("Replace swaps name and value in place",
             T(object.ToString()),
             "{\r\n  \"first\": 1,\r\n  \"third\": 3\r\n}");
  CHECK_TRUE("Remove takes a member out", object.Remove("first"));
  CHECK_TRUE("and reports a missing one", !object.Remove("first"));
}

}

int main() {
  return gate::Run("JObject", [] {
    AnObjectIsARefereceIntoATreeAndRendersIndented();
    AVariantCarriesATokenAndFormatRendersIt();
    AnArrayEnumeratesAndFiltersItsObjects();
    PropertiesEnumerateAndReplace();
  });
}
