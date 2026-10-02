#include "fixture/codeunit/Caller.h"

#include <exception>
#include <cstdio>

int main() {
  try {
    agiru::Fixture::Caller_Codeunit caller;
    return caller.Run() ? 0 : 1;
  } catch (const std::exception &error) {
    std::fputs(error.what(), stderr);
    return 1;
  }
}
