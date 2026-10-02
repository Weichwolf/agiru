#pragma once

#include "type/Variant.h"

/// \file
/// \brief AL `Any` -- the type a generic parameter takes.

namespace agiru {

/// \brief AL `Any`.
///
/// From `any-data-type.md`: the type used where a method takes a value of any type at all. It is
/// what a `Variant` already is -- one value carrying its own type -- so it is that type under the
/// name AL uses for it, rather than a second one with the same job.
using Any = Variant;

}
