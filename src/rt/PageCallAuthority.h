#pragma once

#include "runtime/Database.h"

#include "CommandAuthority.h"

#include <memory>
#include <string>

namespace agiru {
class Session;
struct PageHostOptions;
}

namespace agiru::detail {

struct PageCall;
struct SessionState;

class PageCallAuthority final : public CommandAuthority {
public:
  PageCallAuthority(Session &session,
                    std::shared_ptr<PageCall> call,
                    const PageHostOptions &options);
  ~PageCallAuthority() override;
  PageCallAuthority(const PageCallAuthority &) = delete;
  PageCallAuthority &operator=(const PageCallAuthority &) = delete;
  void Check() override;
  void LockCommit(const Connection &connection) override;

private:
  bool Live(const Connection &connection, bool lock) const;
  void Cancel();
  SessionState &state_;
  std::shared_ptr<PageCall> call_;
  Connection authority_;
  std::string company_;
};

}
