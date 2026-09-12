#include "type/Language.h"

#include "type/Integer.h"

namespace agiru {

namespace {

thread_local Integer g_language = Language::kEnglishUnitedStates;

}

Integer Language::Current() {
  return g_language;
}

void Language::MakeCurrent(Integer id) {
  g_language = id == 0 ? kEnglishUnitedStates : id;
}

}
