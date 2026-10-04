#include "type/AlDecimalArithmetic.h"
#include "type/Decimal.h"

#include "Check.h"

#include <array>
#include <compare>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
using agiru::AlDecimalArithmetic;
using agiru::Decimal;
using agiru::DecimalError;

std::string Representation(const Decimal &value) {
  return value.ToInvariantString() + "|" + std::to_string(value.Scale());
}

std::string Result(std::string_view operation, std::string_view lhs, std::string_view rhs) {
  const Decimal left = Decimal::FromInvariantString(lhs);
  const Decimal right = Decimal::FromInvariantString(rhs);
  if (operation == "N") { return Representation(AlDecimalArithmetic::Normalize(left)); }
  if (operation == "C") {
    const auto order = AlDecimalArithmetic::Compare(left, right);
    return order < 0 ? "-1" : order > 0 ? "1" : "0";
  }
  try {
    if (operation == "+") { return Representation(AlDecimalArithmetic::Add(left, right)); }
    if (operation == "-") { return Representation(AlDecimalArithmetic::Subtract(left, right)); }
    if (operation == "*") { return Representation(AlDecimalArithmetic::Multiply(left, right)); }
    if (operation == "/") { return Representation(AlDecimalArithmetic::Divide(left, right)); }
    if (operation == "%") { return Representation(AlDecimalArithmetic::Remainder(left, right)); }
  } catch (const DecimalError &error) {
    if ((operation == "/" || operation == "%") && right.IsZero()) { return "DIVIDE_BY_ZERO"; }
    if (std::string_view{error.what()}.starts_with("Decimal: overflow")) { return "OVERFLOW"; }
    throw;
  }
  throw std::invalid_argument("unknown decimal reference operation");
}

void Normalization() {
  constexpr std::array cases{
      std::array<std::string_view, 2>{"0", "0|0"},
      std::array<std::string_view, 2>{"0.00", "0.00|2"},
      std::array<std::string_view, 2>{"0.0000000000000000000000000000",
                                      "0.0000000000000000000000000000|28"},
      std::array<std::string_view, 2>{"0.100000000000000000", "0.100000000000000000|18"},
      std::array<std::string_view, 2>{"1.00000000000000000", "1.00000000000000000|17"},
      std::array<std::string_view, 2>{"1.000000000000000000", "1|0"},
      std::array<std::string_view, 2>{"1.0000000000000000000", "1|0"},
      std::array<std::string_view, 2>{"999999999999999999", "999999999999999999|0"},
      std::array<std::string_view, 2>{"1000000000000000000", "1000000000000000000|0"},
      std::array<std::string_view, 2>{"0.1234567890123456785", "0.123456789012345679|18"},
      std::array<std::string_view, 2>{"-0.1234567890123456785", "-0.123456789012345679|18"},
      std::array<std::string_view, 2>{"0.9999999999999999999", "1|0"},
      std::array<std::string_view, 2>{"0.0000000000000000000000000001",
                                      "0.0000000000000000000000000001|28"},
      std::array<std::string_view, 2>{"0.1111111111111111111111111111", "0.111111111111111111|18"},
      std::array<std::string_view, 2>{"79228162514264337593543950335",
                                      "79228162514264337500000000000|0"},
      std::array<std::string_view, 2>{"-79228162514264337593543950335",
                                      "-79228162514264337500000000000|0"},
      std::array<std::string_view, 2>{"9223372036854775807", "9223372036854775810|0"},
      std::array<std::string_view, 2>{"-9223372036854775808", "-9223372036854775810|0"}};
  for (const auto &[input, expected] : cases) {
    CHECK_TEXT("native significance and scale", Result("N", input, "0"), expected);
  }
}

