#pragma once

#include "Ast.h"

#include <optional>
#include <string>

namespace agiru::gen {

struct NativeMethod {
  std::string body;
  std::string header;
};

[[nodiscard]] std::optional<NativeMethod> BindNativeMethod(const al::CodeunitObject &unit,
                                                           const al::ProcedureDecl &procedure);

}
