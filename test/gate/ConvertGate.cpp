#include "dotnet/Base64FormattingOptions.h"
#include "dotnet/Convert.h"
#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/BigText.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

#include "Check.h"
#include "Reference.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace {

using agiru::dotnet::Array;
using agiru::dotnet::Base64FormattingOptions;
using agiru::dotnet::Convert;

constexpr std::size_t kBytePopulation = 256;
constexpr std::size_t kLineBytes = 57;
constexpr std::size_t kLargeBytes = 8193;

Array Bytes(std::string_view value) {
  Array result;
  for (const char cell : value) {
    result.Add(agiru::Variant{agiru::Integer{static_cast<unsigned char>(cell)}});
  }
  return result;
}

std::string Raw(const Array &bytes) {
  std::string result;
  for (const auto &cell : bytes) {
    const agiru::Integer value = cell;
    CHECK_TRUE("decoded cells retain the Integer byte type",
               cell.IsInteger() && value >= 0 &&
                   value < static_cast<agiru::Integer>(kBytePopulation));
    result.push_back(static_cast<char>(value));
  }
  return result;
}

template <typename Operation> bool Fails(Operation operation) {
  try {
    operation();
  } catch (const agiru::Error &) { return true; }
  return false;
}

void KnownValues() {
  constexpr std::array<std::pair<std::string_view, std::string_view>, 8> cases{{
      {"", ""},
      {"f", "Zg=="},
      {"fo", "Zm8="},
      {"foo", "Zm9v"},
      {"foob", "Zm9vYg=="},
      {"fooba", "Zm9vYmE="},
      {"foobar", "Zm9vYmFy"},
      {std::string_view("\0\xff\x80", 3), "AP+A"},
  }};
  for (const auto &[bytes, encoded] : cases) {
    CHECK_TEXT(
        "Convert encodes bytes without a BOM", Convert::ToBase64String(Bytes(bytes)), encoded);
    CHECK_TEXT("Convert decodes exact byte values", Raw(Convert::FromBase64String(encoded)), bytes);
    CHECK_TEXT("Convert exact region excludes surrounding bytes",
               Convert::ToBase64String(Bytes("prefix" + std::string(bytes) + "suffix"),
                                       6,
                                       static_cast<agiru::Integer>(bytes.size())),
               encoded);
  }
  CHECK_TEXT(
      "unused pad bits retain CLR Convert acceptance", Raw(Convert::FromBase64String("TR==")), "M");
  CHECK_TEXT("whitespace may occur between padding cells",
             Raw(Convert::FromBase64String("T Q=\n=\r\t")),
             "M");
  auto first = Convert::FromBase64String("TQ==");
  const auto second = Convert::FromBase64String("TQ==");
  first.SetValue(agiru::Variant{agiru::Integer{0}}, 0);
  CHECK_TEXT("each decoder result has independent owned bytes", Raw(second), "M");
}

void Formatting() {
  const std::string line(kLineBytes, 'a');
  const auto plain = Convert::ToBase64String(Bytes(line));
  const auto breaks = Base64FormattingOptions::InsertLineBreaks();
  constexpr std::size_t kFullLinesAcrossBlock = 64;
  std::string expected;
  for (std::size_t index = 0; index < kFullLinesAcrossBlock; ++index) {
    expected += plain + "\r\n";
  }
  expected += "YQ==";
  CHECK_TEXT("a formatting block seam retains its CRLF",
             Convert::ToBase64String(
                 Bytes(std::string(kLineBytes * kFullLinesAcrossBlock + 1, 'a')), breaks),
             expected);
  CHECK_TEXT("a full final line has no trailing CRLF",
             Convert::ToBase64String(Bytes(line), breaks),
             plain);
  CHECK_TEXT("the next group is placed on a new line",
             Convert::ToBase64String(Bytes(line + "a"), breaks),
             plain + "\r\nYQ==");
  std::string binary;
  for (std::size_t index = 0; index < kLargeBytes; ++index) {
    binary.push_back(static_cast<char>(index % kBytePopulation));
  }
  const auto bytes = Bytes(binary);
  for (const auto options : {Base64FormattingOptions::None(), breaks}) {
    const auto text = Convert::ToBase64String(bytes, options);
    CHECK_TEXT(
        "multi-block encoded values round trip", Raw(Convert::FromBase64String(text)), binary);
    CHECK_TEXT(
        "a selected multi-block region restarts at its own first byte",
        Convert::ToBase64String(bytes, 1, static_cast<agiru::Integer>(kLargeBytes - 1), options),
        Convert::ToBase64String(Bytes(std::string_view(binary).substr(1)), options));
  }
}

