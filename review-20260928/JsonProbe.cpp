#include "runtime/Error.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/Text.h"

#include <cstdio>
#include <string>
#include <string_view>

int main(int argc, char **argv) {
  try {
    if (argc == 2 && std::string_view(argv[1]) == "alias") {
      agiru::JsonObject object;
      object.Add("kept", agiru::Integer{1});
      agiru::JsonToken kept;
      if (!object.Get("kept", kept)) { return 2; }
      for (int i = 0; i < 128; ++i) {
        object.Add("sibling" + std::to_string(i), agiru::Integer{i});
      }
      const int actual = kept.AsValue().AsInteger();
      std::printf("retained child after sibling insertions: %d; expected 1\n", actual);
      return actual == 1 ? 0 : 1;
    }
    int failed = 0;
    for (const std::string_view text : {"1.25", "999999999999999.99", "0.1234567890123456789012345678"}) {
      agiru::JsonObject object;
      const auto expected = agiru::Decimal::FromInvariantString(text);
      object.Add("amount", expected);
      agiru::JsonToken token;
      if (!object.Get("amount", token)) { return 2; }
      agiru::Text<0> serialized;
      object.WriteTo(serialized);
      try {
      const auto actual = token.AsValue().AsDecimal();
      std::printf("decimal JSON round trip: expected=%s actual=%s JSON=%s\n",
                  std::string(text).c_str(), actual.ToInvariantString().c_str(),
                  std::string(std::string_view(serialized)).c_str());
      failed += actual != expected;
      } catch (const agiru::Error &error) {
        std::printf("decimal JSON round trip: expected=%s JSON=%s refusal=%s\n",
                    std::string(text).c_str(), std::string(std::string_view(serialized)).c_str(), error.what());
        ++failed;
      }
    }
    std::printf("decimal contract failures: %d\n", failed);
    return failed == 0 ? 0 : 1;
  } catch (const agiru::Error &error) {
    std::fprintf(stderr, "probe error: %s\n", error.what());
    return 2;
  }
}
