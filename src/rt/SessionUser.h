#pragma once

#include "platform/User.h"

namespace agiru {
class Connection;
class Guid;
}

namespace agiru::detail {

platform::User RequireActiveUser(const Connection &connection, const Guid &identity);

}
