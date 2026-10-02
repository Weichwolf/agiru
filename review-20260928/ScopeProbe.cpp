#include "Apps.h"
#include "Scope.h"

#include <cstdio>

int main() {
  const agiru::gen::TranspileScope current{
      .include = {"System.Text"}, .exclude = {"System.Text"},
      .areaExclude = {}, .areaExcludeSuffix = {}};
  const bool included = agiru::gen::Holds(current, "System.Text");
  const auto other = agiru::gen::Scope::FromFile("build/review-20260928/scope.json");
  const bool otherIncluded = other.Contains("System.Text");
  std::printf("include/exclude tie: transpiler=%s alternate=%s; documented=exclude\n",
              included ? "include" : "exclude", otherIncluded ? "include" : "exclude");
  return included || otherIncluded ? 1 : 0;
}
