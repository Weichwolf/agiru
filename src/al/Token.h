#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::al {

enum class TokenKind : std::uint8_t {
  EndOfFile,
  Identifier,
  QuotedIdentifier,
  String,
  Integer,
  Decimal,
  DateTime,
  Punctuation,
  Directive,
};

struct Token {
  TokenKind kind = TokenKind::EndOfFile;
  std::string text;
  std::size_t line = 0;
  std::size_t column = 0;
};

bool IsKeyword(const Token &token, std::string_view keyword);

bool IsPunctuation(const Token &token, std::string_view punctuation);

}
