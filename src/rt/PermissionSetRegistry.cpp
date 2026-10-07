#include "runtime/PermissionSetRegistry.h"

#include "meta/PermissionSetDef.h"
#include "runtime/ErrorValue.h"
#include "type/Guid.h"

#include <map>
#include <mutex>
#include <span>
#include <vector>

namespace agiru {
namespace {

class Registry {
public:
  void Add(const PermissionSetDef *declaration) {
    const std::scoped_lock lock(mutex_);
    if (frozen_) { throw Error("installed permission metadata is frozen", "PermissionRegistry"); }
    if (declaration == nullptr || declaration->identity.scope != PermissionScope::System ||
        declaration->identity.role.empty() || declaration->identity.app.empty() ||
        Guid(declaration->identity.app).ToStorageText() != declaration->identity.app) {
      throw Error("invalid installed system permission identity", "PermissionRegistry");
    }
    if (!definitions_.emplace(declaration->identity, declaration).second) {
      throw Error("duplicate installed system permission identity", "PermissionRegistry");
    }
  }

  std::span<const PermissionSetDef *const> Read() {
    std::call_once(once_, [this] {
      const std::scoped_lock lock(mutex_);
      declarations_.reserve(definitions_.size());
      for (const auto &[identity, declaration] : definitions_) {
        static_cast<void>(identity);
        declarations_.push_back(declaration);
      }
      frozen_ = true;
    });
    return declarations_;
  }

  const PermissionSetDef *Find(const PermissionSetIdentity &identity) {
    static_cast<void>(Read());
    const auto found = definitions_.find(identity);
    return found == definitions_.end() ? nullptr : found->second;
  }

private:
  std::mutex mutex_;
  std::once_flag once_;
  std::map<PermissionSetIdentity, const PermissionSetDef *> definitions_;
  std::vector<const PermissionSetDef *> declarations_;
  bool frozen_ = false;
};

Registry &Installed() {
  static Registry registry;
  return registry;
}

}

void RegisterPermissionSet(const PermissionSetDef *declaration) {
  Installed().Add(declaration);
}

std::span<const PermissionSetDef *const> InstalledPermissionSetDefinitions() {
  return Installed().Read();
}

const PermissionSetDef *InstalledPermissionSets::Find(const PermissionSetIdentity &identity) const {
  return Installed().Find(identity);
}

}
