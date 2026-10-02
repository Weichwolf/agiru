#include "platform/ReflectionOptions.h"

#include <cstddef>

static_assert(static_cast<int>(agiru::platform::ObsoleteState::Removed) == 2);
static_assert(static_cast<int>(agiru::platform::FieldDataClassification::SystemMetadata) == 6);
static_assert(agiru::OptionTraits<agiru::platform::ObsoleteState>::kValues.size() == 3);
static_assert(agiru::OptionTraits<agiru::platform::FieldDataClassification>::kValues.size() == 7);
