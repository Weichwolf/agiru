#include "NativeMethods.h"

#include "Ast.h"
#include "EnumWriter.h"
#include "Names.h"

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace agiru::gen {
namespace {

constexpr int kBase64ConvertId = 2000000024;

bool Scalar(const al::VarDecl &declared, std::string_view type) {
  return LowerKey(declared.type) == LowerKey(std::string(type)) && !declared.byReference &&
         !declared.temporary && declared.subtype.empty() && declared.length == 0 &&
         declared.members.empty() && declared.arguments.empty() && declared.dimensions.empty();
}

bool Signature(const al::ProcedureDecl &procedure,
               std::span<const std::string_view> parameters,
               std::string_view returned) {
  if (procedure.parameters.size() != parameters.size() || !Scalar(procedure.returned, returned) ||
      LowerKey(procedure.returnType) != LowerKey(std::string(returned))) {
    return false;
  }
  for (std::size_t at = 0; at < parameters.size(); ++at) {
    if (!Scalar(procedure.parameters[at], parameters[at])) { return false; }
  }
  return true;
}

std::string Argument(const al::ProcedureDecl &procedure, std::size_t at) {
  return Identifier(procedure.parameters[at].name);
}

NativeMethod Call(const al::ProcedureDecl &procedure,
                  std::string_view name,
                  std::span<const std::size_t> arguments) {
  std::string body = procedure.returnType.empty() ? "  " : "  return ";
  body += "::agiru::";
  body += name;
  body += '(';
  bool first = true;
  for (const std::size_t at : arguments) {
    if (!first) { body += ", "; }
    first = false;
    body += Argument(procedure, at);
  }
  body += ");\n";
  return {.body = body, .header = "runtime/NativeBase64.h"};
}

}

std::optional<NativeMethod> BindNativeMethod(const al::CodeunitObject &unit,
                                             const al::ProcedureDecl &procedure) {
  if (!al::HasAttribute(procedure, "Native") || unit.id != kBase64ConvertId ||
      LowerKey(unit.nameSpace) != "system.runtime" || LowerKey(unit.name) != "base64convert" ||
      procedure.isLocal || procedure.isTrigger) {
    return std::nullopt;
  }
  constexpr std::array<std::string_view, 5> encode{
      "Text", "Boolean", "TextEncoding", "Integer", "OutStream"};
  constexpr std::array<std::string_view, 4> decode{"Text", "TextEncoding", "Integer", "OutStream"};
  constexpr std::array<std::string_view, 2> bytes{"Text", "OutStream"};
  constexpr std::array<std::size_t, 5> positions{0, 1, 2, 3, 4};
  constexpr std::array<std::size_t, 2> rawPositions{0, 3};
  const std::string name = LowerKey(procedure.name);
  if (name == "tobase64") {
    if (Signature(procedure, std::span{encode}.first(4), "Text")) {
      return Call(procedure, "NativeToBase64", std::span{positions}.first(4));
    }
    if (Signature(procedure, encode, "")) { return Call(procedure, "NativeToBase64", positions); }
  } else if (name == "frombase64") {
    if (Signature(procedure, std::span{decode}.first(3), "Text")) {
      return Call(procedure, "NativeFromBase64", std::span{positions}.first(3));
    }
    if (Signature(procedure, decode, "")) {
      return Call(procedure, "NativeFromBase64", rawPositions);
    }
    if (Signature(procedure, bytes, "")) {
      return Call(procedure, "NativeFromBase64", std::span{positions}.first(2));
    }
  }
  return std::nullopt;
}

}
