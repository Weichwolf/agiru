#include "type/Char.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"
#include "fixture/codeunit/TextPositionConsumer.h"

int main() {
  return gate::Run("Generated Text Positions", [] {
    agiru::Fixture::TextPositionConsumer_Codeunit consumer;
    CHECK_TRUE("generated AL reads a Unicode character rather than its first UTF-8 byte",
               consumer.ReadPosition("AÆØÅ€Z", 2) == 198);
    CHECK_TRUE("generated AL reads the character after a surrogate pair",
               consumer.ReadPosition("A💡Z", 4) == 90);
    CHECK_TEXT("generated AL replaces the complete character",
               consumer.ReplacePosition("AÆØÅ€Z", 2, agiru::Char{65}).Value(),
               "AAØÅ€Z");
    CHECK_TEXT("generated AL encodes a wider replacement",
               consumer.ReplacePosition("AÆØÅ€Z", 3, agiru::Char{8364}).Value(),
               "AÆ€Å€Z");
    CHECK_TEXT("generated AL copies values between same-type text proxies",
               consumer.CopyPosition("AB", "ÆØ", 1, 2).Value(),
               "ØB");
    CHECK_TEXT("generated AL loops preserve positions as UTF-8 widths change",
               consumer.ReplaceEach("ABCÆØÅ@!", agiru::Char{88}).Value(),
               "XXXXXXXX");
    CHECK_TEXT("generated AL appends at its UTF-16 terminator position",
               consumer.ReplacePosition("ÆØ", 3, agiru::Char{8364}).Value(),
               "ÆØ€");
    CHECK_TEXT("generated AL retains a bounded destination's capacity",
               consumer.AppendBounded("ÆØ").Value(),
               "ÆØ€");
    bool refused = false;
    try {
      (void)consumer.AppendBounded("ÆØÅ");
    } catch (const agiru::StringError &) { refused = true; }
    CHECK_TRUE("generated AL refuses an over-capacity append", refused);
    refused = false;
    try {
      (void)consumer.ReplacePosition("A💡Z", 2, agiru::Char{"X"});
    } catch (const agiru::StringError &) { refused = true; }
    CHECK_TRUE("generated AL explicitly refuses unsupported half-surrogate replacement", refused);
  });
}
