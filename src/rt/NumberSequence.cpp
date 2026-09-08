#include "type/NumberSequence.h"

#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

constexpr std::string_view kPrefix = "NumSeq$";

std::string Quoted(std::string_view name) {
  std::string out = "\"";
  for (const char c : name) {
    if (c == '"') { out += '"'; }
    out += c;
  }
  out += '"';
  return out;
}

std::string SequenceName(std::string_view name, Boolean companySpecific) {
  std::string built(kPrefix);
  if (companySpecific) {
    built += Session::Current().CompanyName();
    built += '$';
  }
  built += name;
  return built;
}

std::string Named(std::string_view name, Boolean companySpecific) {
  return Quoted(SequenceName(name, companySpecific));
}

bool Declared(std::string_view name, Boolean companySpecific) {
  const std::string sequence = SequenceName(name, companySpecific);
  const Result found = Session::Current().Database().Execute(
      "SELECT 1 FROM pg_class WHERE relkind = 'S' AND relname = $1",
      std::vector<std::optional<std::string>>{sequence});
  return found.Rows() != 0;
}

[[noreturn]] void Absent(std::string_view name) {
  throw Error("the number sequence " + std::string(name) + " does not exist");
}

BigInteger Read(std::string_view sql, std::string_view name, Boolean companySpecific) {
  if (!Declared(name, companySpecific)) { Absent(name); }
  const Result row = Session::Current().Database().Execute(std::string(sql));
  const std::optional<std::string_view> value = row.Value(0, 0);
  if (!value.has_value()) { Absent(name); }
  return std::stoll(std::string(*value));
}

}

BigInteger NumberSequence::Current(std::string_view Name, Boolean CompanySpecific) {
  return Read("SELECT last_value FROM " + Named(Name, CompanySpecific), Name, CompanySpecific);
}

void NumberSequence::Delete(std::string_view Name, Boolean CompanySpecific) {
  if (!Declared(Name, CompanySpecific)) { Absent(Name); }
  Session::Current().Database().Run("DROP SEQUENCE " + Named(Name, CompanySpecific));
}

Boolean NumberSequence::Exists(std::string_view Name, Boolean CompanySpecific) {
  return Declared(Name, CompanySpecific);
}

void NumberSequence::Insert(std::string_view Name,
                            BigInteger Seed,
                            BigInteger Increment,
                            Boolean CompanySpecific) {
  if (Declared(Name, CompanySpecific)) {
    throw Error("the number sequence " + std::string(Name) + " already exists");
  }
  const BigInteger step = Increment == 0 ? 1 : Increment;
  Session::Current().Database().Run("CREATE SEQUENCE " + Named(Name, CompanySpecific) +
                                    " INCREMENT BY " + std::to_string(step) + " START WITH " +
                                    std::to_string(Seed) + " MINVALUE " + std::to_string(Seed));
}

BigInteger NumberSequence::Next(std::string_view Name, Boolean CompanySpecific) {
  return Read(
      "SELECT nextval('" + SequenceName(Name, CompanySpecific) + "')", Name, CompanySpecific);
}

BigInteger NumberSequence::Range(std::string_view Name, Integer Count, BigInteger &Increment) {
  return Range(Name, Count, Increment, true);
}

BigInteger NumberSequence::Range(std::string_view Name,
                                 Integer Count,
                                 BigInteger &Increment,
                                 Boolean CompanySpecific) {
  if (!Declared(Name, CompanySpecific)) { Absent(Name); }
  const Result step = Session::Current().Database().Execute(
      "SELECT increment_by FROM pg_sequences WHERE sequencename = $1",
      std::vector<std::optional<std::string>>{SequenceName(Name, CompanySpecific)});
  const std::optional<std::string_view> by = step.Rows() == 0 ? std::nullopt : step.Value(0, 0);
  Increment = by.has_value() ? std::stoll(std::string(*by)) : 1;
  return Range(Name, Count, CompanySpecific);
}

BigInteger NumberSequence::Range(std::string_view Name, Integer Count, Boolean CompanySpecific) {
  if (Count < 1) {
    throw Error("NumberSequence.Range: a range of " + std::to_string(Count) + " values is none");
  }
  const BigInteger first = Next(Name, CompanySpecific);
  for (Integer taken = 1; taken < Count; ++taken) {
    static_cast<void>(Next(Name, CompanySpecific));
  }
  return first;
}

void NumberSequence::Restart(std::string_view Name, BigInteger Seed, Boolean CompanySpecific) {
  if (!Declared(Name, CompanySpecific)) { Absent(Name); }
  Session::Current().Database().Run("ALTER SEQUENCE " + Named(Name, CompanySpecific) +
                                    " RESTART WITH " + std::to_string(Seed));
}

}
