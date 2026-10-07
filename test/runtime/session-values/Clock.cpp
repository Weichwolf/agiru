#include "type/DateTime.h"

namespace agiru {

DateTime CurrentDateTime() {
  constexpr auto kFixtureMilliseconds = 12345;
  return DateTime::FromMilliseconds(kFixtureMilliseconds);
}

}
