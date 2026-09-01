#include "Lexer.h"

#include "Token.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::al {

namespace {

bool IsIdentifierStart(char c) {
  return (std::isalpha(static_cast<unsigned char>(c)) != 0) || c == '_';
}

bool IsIdentifierPart(char c) {
  return (std::isalnum(static_cast<unsigned char>(c)) != 0) || c == '_';
}

bool IsDigit(char c) {
  return std::isdigit(static_cast<unsigned char>(c)) != 0;
}

char Lower(char c) {
  return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

constexpr std::array kTwoCharacterPunctuation{std::string_view{"::"},
                                              std::string_view{":="},
                                              std::string_view{"<>"},
                                              std::string_view{"<="},
                                              std::string_view{">="},
                                              std::string_view{".."},
                                              std::string_view{"+="},
                                              std::string_view{"-="},
                                              std::string_view{"*="},
                                              std::string_view{"/="}};

class Scanner {
public:
  explicit Scanner(std::string_view source) : source_(source) {
    if (source_.starts_with("\xEF\xBB\xBF")) { position_ = 3; }
  }

  std::vector<Token> Run() {
    std::vector<Token> tokens;
    while (true) {
      SkipTrivia();
      if (position_ >= source_.size()) { break; }
      tokens.push_back(Next());
    }
    tokens.push_back(Token{.kind = TokenKind::EndOfFile, .text = {}, .line = line_, .column = 1});
    return tokens;
  }

private:
  [[nodiscard]] bool Starts(std::string_view text) const {
    return source_.compare(position_, text.size(), text) == 0;
  }

  void TakeLine() {
    while (position_ < source_.size() && source_[position_] != '\n') { ++position_; }
  }

  void TakeNewline() {
    ++line_;
    lineStart_ = position_ + 1;
    ++position_;
  }

  void TakeBlockComment() {
    position_ += 2;
    while (position_ + 1 < source_.size() && !Starts("*/")) {
      if (source_[position_] == '\n') {
        TakeNewline();
      } else {
        ++position_;
      }
    }
    position_ = position_ + 1 < source_.size() ? position_ + 2 : source_.size();
  }

  bool SkipOnce() {
    const char c = source_[position_];
    if (c == '\n') {
      TakeNewline();
      return true;
    }
    if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      ++position_;
      return true;
    }
    if (Starts("//")) {
      TakeLine();
      return true;
    }
    if (Starts("/*")) {
      TakeBlockComment();
      return true;
    }
    return false;
  }

  void SkipTrivia() {
    while (position_ < source_.size() && SkipOnce()) {}
  }

  [[nodiscard]] Token Make(TokenKind kind, std::string text, std::size_t start) const {
    return Token{
        .kind = kind, .text = std::move(text), .line = line_, .column = start - lineStart_ + 1};
  }

  Token LexIdentifier(std::size_t start) {
    while (position_ < source_.size() && IsIdentifierPart(source_[position_])) { ++position_; }
    return Make(
        TokenKind::Identifier, std::string(source_.substr(start, position_ - start)), start);
  }

  Token LexQuotedIdentifier(std::size_t start) {
    ++position_;
    std::string value;
    while (position_ < source_.size()) {
      if (source_[position_] == '"') {
        if (position_ + 1 < source_.size() && source_[position_ + 1] == '"') {
          value += '"';
          position_ += 2;
          continue;
        }
        break;
      }
      if (source_[position_] == '\n') {
        TakeNewline();
      } else {
        value += source_[position_];
        ++position_;
      }
    }
    if (position_ >= source_.size()) { throw LexError("unterminated quoted identifier"); }
    ++position_;
    return Make(TokenKind::QuotedIdentifier, std::move(value), start);
  }

  Token LexString(std::size_t start) {
    ++position_;
    std::string value;
    while (position_ < source_.size()) {
      if (source_[position_] == '\'') {
        if (position_ + 1 < source_.size() && source_[position_ + 1] == '\'') {
          value += '\'';
          position_ += 2;
          continue;
        }
        break;
      }
      if (source_[position_] == '\n') {
        TakeNewline();
      } else {
        value += source_[position_];
        ++position_;
      }
    }
    if (position_ >= source_.size()) { throw LexError("unterminated string"); }
    ++position_;
    return Make(TokenKind::String, std::move(value), start);
  }

  [[nodiscard]] bool SuffixEnds(std::size_t after) const {
    return after >= source_.size() || !IsIdentifierPart(source_[after]);
  }

  bool TakeSuffix(std::string_view letters) {
    std::size_t after = position_;
    while (after < source_.size() &&
           letters.find(Lower(source_[after])) != std::string_view::npos) {
      ++after;
    }
    if (after == position_ || !SuffixEnds(after)) { return false; }
    position_ = after;
    return true;
  }

  Token LexNumber(std::size_t start) {
    bool fractional = false;
    while (position_ < source_.size()) {
      const char d = source_[position_];
      if (IsDigit(d)) {
        ++position_;
      } else if (d == '.' && !fractional && position_ + 1 < source_.size() &&
                 IsDigit(source_[position_ + 1])) {
        fractional = true;
        ++position_;
      } else {
        break;
      }
    }
    if (TakeSuffix("dt")) {
      return Make(
          TokenKind::DateTime, std::string(source_.substr(start, position_ - start)), start);
    }
    if (TakeSuffix("l")) {
      return Make(
          TokenKind::Integer, std::string(source_.substr(start, position_ - start - 1)), start);
    }
    return Make(fractional ? TokenKind::Decimal : TokenKind::Integer,
                std::string(source_.substr(start, position_ - start)),
                start);
  }

  Token LexDirective(std::size_t start) {
    ++position_;
    std::string word;
    while (position_ < source_.size() && IsIdentifierPart(source_[position_])) {
      word += Lower(source_[position_]);
      ++position_;
    }
    const std::size_t conditionStart = position_;
    TakeLine();
    if (word != "if" && word != "else" && word != "endif") {
      return Make(TokenKind::Directive, "", start);
    }
    std::string text = word;
    if (word == "if") {
      text += " ";
      text += source_.substr(conditionStart, position_ - conditionStart);
    }
    return Make(TokenKind::Directive, std::move(text), start);
  }

  Token LexPunctuation(std::size_t start) {
    for (const std::string_view two : kTwoCharacterPunctuation) {
      if (Starts(two)) {
        position_ += 2;
        return Make(TokenKind::Punctuation, std::string(two), start);
      }
    }
    const char c = source_[position_];
    ++position_;
    return Make(TokenKind::Punctuation, std::string(1, c), start);
  }

  Token Next() {
    const std::size_t start = position_;
    const char c = source_[position_];
    if (c == '#') { return LexDirective(start); }
    if (IsIdentifierStart(c)) { return LexIdentifier(start); }
    if (c == '"') { return LexQuotedIdentifier(start); }
    if (c == '\'') { return LexString(start); }
    if (IsDigit(c)) { return LexNumber(start); }
    return LexPunctuation(start);
  }

  std::string_view source_;
  std::size_t position_ = 0;
  std::size_t line_ = 1;
  std::size_t lineStart_ = 0;
};

class ConditionReader {
public:
  explicit ConditionReader(std::string_view text) : text_(text) {}

