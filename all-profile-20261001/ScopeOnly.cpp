#include "platform/PersonalizationScope.h"

static_assert(agiru::OptionTraits<agiru::platform::PersonalizationScope>::kValues.size() == 2);
static_assert(static_cast<int>(agiru::platform::PersonalizationScope::System) == 0);
static_assert(static_cast<int>(agiru::platform::PersonalizationScope::Tenant) == 1);
