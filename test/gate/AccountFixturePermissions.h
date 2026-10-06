#pragma once

#include "platform/User.h"
#include "runtime/Session.h"
#include "runtime/TablePermissions.h"

#include <memory>

namespace gate {

class AccountFixtureAuthority final : public agiru::TablePermissionAuthority {
public:
  bool Allows(const agiru::TableDef &table,
              [[maybe_unused]] agiru::TableOperation operation) const override {
    return table.id == agiru::platform::kUserTable.id;
  }
};

inline void AccountFixturePermissions(agiru::Session &session) {
  session.TablePermissions(std::make_shared<AccountFixtureAuthority>());
}

}
