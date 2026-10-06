#include "dotnet/CultureInfo.h"

#include "type/Integer.h"
#include "type/Language.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::int32_t kInvariantLcid = 127;

constexpr std::array<CultureValue::Identity, 52> kCultures{{
    {.lcid = kInvariantLcid, .name = "", .two = "iv", .three = "IVL"},
    {.lcid = 1025, .name = "ar-SA", .two = "ar", .three = "ARA"},
    {.lcid = 1026, .name = "bg-BG", .two = "bg", .three = "BGR"},
    {.lcid = 1027, .name = "ca-ES", .two = "ca", .three = "CAT"},
    {.lcid = 1028, .name = "zh-TW", .two = "zh", .three = "CHT"},
    {.lcid = 1029, .name = "cs-CZ", .two = "cs", .three = "CSY"},
    {.lcid = 1030, .name = "da-DK", .two = "da", .three = "DAN"},
    {.lcid = 1031, .name = "de-DE", .two = "de", .three = "DEU"},
    {.lcid = 1032, .name = "el-GR", .two = "el", .three = "ELL"},
    {.lcid = 1033, .name = "en-US", .two = "en", .three = "ENU"},
    {.lcid = 1034, .name = "es-ES_tradnl", .two = "es", .three = "ESP"},
    {.lcid = 1035, .name = "fi-FI", .two = "fi", .three = "FIN"},
    {.lcid = 1036, .name = "fr-FR", .two = "fr", .three = "FRA"},
    {.lcid = 1037, .name = "he-IL", .two = "he", .three = "HEB"},
    {.lcid = 1038, .name = "hu-HU", .two = "hu", .three = "HUN"},
    {.lcid = 1039, .name = "is-IS", .two = "is", .three = "ISL"},
    {.lcid = 1040, .name = "it-IT", .two = "it", .three = "ITA"},
    {.lcid = 1041, .name = "ja-JP", .two = "ja", .three = "JPN"},
    {.lcid = 1042, .name = "ko-KR", .two = "ko", .three = "KOR"},
    {.lcid = 1043, .name = "nl-NL", .two = "nl", .three = "NLD"},
    {.lcid = 1044, .name = "nb-NO", .two = "nb", .three = "NOR"},
    {.lcid = 1045, .name = "pl-PL", .two = "pl", .three = "PLK"},
    {.lcid = 1046, .name = "pt-BR", .two = "pt", .three = "PTB"},
    {.lcid = 1048, .name = "ro-RO", .two = "ro", .three = "ROM"},
    {.lcid = 1049, .name = "ru-RU", .two = "ru", .three = "RUS"},
    {.lcid = 1050, .name = "hr-HR", .two = "hr", .three = "HRV"},
    {.lcid = 1051, .name = "sk-SK", .two = "sk", .three = "SKY"},
    {.lcid = 1053, .name = "sv-SE", .two = "sv", .three = "SVE"},
    {.lcid = 1054, .name = "th-TH", .two = "th", .three = "THA"},
    {.lcid = 1055, .name = "tr-TR", .two = "tr", .three = "TRK"},
    {.lcid = 1057, .name = "id-ID", .two = "id", .three = "IND"},
    {.lcid = 1058, .name = "uk-UA", .two = "uk", .three = "UKR"},
    {.lcid = 1060, .name = "sl-SI", .two = "sl", .three = "SLV"},
    {.lcid = 1061, .name = "et-EE", .two = "et", .three = "ETI"},
    {.lcid = 1062, .name = "lv-LV", .two = "lv", .three = "LVI"},
    {.lcid = 1063, .name = "lt-LT", .two = "lt", .three = "LTH"},
    {.lcid = 1066, .name = "vi-VN", .two = "vi", .three = "VIT"},
    {.lcid = 1069, .name = "eu-ES", .two = "eu", .three = "EUQ"},
    {.lcid = 1086, .name = "ms-MY", .two = "ms", .three = "MSL"},
    {.lcid = 2052, .name = "zh-CN", .two = "zh", .three = "CHS"},
    {.lcid = 2055, .name = "de-CH", .two = "de", .three = "DES"},
    {.lcid = 2057, .name = "en-GB", .two = "en", .three = "ENG"},
    {.lcid = 2058, .name = "es-MX", .two = "es", .three = "ESM"},
    {.lcid = 2060, .name = "fr-BE", .two = "fr", .three = "FRB"},
    {.lcid = 2064, .name = "it-CH", .two = "it", .three = "ITS"},
    {.lcid = 2067, .name = "nl-BE", .two = "nl", .three = "NLB"},
    {.lcid = 2068, .name = "nn-NO", .two = "nn", .three = "NON"},
    {.lcid = 2070, .name = "pt-PT", .two = "pt", .three = "PTG"},
    {.lcid = 3079, .name = "de-AT", .two = "de", .three = "DEA"},
    {.lcid = 3081, .name = "en-AU", .two = "en", .three = "ENA"},
    {.lcid = 3082, .name = "es-ES", .two = "es", .three = "ESN"},
    {.lcid = 3084, .name = "fr-CA", .two = "fr", .three = "FRC"},
}};

bool SameTag(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

}

CultureValue::CultureValue(Identity identity)
    : name_(identity.name), two_(identity.two), three_(identity.three), lcid_(identity.lcid) {}

CultureValue CultureInfo::Binder::operator()(Integer lcid) const {
  return GetCultureInfo(lcid);
}

CultureValue CultureInfo::Binder::operator()(std::string_view name) const {
  return GetCultureInfo(name);
}

CultureValue CultureInfo::InvariantCulture() {
  return CultureValue{kCultures.front()};
}

CultureValue CultureInfo::CurrentCulture() {
  return GetCultureInfo(Language::Current());
}

CultureValue CultureInfo::GetCultureInfo(Integer lcid) {
  const auto *const found = std::ranges::find_if(
      kCultures, [lcid](const Identity &culture) { return culture.lcid == lcid; });
  if (found != kCultures.end()) { return CultureValue{*found}; }
  return CultureValue{Identity{.lcid = lcid, .name = "", .two = "Unknown", .three = "Unknown"}};
}

CultureValue CultureInfo::GetCultureInfo(std::string_view name) {
  const auto *const found = std::ranges::find_if(
      kCultures, [name](const Identity &culture) { return SameTag(culture.name, name); });
  if (found != kCultures.end()) { return CultureValue{*found}; }
  return CultureValue{Identity{.lcid = 0, .name = name, .two = "Unknown", .three = "Unknown"}};
}

CultureValue CultureInfo::Parent() const {
  const std::size_t dash = name_.find('-');
  if (dash == std::string::npos) { return *this; }
  const std::string_view two = std::string_view(name_).substr(0, dash);
  return CultureValue{Identity{.lcid = 0, .name = two, .two = two, .three = three_}};
}

}
