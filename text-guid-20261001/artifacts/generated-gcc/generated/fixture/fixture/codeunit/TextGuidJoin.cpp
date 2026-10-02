// Generated from Join.Codeunit.al. Do not edit.

#include "TextGuidJoin.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Code.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/SecretText.h"
#include "type/Text.h"


namespace agiru::Fixture {

namespace {
namespace TextGuidJoin_unit {
const RegisterCodeunit<TextGuidJoin_Codeunit> kInCodeunitCatalogue;
} // namespace TextGuidJoin_unit
} // namespace

::agiru::Integer TextGuidJoin_Codeunit::Exercise() {
  [[maybe_unused]] Guid Identity{};
  [[maybe_unused]] Guid EmptyIdentity{};
  [[maybe_unused]] ::agiru::Text<2> Prefix{};
  [[maybe_unused]] ::agiru::Text<0> Suffix{};
  [[maybe_unused]] ::agiru::Text<3> Unicode{};
  [[maybe_unused]] ::agiru::Code<2> UnitCode{};
  [[maybe_unused]] SecretText Secret{};

  Identity = "aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb";
  Prefix = " x";
  Suffix = "y ";
  Unicode = "ä💡";
  UnitCode = "ab";
  if (Prefix + Identity != " x{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}") {
    ::agiru::RaiseOrCollect("Bounded prefix");
  }
  if (Identity + Prefix != "{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB} x") {
    ::agiru::RaiseOrCollect("Bounded suffix");
  }
  if (Suffix + Identity != "y {AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}") {
    ::agiru::RaiseOrCollect("Unbounded prefix");
  }
  if (Identity + Suffix != "{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}y ") {
    ::agiru::RaiseOrCollect("Unbounded suffix");
  }
  if (Prefix + Identity + Suffix + UnitCode != " x{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}y AB") {
    ::agiru::RaiseOrCollect("Text Guid Code chain");
  }
  if (::agiru::Text<0>("a") + Identity + ::agiru::Text<0>("b") != "a{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}b") {
    ::agiru::RaiseOrCollect("Literal Guid chain");
  }
  if (Suffix + EmptyIdentity != "y {00000000-0000-0000-0000-000000000000}") {
    ::agiru::RaiseOrCollect("Null Guid");
  }
  if (StrLen(Prefix + Identity) != 40) {
    ::agiru::RaiseOrCollect("No input bound on result");
  }
  if (StrLen(Unicode + Identity) != 41) {
    ::agiru::RaiseOrCollect("UTF-16 length");
  }
  if (Unicode + Identity != "ä💡{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}") {
    ::agiru::RaiseOrCollect("Unicode");
  }
  if (Which(Prefix + Identity) != 1) {
    ::agiru::RaiseOrCollect("Bounded Text Guid overload");
  }
  if (Which(Identity + Prefix) != 1) {
    ::agiru::RaiseOrCollect("Guid bounded Text overload");
  }
  if (Which(Suffix + Identity) != 1) {
    ::agiru::RaiseOrCollect("Unbounded Text Guid overload");
  }
  if (Which(Identity + Suffix) != 1) {
    ::agiru::RaiseOrCollect("Guid unbounded Text overload");
  }
  if (Which(Identity) != 2) {
    ::agiru::RaiseOrCollect("Guid remains Guid");
  }
  if (Which(Secret) != 3) {
    ::agiru::RaiseOrCollect("Secret remains Secret");
  }
  return 16;
}

::agiru::Text<3> TextGuidJoin_Codeunit::Limited() {
  [[maybe_unused]] Guid Identity{};
  [[maybe_unused]] ::agiru::Text<2> Prefix{};
  [[maybe_unused]] ::agiru::Text<3> Destination{};

  Prefix = " x";
  Destination = Prefix + Identity;
  return Destination;
}

::agiru::Integer TextGuidJoin_Codeunit::Which([[maybe_unused]] ::agiru::Text<0> Value) {
  return 1;
}

::agiru::Integer TextGuidJoin_Codeunit::Which([[maybe_unused]] Guid Value) {
  return 2;
}

::agiru::Integer TextGuidJoin_Codeunit::Which([[maybe_unused]] SecretText Value) {
  return 3;
}

void TextGuidJoin_Codeunit::ClearAll() {
}

constexpr CodeunitDef kTextGuidJoinCodeunit{
    .id = ::agiru::CodeunitTraits<TextGuidJoin_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<TextGuidJoin_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<TextGuidJoin_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
