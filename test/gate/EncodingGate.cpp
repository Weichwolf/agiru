#include "dotnet/Encoding.h"
#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"
#include "type/Utf8.h"

#include "Check.h"
#include "Reference.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

using agiru::dotnet::Encoding;

namespace {

static_assert(std::is_const_v<decltype(Encoding::Encoding)>);
static_assert(std::is_const_v<decltype(agiru::dotnet::UTF8Encoding::UTF8Encoding)>);
static_assert(std::is_const_v<decltype(agiru::dotnet::UnicodeEncoding::UnicodeEncoding)>);
static_assert(std::is_const_v<decltype(agiru::dotnet::ASCIIEncoding::ASCIIEncoding)>);

constexpr unsigned kAsciiBoundary = 128;
constexpr unsigned kTwoByteBoundary = 2048;
constexpr unsigned kUnitPopulation = 65536;
constexpr unsigned kBytePopulation = 256;
constexpr unsigned kContinuation = 0x80U;
constexpr unsigned kTwoByteLead = 0xC0U;
constexpr unsigned kThreeByteLead = 0xE0U;
constexpr unsigned kFourByteLead = 0xF0U;
constexpr unsigned kPayloadMask = 0x3FU;
constexpr unsigned kSixBits = 6U;
constexpr unsigned kTwelveBits = 12U;
constexpr unsigned kEighteenBits = 18U;
constexpr unsigned kByteBits = 8U;

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
  constexpr std::array<std::pair<std::string_view, std::string_view>, 14> cases{{
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
      {"EFBFBD", "EFBFBD"},
  }};
  for (const auto &[input, expected] : cases) {
    CHECK_TRUE(
        "strict UTF-8 validity matches unchanged decoder bytes, including literal replacement",
        agiru::IsValidUtf8(gate::Unhex(input)) == (input == expected));
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

void SingleBytePages() {
  const Encoding windows = Encoding::GetEncoding(agiru::Integer{1252});
  const Encoding latin = Encoding::GetEncoding(agiru::Integer{28591});
  CHECK_TEXT("Windows euro differs from the Latin-1 control",
             windows.Decode(gate::Unhex("80")),
             gate::Unhex("E282AC"));
  CHECK_TEXT(
      "Latin-1 retains its C1 control", latin.Decode(gate::Unhex("80")), gate::Unhex("C280"));
  CHECK_TEXT("Windows smart punctuation is not Latin-1",
             windows.Decode(gate::Unhex("919293949697")),
             gate::Unhex("E28098E28099E2809CE2809DE28093E28094"));
  CHECK_TEXT("undefined Windows byte identities roundtrip as controls",
             windows.Decode(gate::Unhex("818D8F909D")),
             gate::Unhex("C281C28DC28FC290C29D"));
  CHECK_TEXT("Windows reverse mapping preserves the euro",
             windows.Encode(gate::Unhex("E282AC")),
             gate::Unhex("80"));
  CHECK_TEXT("best-fit is a byte mapping, not Unicode normalization",
             windows.Encode(gate::Unhex("65CC81")),
             gate::Unhex("65B4"));
  CHECK_TEXT("best-fit folds fullwidth and accented characters",
             windows.Encode(gate::Unhex("C480EFBCA1")),
             "AA");
  CHECK_TEXT("Latin-1 never borrows Windows best-fit", latin.Encode(gate::Unhex("65CC81")), "e?");
  CHECK_TEXT("Latin-1 retains its own punctuation best-fit",
             latin.Encode(gate::Unhex("E282ACE28098E28099E2809CE2809DE28093E28094")),
             "?''\"\"--");
  CHECK_TEXT(
      "ASCII replaces each non-ASCII byte", Encoding::ASCII().Decode(gate::Unhex("80FF")), "??");
  for (const Encoding &encoding : {windows, latin, Encoding::ASCII()}) {
    CHECK_TEXT("single-byte encoders replace both supplementary UTF-16 units",
               encoding.Encode(gate::Unhex("F09F9880")),
               "??");
    CHECK_TEXT("single-byte encoders replace an isolated surrogate once",
               encoding.Encode(gate::Unhex("EDA080")),
               "?");
  }
  CHECK_TEXT("Latin-1 reports its actual codepage name", latin.WebName(), "iso-8859-1");
}

void EncodingFactories() {
  CHECK_TRUE("numeric UTF-8 retains its separately requested preamble",
             Encoding::GetEncoding(agiru::Integer{65001}).GetPreamble().Length() == 3);
  CHECK_TRUE("named UTF-8 selects the same preamble",
             Encoding::GetEncoding("UTF-8").GetPreamble().Length() == 3);
  CHECK_TRUE("Default does not acquire the UTF8 singleton preamble",
             Encoding::Default().GetPreamble().Length() == 0);
  CHECK_TRUE("Latin-1 aliases do not select Windows-1252",
             Encoding::GetEncoding("latin1").CodePage() == 28591);
  CHECK_TRUE("cp819 selects Latin-1", Encoding::GetEncoding("cp819").CodePage() == 28591);
  CHECK_TRUE("cp1252 selects Windows", Encoding::GetEncoding("cp1252").CodePage() == 1252);
  CHECK_TRUE("x-ansi selects Windows", Encoding::GetEncoding("x-ansi").CodePage() == 1252);
  CHECK_TRUE("UTF-16LE is the little-endian alias",
             Encoding::GetEncoding("utf-16le").CodePage() == 1200);
  CHECK_TRUE("UTF-32LE is the little-endian alias",
             Encoding::GetEncoding("utf-32le").CodePage() == 12000);
  for (const char *name : {"utf8", "utf32", "latin-1", "x-cp1252", "windows-1251", "iso-8859-2"}) {
    bool refused = false;
    try {
      static_cast<void>(Encoding::GetEncoding(name));
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("unknown aliases and unimplemented pages never select an unrelated encoding",
               refused);
  }
  for (const int page : {-1, 437, 1251, 28592, 65536}) {
    bool refused = false;
    try {
      static_cast<void>(Encoding::GetEncoding(agiru::Integer{page}));
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("unimplemented numeric pages never fall through to Latin-1", refused);
  }
  const Encoding unimplemented = Encoding::Made(437, false);
  bool encodeRefused = false;
  bool decodeRefused = false;
  try {
    static_cast<void>(unimplemented.Encode(""));
  } catch (const agiru::Error &) { encodeRefused = true; }
  try {
    static_cast<void>(unimplemented.Decode(""));
  } catch (const agiru::Error &) { decodeRefused = true; }
  CHECK_TRUE("raw unimplemented encoders refuse even empty input", encodeRefused);
  CHECK_TRUE("raw unimplemented decoders refuse even empty input", decodeRefused);
}

std::string ScalarInput(unsigned code) {
  std::string text;
  if (code < kAsciiBoundary) {
    text += static_cast<char>(code);
  } else if (code < kTwoByteBoundary) {
    text += static_cast<char>(kTwoByteLead | (code >> kSixBits));
    text += static_cast<char>(kContinuation | (code & kPayloadMask));
  } else if (code < kUnitPopulation) {
    text += static_cast<char>(kThreeByteLead | (code >> kTwelveBits));
    text += static_cast<char>(kContinuation | ((code >> kSixBits) & kPayloadMask));
    text += static_cast<char>(kContinuation | (code & kPayloadMask));
  } else {
    text += static_cast<char>(kFourByteLead | (code >> kEighteenBits));
    text += static_cast<char>(kContinuation | ((code >> kTwelveBits) & kPayloadMask));
    text += static_cast<char>(kContinuation | ((code >> kSixBits) & kPayloadMask));
    text += static_cast<char>(kContinuation | (code & kPayloadMask));
  }
  return text;
}

unsigned HexUnit(std::string_view hex) {
  const std::string bytes = gate::Unhex(hex);
  if (bytes.size() != 2) { throw agiru::Error("codepage reference requires one UTF-16 unit"); }
  return (static_cast<unsigned>(static_cast<unsigned char>(bytes[0])) << kByteBits) |
         static_cast<unsigned char>(bytes[1]);
}

void CodePageReference(const char *path, int page) {
  std::array<bool, kUnitPopulation> encoded{};
  std::array<bool, kBytePopulation> decoded{};
  std::size_t encodeRows = 0;
  std::size_t decodeRows = 0;
  std::ifstream input(path);
  if (!input) { throw agiru::Error("cannot open codepage reference fixture"); }
  const Encoding encoding = Encoding::GetEncoding(agiru::Integer{page});
  std::string line;
  while (std::getline(input, line)) {
    const auto fields = gate::ReferenceFields<3>(line);
    if (fields[0] == "E") {
      const unsigned unit = HexUnit(fields[1]);
      if (encoded[unit]) { throw agiru::Error("duplicate codepage encode identity"); }
      encoded[unit] = true;
      ++encodeRows;
      const std::string expected = gate::Unhex(fields[2]);
      if (expected.size() != 1) {
        throw agiru::Error("codepage reference requires one output byte");
      }
      CHECK_TEXT("every UTF-16 unit matches original native codepage bytes",
                 encoding.Encode(ScalarInput(unit)),
                 expected);
    } else if (fields[0] == "D") {
      const std::string bytes = gate::Unhex(fields[1]);
      if (bytes.size() != 1) { throw agiru::Error("codepage reference requires one input byte"); }
      const auto byte = static_cast<unsigned char>(bytes[0]);
      if (decoded[byte]) { throw agiru::Error("duplicate codepage decode identity"); }
      decoded[byte] = true;
      ++decodeRows;
      CHECK_TEXT("every byte matches original native codepage UTF-16 units",
                 encoding.Decode(bytes),
                 ScalarInput(HexUnit(fields[2])));
    } else {
      throw agiru::Error("unknown codepage reference operation");
    }
  }
  if (!input.eof()) { throw agiru::Error("cannot read codepage reference fixture"); }
  CHECK_TRUE("the complete BMP encode population remains counted", encodeRows == kUnitPopulation);
  CHECK_TRUE("the complete byte decode population remains counted", decodeRows == kBytePopulation);
  constexpr unsigned kLastScalar = 0x10FFFF;
  for (unsigned point = kUnitPopulation; point <= kLastScalar; ++point) {
    CHECK_TEXT("every supplementary scalar matches the original two-unit fallback",
               encoding.Encode(ScalarInput(point)),
               "??");
  }
  std::println("Codepage {} reference: {} encodes, {} decodes, {} supplementary scalars",
               page,
               encodeRows,
               decodeRows,
               kLastScalar - kUnitPopulation + 1);
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
    } else if (fields[2] == "1252") {
      encoding = Encoding::GetEncoding(agiru::Integer{Encoding::kWindows1252});
    } else if (fields[2] == "20127") {
      encoding = Encoding::ASCII();
    } else if (fields[2] == "28591") {
      encoding = Encoding::GetEncoding(agiru::Integer{Encoding::kLatin1});
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
  CHECK_TEXT("shared declared encoding profile matches the original BC29 text core",
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
  CHECK_TRUE("the original encoding reference population is nonempty", selected != 0);
  std::println("Encoding reference: {} total, {} selected, {} outside the declared profile",
               selected + outside,
               selected,
               outside);
}

}

int main(int argc, char **argv) {
  return gate::Run("Encoding", [&] {
    if (argc == 3 && std::string_view(argv[1]) == "--codepage-reference") {
      CodePageReference(argv[2], Encoding::kWindows1252);
      return;
    }
    if (argc == 3 && std::string_view(argv[1]) == "--ascii-reference") {
      CodePageReference(argv[2], Encoding::kAscii);
      return;
    }
    if (argc == 3 && std::string_view(argv[1]) == "--latin1-reference") {
      CodePageReference(argv[2], Encoding::kLatin1);
      return;
    }
    if (argc > 2) { throw agiru::Error("EncodingGate requires a declared reference mode"); }
    if (argc == 2) {
      Reference(argv[1]);
      return;
    }
    CodePageZeroIsTheAnsiPage();
    Utf8Replacement();
    Utf16Replacement();
    Utf32Replacement();
    CharacterArrays();
    SingleBytePages();
    EncodingFactories();
  });
}
