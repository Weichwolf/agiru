#include "meta/EnumDef.h"
#include "type/Date.h"
#include "type/Enum.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Time.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <array>
#include <cstdint>
#include <exception>
#include <string>

namespace {

/// The two enumerations `devenv-format-property.md` tabulates separately: an OPTION, whose members
/// are the numbers 0, 1, 2, and an ENUM, whose ordinals are declared.
enum class Colour : std::int32_t { Blue = 0, Bronze = 1, Silver = 2 };

enum class YesNo : std::int32_t { Yes = 0, No = 10 };

} // namespace

template <> struct agiru::OptionTraits<Colour> {
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      agiru::EnumValueDef{.ordinal = 0, .name = "Blue", .caption = "Blue"},
      agiru::EnumValueDef{.ordinal = 1, .name = "Bronze", .caption = "Bronze"},
      agiru::EnumValueDef{.ordinal = 2, .name = "Silver", .caption = "Silver"},
  }};
};

template <> struct agiru::EnumTraits<YesNo> {
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      agiru::EnumValueDef{.ordinal = 0, .name = "Yes", .caption = "Yes"},
      agiru::EnumValueDef{.ordinal = 10, .name = "No", .caption = "No"},
  }};
};

namespace {

constexpr agiru::Integer kUntabulatedFormat = 5;

// The date every table in `devenv-format-property.md` illustrates itself with, and the time beside
// it: `2021-04-05` reads `040521D` as an AL code constant, `04:35:55.553` reads `043555.553T`.
constexpr agiru::Date kExampleDate = agiru::Date::FromYmd(2021, 4, 5);
constexpr agiru::Time kExampleTime = agiru::Time::FromHms(4, 35, 55, 553);

using agiru::Format;
using agiru::Variant;

std::string Raised(const auto &body) {
  try {
    body();
  } catch (const std::exception &e) { return e.what(); }
  return {};
}

// THE TABLE IS THE CASE. `devenv-format-property.md`, "Standard option formats" and "Standard enum
// formats": <Text> for 0 and 1, <Number> for 2 and 9. Both tables say the same thing, which is why
// one Variant alternative carries both.
void AnEnumerationRendersItsTextForZeroAndOneAndItsNumberForTwoAndNine() {
  const Variant colour{agiru::Option<Colour>{Colour::Bronze}};
  CHECK_TEXT("format 0 is the member text", Format(colour, 0, 0), "Bronze");
  CHECK_TEXT("format 1 is the member text", Format(colour, 0, 1), "Bronze");
  CHECK_TEXT("format 2 is the number", Format(colour, 0, 2), "1");
  CHECK_TEXT("format 9 is the number", Format(colour, 0, 9), "1");

  const Variant answer{agiru::Enum<YesNo>{YesNo::No}};
  CHECK_TEXT("an enum renders its text the same way", Format(answer, 0, 0), "No");
  CHECK_TEXT("and its DECLARED ordinal, not its position", Format(answer, 0, 2), "10");
}

// The negative control for the alternative itself: `Assert.Equal` decides on `TypeOf`, so an option
// and an integer of the same ordinal must NOT be the same thing in a Variant.
void AnOptionIsNotAnInteger() {
  const Variant colour{agiru::Option<Colour>{Colour::Bronze}};
  const Variant one{agiru::Integer{1}};
  CHECK_TRUE("the option answers IsOption", colour.IsOption());
  CHECK_TRUE("an enum answers IsOption too, because no IsEnum is documented",
             Variant{agiru::Enum<YesNo>{YesNo::Yes}}.IsOption());
  CHECK_TRUE("the integer does not", !one.IsOption());
  CHECK_TRUE("and the two are not equal, though both render as 1 under format 2", !(colour == one));
  CHECK_TEXT("which they do", Format(colour, 0, 2), Format(one, 0, 2));
}

// AL writes the member itself into an `Any` parameter -- `Assert.AreEqual(Enum::"X"::Y, ...)` --
// and that is a bare enumerator, not a variable of the holder type.
void ABareMemberReachesAVariant() {
  CHECK_TEXT("an option member", Format(Variant{Colour::Silver}, 0, 0), "Silver");
  CHECK_TEXT("an enum member", Format(Variant{YesNo::No}, 0, 2), "10");
}

// "Standard date formats", the AL code constant column: <Month,2><Day,2><Year><Closing>D.
void ADateRendersTheCodeConstantForFormatTwo() {
  const Variant when{kExampleDate};
  CHECK_TEXT("format 2 is the AL code constant", Format(when, 0, 2), "040521D");
  CHECK_TEXT("format 9 is the XML format", Format(when, 0, 9), "2021-04-05");
  const Variant closing{kExampleDate.Closing()};
  CHECK_TEXT("a closing date carries its marker, where the table puts <Closing> before the D",
             Format(closing, 0, 2),
             "040521CD");
}

void ATimeRendersTheCodeConstantForFormatTwo() {
  const Variant when{kExampleTime};
  CHECK_TEXT("format 2 is hhmmss.fffT", Format(when, 0, 2), "043555.553T");
}

// The Boolean table gives <Text> for 0 and 1, <Number> for 2 and the XML spelling for 9. The text
// is AL's own Yes/No and not the table's illustrative True/False -- the predecessor measured a test
// that rebuilt an expected message through Format(false) and looked for "No" in it.
void ABooleanRendersThreeDifferentWays() {
  const Variant no{false};
  CHECK_TEXT("format 0 is the AL text", Format(no, 0, 0), "No");
  CHECK_TEXT("format 2 is the number", Format(no, 0, 2), "0");
  CHECK_TEXT("format 9 is the XML spelling", Format(no, 0, 9), "false");
}

// The specification form. `<Integer,2><Filler Character,0>` is the commonest one in Layers/W1 at 26
// sites, and the filler is written AFTER the element it fills.
void ASpecificationFillsFromTheWholeStringAndNotFromTheLeft() {
  const Variant seven{agiru::Integer{7}};
  CHECK_TEXT("the filler reaches back", Format(seven, 0, "<Integer,2><Filler Character,0>"), "07");
  CHECK_TEXT("literal text between elements survives",
             Format(seven, 0, "no. <Integer,3><Filler Character,0>"),
             "no. 007");
  CHECK_TEXT("a sign is empty for a positive number", Format(seven, 0, "<Sign><Integer>"), "7");
  CHECK_TEXT("and present for a negative one",
             Format(Variant{agiru::Integer{-7}}, 0, "<Sign><Integer>"),
             "-7");
}

// AL's own `LibraryUtility` builds a mod-97 code as `ConvertStr(Format(N, 8, '<Integer>'), ' ',
// '0')`, which only produces eight digits if the LENGTH padded the number to eight.
void TheLengthPadsANumberOnTheLeftAndTextOnTheRight() {
  CHECK_TEXT("a number is right-justified",
             Format(Variant{agiru::Integer{123}}, 8, "<Integer>"),
             "     123");
  CHECK_TEXT("text is left-justified", Format(Variant{"ab"}, 4, 0), "ab  ");
  CHECK_TEXT(
      "and a longer result is cut rather than padded", Format(Variant{"abcdef"}, 4, 0), "abcd");
}

// A HALF-UNDERSTOOD SPECIFICATION IS THE DEFECT THIS REFUSES TO BE. An element that is not
// rendered names itself, so the failure says which one rather than producing a plausible string.
void AnElementThatIsNotRenderedNamesItself() {
  const std::string said =
      Raised([] { return Format(Variant{kExampleDate}, 0, "<Month Text,3>"); });
  CHECK_TRUE("the refusal names the element", said.find("<month text>") != std::string::npos);
  const std::string number =
      Raised([] { return Format(Variant{agiru::Integer{1}}, 0, kUntabulatedFormat); });
  CHECK_TRUE("and an untabulated standard format names its number",
             number.find("standard format 5") != std::string::npos);
}

} // namespace

int main() {
  return gate::Run("Format", [] {
    AnEnumerationRendersItsTextForZeroAndOneAndItsNumberForTwoAndNine();
    AnOptionIsNotAnInteger();
    ABareMemberReachesAVariant();
    ADateRendersTheCodeConstantForFormatTwo();
    ATimeRendersTheCodeConstantForFormatTwo();
    ABooleanRendersThreeDifferentWays();
    ASpecificationFillsFromTheWholeStringAndNotFromTheLeft();
    TheLengthPadsANumberOnTheLeftAndTextOnTheRight();
    AnElementThatIsNotRenderedNamesItself();
  });
}
