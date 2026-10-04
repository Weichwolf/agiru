#include "dotnet/Encoding.h"
#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"

#include "Check.h"
#include "Reference.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <utility>

using agiru::dotnet::Encoding;

namespace {

void CodePageZeroIsTheAnsiPage() {
  const Encoding ansi = Encoding::GetEncoding(agiru::Integer{0});
  CHECK_TEXT("bytes E6 F8 E5 read as Latin letters",
             ansi.Decode("\xE6\xF8\xE5"),
             "\xC3\xA6\xC3\xB8\xC3\xA5");
  CHECK_TEXT("and the letters write as the same bytes",
             ansi.Encode("\xC3\xA6\xC3\xB8\xC3\xA5"),
             "\xE6\xF8\xE5");
  CHECK_TRUE("the same page by name",
             Encoding::GetEncoding("windows-1252").Decode("\xE6") == ansi.Decode("\xE6"));
  CHECK_TEXT("Default stays UTF-8", Encoding::Default().Decode("\xC3\xA6"), "\xC3\xA6");
  CHECK_TEXT("and UTF-8 round-trips",
             Encoding::GetEncoding(agiru::Integer{65001}).Encode("\xC3\xA6"),
             "\xC3\xA6");
}

void Utf8Replacement() {
  constexpr std::array<std::pair<std::string_view, std::string_view>, 13> cases{{
      {"80", "EFBFBD"},
      {"C080", "EFBFBDEFBFBD"},
      {"E08080", "EFBFBDEFBFBDEFBFBD"},
      {"EDA080", "EFBFBDEFBFBDEFBFBD"},
      {"F4908080", "EFBFBDEFBFBDEFBFBDEFBFBD"},
      {"F5808080", "EFBFBDEFBFBDEFBFBDEFBFBD"},
      {"E282", "EFBFBD"},
      {"E28241", "EFBFBD41"},
      {"F09080", "EFBFBD"},
      {"C241", "EFBFBD41"},
      {"EFBBBF00", "EFBBBF00"},
      {"F48FBFBF", "F48FBFBF"},
      {"F09F9880", "F09F9880"},
  }};
  for (const auto &[input, expected] : cases) {
    CHECK_TEXT("UTF-8 maximal valid subparts determine replacement count",
               Encoding::UTF8().Decode(gate::Unhex(input)),
               gate::Unhex(expected));
  }
  CHECK_TEXT("UTF-8 encoder replaces an isolated UTF-16 surrogate once",
             Encoding::UTF8().Encode(gate::Unhex("EDA080")),
             gate::Unhex("EFBFBD"));
  CHECK_TEXT("UTF-8 encoder joins an actual UTF-16 surrogate pair",
             Encoding::UTF8().Encode(gate::Unhex("EDA0BDEDB880")),
             gate::Unhex("F09F9880"));
  CHECK_TEXT("an unpaired high surrogate does not swallow the next character",
             Encoding::UTF8().Encode(gate::Unhex("EDA08041")),
             gate::Unhex("EFBFBD41"));
}

void Utf16Replacement() {
  constexpr std::array<std::pair<std::string_view, std::string_view>, 8> cases{{
      {"00D8", "EFBFBD"},
      {"00DC", "EFBFBD"},
      {"00D84100", "EFBFBD41"},
      {"00D841", "EFBFBDEFBFBD"},
      {"00D800D800DC", "EFBFBDF0908080"},
      {"3DD800DE", "F09F9880"},
      {"FFFE0000", "EFBBBF00"},
      {"41", "EFBFBD"},
  }};
  for (const auto &[input, expected] : cases) {
    CHECK_TEXT("UTF-16 validates pairs and preserves odd trailing-byte replacements",
               Encoding::Unicode().Decode(gate::Unhex(input)),
               gate::Unhex(expected));
  }
  CHECK_TEXT("UTF-16 encoder emits two units without a BOM",
             Encoding::Unicode().Encode(gate::Unhex("F09F9880")),
             gate::Unhex("3DD800DE"));
  CHECK_TEXT("UTF-16 encoder replaces an isolated surrogate",
             Encoding::Unicode().Encode(gate::Unhex("EDB080")),
             gate::Unhex("FDFF"));
}

void Utf32Replacement() {
  constexpr std::array<std::pair<std::string_view, std::string_view>, 8> cases{{
      {"00D80000", "EFBFBD"},
      {"00001100", "EFBFBD"},
      {"FFFFFFFF", "EFBFBD"},
      {"FFFF1000", "F48FBFBF"},
      {"00F60100", "F09F9880"},
      {"01", "EFBFBD"},
      {"0102", "EFBFBD"},
      {"010203", "EFBFBD"},
  }};
  for (const auto &[input, expected] : cases) {
    CHECK_TEXT("UTF-32 validates scalars and replaces one incomplete unit",
               Encoding::UTF32().Decode(gate::Unhex(input)),
               gate::Unhex(expected));
  }
  CHECK_TEXT("UTF-32 encoder writes a scalar rather than two surrogate units",
             Encoding::UTF32().Encode(gate::Unhex("EDA0BDEDB880")),
             gate::Unhex("00F60100"));
}

void CharacterArrays() {
  const auto bytes = Encoding::UTF8().GetBytes(gate::Unhex("41F09F9880"));
  const auto chars = Encoding::UTF8().GetChars(bytes);
  CHECK_TRUE("char arrays count UTF-16 units, not Unicode scalars", chars.Length() == 3);
  const agiru::Integer high = chars.GetValue(1);
  const agiru::Integer low =
      chars.Length() > 2 ? agiru::Integer{chars.GetValue(2)} : agiru::Integer{-1};
  CHECK_TRUE("the high surrogate is retained", high == 0xD83D);
  CHECK_TRUE("the low surrogate is retained", low == 0xDE00);
  const auto paired = Encoding::UTF8().GetBytes(chars, 1, 2);
  CHECK_TEXT("char-array encoding joins a pair",
             Encoding::UTF8().GetString(paired),
             gate::Unhex("F09F9880"));
  const auto isolated = Encoding::UTF8().GetBytes(chars, 1, 1);
  CHECK_TEXT("a char-array slice may isolate a high surrogate",
             Encoding::UTF8().GetString(isolated),
             gate::Unhex("EFBFBD"));
  const auto offset = Encoding::UTF8().GetChars(bytes, 1, 4);
  CHECK_TRUE("byte-array slices retain the same pair", offset.Length() == 2);
  CHECK_TRUE("Unicode byte conversion emits no preamble", Encoding::Unicode().Encode("").empty());
  CHECK_TRUE("Unicode preambles remain separately available",
             Encoding::Unicode().GetPreamble().Length() == 2);
}

bool ReferenceRow(const std::string &line) {
  const auto fields = gate::ReferenceFields<5>(line);
  if (fields[0] != "E" && fields[0] != "D") {
    throw agiru::Error("unknown encoding reference operation");
  }
  Encoding encoding;
  if (fields[1] == "1") {
    encoding = Encoding::UTF8();
  } else if (fields[1] == "2") {
    encoding = Encoding::Unicode();
  } else if (fields[1] == "0" || fields[1] == "3") {
    if (fields[2] == "65001") {
      encoding = Encoding::UTF8();
    } else if (fields[2] == "1200") {
      encoding = Encoding::Unicode();
    } else if (fields[2] == "12000") {
      encoding = Encoding::UTF32();
    } else {
      return false;
    }
  } else if (fields[1] == "99") {
    return false;
  } else {
    throw agiru::Error("unknown encoding reference type");
  }
  const std::string input = gate::Unhex(fields[3]);
  const std::string expected = gate::Unhex(fields[4]);
  CHECK_TEXT("shared Unicode codec matches the original BC29 text core",
             fields[0] == "E" ? encoding.Encode(input) : encoding.Decode(input),
             expected);
  return true;
}

void Reference(const char *path) {
  std::ifstream input(path);
  if (!input) { throw agiru::Error("cannot open encoding reference fixture"); }
  std::size_t selected = 0;
  std::size_t outside = 0;
  std::string line;
  while (std::getline(input, line)) {
    if (ReferenceRow(line)) {
      ++selected;
    } else {
      ++outside;
    }
  }
  if (!input.eof()) { throw agiru::Error("cannot read encoding reference fixture"); }
  CHECK_TRUE("the original Unicode reference population is nonempty", selected != 0);
  std::println("Encoding reference: {} total, {} Unicode, {} outside Unicode profile",
               selected + outside,
               selected,
               outside);
}

}

int main(int argc, char **argv) {
  return gate::Run("Encoding", [&] {
    if (argc > 2) { throw agiru::Error("EncodingGate accepts at most one reference fixture"); }
    if (argc == 2) {
      Reference(argv[1]);
      return;
    }
    CodePageZeroIsTheAnsiPage();
    Utf8Replacement();
    Utf16Replacement();
    Utf32Replacement();
    CharacterArrays();
  });
}