void Refusals() {
  CHECK_TRUE("BigText uses its named adapter refusal rather than an empty substitute",
             Fails([] { (void)Convert::FromBase64String(agiru::BigText{}); }));
  for (const auto *const text :
       {"TQ", "TQ=", "T===", "TQ===", "TQ==TQ==", "TQ==\v", "TQ==\f", "TQ==\xc2\xa0", "!!!!"}) {
    CHECK_TRUE("invalid decoder input refuses without returning partial bytes",
               Fails([&] { (void)Convert::FromBase64String(text); }));
  }
  for (const auto value : {-1, static_cast<agiru::Integer>(kBytePopulation)}) {
    Array invalid;
    invalid.Add(agiru::Variant{value});
    CHECK_TRUE("out-of-range byte cells refuse rather than wrap",
               Fails([&] { (void)Convert::ToBase64String(invalid); }));
  }
  Array invalid;
  invalid.Add(agiru::Variant{agiru::Text<0>{"65"}});
  CHECK_TRUE("text is not a byte cell", Fails([&] { (void)Convert::ToBase64String(invalid); }));
  for (const auto value : {-1, 2, 3}) {
    CHECK_TRUE("invalid formatting options refuse even for empty input", Fails([&] {
                 (void)Convert::ToBase64String(Bytes(""), Base64FormattingOptions{value});
               }));
  }
  const auto bytes = Bytes("abc");
  constexpr std::array<std::array<agiru::Integer, 2>, 5> ranges{
      {{-1, 0}, {0, -1}, {4, 0}, {1, 3}, {std::numeric_limits<agiru::Integer>::max(), 1}}};
  for (const auto &range : ranges) {
    CHECK_TRUE("invalid regions refuse without overflow",
               Fails([&] { (void)Convert::ToBase64String(bytes, range[0], range[1]); }));
  }
  CHECK_TEXT("empty end-position region is valid", Convert::ToBase64String(bytes, 3, 0), "");
  CHECK_TRUE("unbuilt numeric conversion retains its named refusal",
             Fails([] { (void)Convert::ToInt32(1); }));
}

void CoreReference(std::string_view path) {
  std::ifstream input{std::string(path)};
  if (!input) { throw agiru::Error("Convert core reference cannot be opened"); }
  std::size_t rows = 0;
  for (std::string line; std::getline(input, line);) {
    ++rows;
    const auto fields = gate::ReferenceFields<4>(line);
    if (fields[0] == "E") {
      const auto options = fields[2] == "1" ? Base64FormattingOptions::InsertLineBreaks()
                                            : Base64FormattingOptions::None();
      CHECK_TEXT("original byte-core encoder agrees through Convert",
                 Convert::ToBase64String(Bytes(gate::Unhex(fields[1])), options),
                 gate::Unhex(fields[3]));
    } else if (fields[0] == "D") {
      std::string result;
      const bool failed =
          Fails([&] { result = Raw(Convert::FromBase64String(gate::Unhex(fields[1]))); });
      CHECK_TRUE("original byte-core decoder refusal agrees through Convert",
                 failed == (fields[3] == "FORMAT"));
      if (fields[3] != "FORMAT") {
        CHECK_TEXT("original byte-core decoder bytes agree through Convert",
                   result,
                   gate::Unhex(fields[3]));
      }
    } else {
      throw agiru::Error("Unknown Convert core reference operation");
    }
  }
  CHECK_TRUE("entire original core reference population retained", rows == 10051);
}

void RegionReference(std::string_view path) {
  constexpr std::size_t kStatusField = 5;
  constexpr std::size_t kOutputField = 6;
  std::ifstream input{std::string(path)};
  if (!input) { throw agiru::Error("Convert region reference cannot be opened"); }
  std::size_t rows = 0;
  for (std::string line; std::getline(input, line);) {
    ++rows;
    const auto fields = gate::ReferenceFields<7>(line);
    if (fields[0] != "R") { throw agiru::Error("Unknown Convert region reference operation"); }
    const auto bytes = Bytes(gate::Unhex(fields[1]));
    const auto offset = std::stoi(std::string(fields[2]));
    const auto length = std::stoi(std::string(fields[3]));
    const Base64FormattingOptions options{std::stoi(std::string(fields[4]))};
    std::string result;
    const bool failed =
        Fails([&] { result = Convert::ToBase64String(bytes, offset, length, options); });
    CHECK_TRUE("CLR region and option rejection agrees",
               failed == (fields[kStatusField] == "ERROR"));
    if (fields[kStatusField] == "OK") {
      CHECK_TEXT("CLR region and option bytes agree", result, gate::Unhex(fields[kOutputField]));
    }
  }
  CHECK_TRUE("entire CLR region reference population retained", rows == 710);
}

}

int main(int argc, char **argv) {
  return gate::Run("Convert", [&] {
    KnownValues();
    Formatting();
    Refusals();
    if (argc > 1) { CoreReference(argv[1]); }
    if (argc > 2) { RegionReference(argv[2]); }
    if (argc > 3) { throw agiru::Error("Convert expects at most two reference files"); }
  });
}