  bool Evaluate() { return ReadOr(); }

private:
  void SkipSpace() {
    while (position_ < text_.size() &&
           (std::isspace(static_cast<unsigned char>(text_[position_])) != 0)) {
      ++position_;
    }
  }

  std::string ReadWord() {
    SkipSpace();
    std::string word;
    while (position_ < text_.size() && IsIdentifierPart(text_[position_])) {
      word += Lower(text_[position_]);
      ++position_;
    }
    return word;
  }

  bool AtCharacter(char c) {
    SkipSpace();
    return position_ < text_.size() && text_[position_] == c;
  }

  bool ReadPrimary() {
    if (++depth_ > kMaxDepth) { throw LexError("a #if condition nests too deeply"); }
    SkipSpace();
    if (AtCharacter('(')) {
      ++position_;
      const bool inner = ReadOr();
      if (AtCharacter(')')) { ++position_; }
      --depth_;
      return inner;
    }
    const std::size_t mark = position_;
    const std::string word = ReadWord();
    --depth_;
    if (word == "not") { return !ReadPrimary(); }
    if (word == "true") { return true; }
    if (word.empty()) { position_ = mark + 1; }
    return false;
  }

  bool ReadAnd() {
    bool value = ReadPrimary();
    while (true) {
      const std::size_t mark = position_;
      if (ReadWord() != "and") {
        position_ = mark;
        return value;
      }
      value = ReadPrimary() && value;
    }
  }

  bool ReadOr() {
    bool value = ReadAnd();
    while (true) {
      const std::size_t mark = position_;
      if (ReadWord() != "or") {
        position_ = mark;
        return value;
      }
      value = ReadAnd() || value;
    }
  }

  static constexpr int kMaxDepth = 64;

  std::string_view text_;
  std::size_t position_ = 0;
  int depth_ = 0;
};

std::vector<Token> ApplyDirectives(std::vector<Token> tokens) {
  std::vector<Token> live;
  std::vector<bool> keeping;
  for (Token &token : tokens) {
    if (token.kind == TokenKind::Directive) {
      if (token.text.starts_with("if")) {
        keeping.push_back(ConditionReader(std::string_view(token.text).substr(2)).Evaluate());
      } else if (token.text == "else" && !keeping.empty()) {
        keeping.back() = !keeping.back();
      } else if (token.text == "endif" && !keeping.empty()) {
        keeping.pop_back();
      }
      continue;
    }
    bool inside = true;
    for (const bool k : keeping) { inside = inside && k; }
    if (inside) { live.push_back(std::move(token)); }
  }
  return live;
}

} // namespace

bool IsKeyword(const Token &token, std::string_view keyword) {
  if (token.kind != TokenKind::Identifier || token.text.size() != keyword.size()) { return false; }
  for (std::size_t i = 0; i < keyword.size(); ++i) {
    if (Lower(token.text[i]) != Lower(keyword[i])) { return false; }
  }
  return true;
}

bool IsPunctuation(const Token &token, std::string_view punctuation) {
  return token.kind == TokenKind::Punctuation && token.text == punctuation;
}

std::vector<Token> Tokenize(std::string_view source) {
  return ApplyDirectives(Scanner(source).Run());
}

} // namespace agiru::al
