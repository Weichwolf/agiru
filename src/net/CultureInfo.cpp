#include "dotnet/CultureInfo.h"

#include "type/Language.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

struct Culture {
  std::int32_t lcid;
  std::string_view name;
  std::string_view two;
  std::string_view three;
};

constexpr std::int32_t kInvariantLcid = 127;

constexpr std::array<Culture, 52> kCultures{{
    {kInvariantLcid, "", "iv", "IVL"},   {1025, "ar-SA", "ar", "ARA"},
    {1026, "bg-BG", "bg", "BGR"},        {1027, "ca-ES", "ca", "CAT"},
    {1028, "zh-TW", "zh", "CHT"},        {1029, "cs-CZ", "cs", "CSY"},
    {1030, "da-DK", "da", "DAN"},        {1031, "de-DE", "de", "DEU"},
    {1032, "el-GR", "el", "ELL"},        {1033, "en-US", "en", "ENU"},
    {1034, "es-ES_tradnl", "es", "ESP"}, {1035, "fi-FI", "fi", "FIN"},
    {1036, "fr-FR", "fr", "FRA"},        {1037, "he-IL", "he", "HEB"},
    {1038, "hu-HU", "hu", "HUN"},        {1039, "is-IS", "is", "ISL"},
    {1040, "it-IT", "it", "ITA"},        {1041, "ja-JP", "ja", "JPN"},
    {1042, "ko-KR", "ko", "KOR"},        {1043, "nl-NL", "nl", "NLD"},
    {1044, "nb-NO", "nb", "NOR"},        {1045, "pl-PL", "pl", "PLK"},
    {1046, "pt-BR", "pt", "PTB"},        {1048, "ro-RO", "ro", "ROM"},
    {1049, "ru-RU", "ru", "RUS"},        {1050, "hr-HR", "hr", "HRV"},
    {1051, "sk-SK", "sk", "SKY"},        {1053, "sv-SE", "sv", "SVE"},
    {1054, "th-TH", "th", "THA"},        {1055, "tr-TR", "tr", "TRK"},
    {1057, "id-ID", "id", "IND"},        {1058, "uk-UA", "uk", "UKR"},
    {1060, "sl-SI", "sl", "SLV"},        {1061, "et-EE", "et", "ETI"},
    {1062, "lv-LV", "lv", "LVI"},        {1063, "lt-LT", "lt", "LTH"},
    {1066, "vi-VN", "vi", "VIT"},        {1069, "eu-ES", "eu", "EUQ"},
    {1086, "ms-MY", "ms", "MSL"},        {2052, "zh-CN", "zh", "CHS"},
    {2055, "de-CH", "de", "DES"},        {2057, "en-GB", "en", "ENG"},
    {2058, "es-MX", "es", "ESM"},        {2060, "fr-BE", "fr", "FRB"},
    {2064, "it-CH", "it", "ITS"},        {2067, "nl-BE", "nl", "NLB"},
    {2068, "nn-NO", "nn", "NON"},        {2070, "pt-PT", "pt", "PTG"},
    {3079, "de-AT", "de", "DEA"},        {3081, "en-AU", "en", "ENA"},
    {3082, "es-ES", "es", "ESN"},        {3084, "fr-CA", "fr", "FRC"},
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

class CultureInfo Made(const Culture &culture) {
  class CultureInfo made;
  return made.Fill_(culture.lcid, culture.name, culture.two, culture.three);
}

}

class CultureInfo &CultureInfo::Fill_(std::int32_t lcid,
                                      std::string_view name,
                                      std::string_view two,
                                      std::string_view three) {
  lcid_ = lcid;
  name_ = std::string(name);
  two_ = std::string(two);
  three_ = std::string(three);
  return *this;
}

class CultureInfo CultureInfo::Binder::operator()(Integer lcid) const {
  return GetCultureInfo(lcid);
}

class CultureInfo CultureInfo::Binder::operator()(std::string_view name) const {
  return GetCultureInfo(name);
}

class CultureInfo CultureInfo::InvariantCulture() {
  return Made(kCultures.front());
}

class CultureInfo CultureInfo::CurrentCulture() {
  return GetCultureInfo(Language::Current());
}

class CultureInfo CultureInfo::GetCultureInfo(Integer lcid) {
  const auto found = std::ranges::find_if(
      kCultures, [lcid](const Culture &culture) { return culture.lcid == lcid; });
  if (found != kCultures.end()) { return Made(*found); }
  class CultureInfo unknown;
  return unknown.Fill_(lcid, "", "Unknown", "Unknown");
}

class CultureInfo CultureInfo::GetCultureInfo(std::string_view name) {
  const auto found = std::ranges::find_if(
      kCultures, [name](const Culture &culture) { return SameTag(culture.name, name); });
  if (found != kCultures.end()) { return Made(*found); }
  class CultureInfo unknown;
  return unknown.Fill_(0, name, "Unknown", "Unknown");
}

class CultureInfo CultureInfo::Parent() const {
  const std::size_t dash = name_.find('-');
  if (dash == std::string::npos) { return *this; }
  const std::string_view two = std::string_view(name_).substr(0, dash);
  class CultureInfo neutral;
  return neutral.Fill_(0, two, two, three_);
}

}