void Arithmetic() {
  CHECK_TEXT(
      "AL reciprocal is significance-limited", Result("/", "1", "9"), "0.111111111111111111|18");
  CHECK_TEXT("AL product has exact reciprocal divisibility",
             Result("*", "0.111111111111111111", "100"),
             "11.1111111111111111|16");
  CHECK_TEXT("AL remainder normalizes both operands",
             Result("%", "11.1111111111111111", "0.1111111111111111111111111111"),
             "0.000000000000000000|18");
  CHECK_TEXT("addition normalizes its operands before CLR arithmetic",
             Result("+", "0.1234567890123456785", "-0.123456789012345679"),
             "0.000000000000000000|18");
  CHECK_TEXT("subtraction normalizes its operands before CLR arithmetic",
             Result("-", "0.1234567890123456785", "0.123456789012345679"),
             "0.000000000000000000|18");
  CHECK_TEXT("comparison uses AL normalized values",
             Result("C", "0.1234567890123456785", "0.123456789012345679"),
             "0");
  CHECK_TEXT("comparison retains negative order", Result("C", "-1", "1"), "-1");
  CHECK_TEXT("comparison retains positive order", Result("C", "1", "-1"), "1");
  CHECK_TEXT("tiny AL operands retain scale 28",
             Result("+", "0.0000000000000000000000000001", "0.0000000000000000000000000001"),
             "0.0000000000000000000000000002|28");
  CHECK_TEXT("addition still reports CLR overflow",
             Result("+", "79228162514264337593543950335", "79228162514264337593543950335"),
             "OVERFLOW");
  CHECK_TEXT("multiplication still reports CLR overflow",
             Result("*", "79228162514264337593543950335", "2"),
             "OVERFLOW");
  CHECK_TEXT("division reports zero divisor", Result("/", "1", "0"), "DIVIDE_BY_ZERO");
  CHECK_TEXT("remainder reports zero divisor", Result("%", "1", "0"), "DIVIDE_BY_ZERO");
  const Decimal raw = Decimal::FromInvariantString("0.1111111111111111111111111111");
  const Decimal unused = AlDecimalArithmetic::Multiply(raw, Decimal(100));
  CHECK_TRUE("owned arithmetic leaves caller values unchanged", !unused.IsZero());
  CHECK_TEXT("CLR caller retains every original digit",
             Representation(raw),
             "0.1111111111111111111111111111|28");
  CHECK_TEXT("ordinary CLR division is not changed",
             Representation(Decimal(1) / Decimal(9)),
             "0.1111111111111111111111111111|28");
}

std::array<std::string_view, 4> Fields(std::string_view line) {
  if (!line.ends_with('\n')) { throw std::invalid_argument("unterminated reference line"); }
  line.remove_suffix(1);
  std::array<std::string_view, 4> fields{};
  for (std::size_t field = 0; field < fields.size() - 1; ++field) {
    const auto tab = line.find('\t');
    if (tab == std::string_view::npos || tab == 0) {
      throw std::invalid_argument("missing decimal reference column");
    }
    fields[field] = line.substr(0, tab);
    line.remove_prefix(tab + 1);
  }
  if (line.empty() || line.find('\t') != std::string_view::npos) {
    throw std::invalid_argument("invalid decimal reference result column");
  }
  fields.back() = line;
  return fields;
}

int Reference() {
  constexpr std::size_t kLineCapacity = 384;
  std::array<char, kLineCapacity> buffer{};
  std::size_t lines = 0;
  while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), stdin) != nullptr) {
    const auto fields = Fields(std::string_view{buffer.data()});
    const std::string result = Result(fields[0], fields[1], fields[2]);
    const std::string output = std::string(fields[0]) + '\t' + std::string(fields[1]) + '\t' +
                               std::string(fields[2]) + '\t' + result + '\n';
    if (std::fwrite(output.data(), 1, output.size(), stdout) != output.size()) {
      throw std::runtime_error("decimal reference output failed");
    }
    ++lines;
  }
  if (lines == 0) { throw std::invalid_argument("empty decimal reference population"); }
  return std::ferror(stdin) == 0 && std::fflush(stdout) == 0 ? 0 : 2;
}
}

int main(int argc, char **argv) {
  try {
    if (argc == 2 && std::string_view{argv[1]} == "--reference") { return Reference(); }
    if (argc != 1) { throw std::invalid_argument("unexpected AlDecimalGate arguments"); }
    return gate::Run("AlDecimalGate", [] {
      Normalization();
      Arithmetic();
    });
  } catch (const std::exception &error) {
    std::fputs("AlDecimalGate refuses: ", stderr);
    std::fputs(error.what(), stderr);
    std::fputc('\n', stderr);
    return 2;
  }
}
