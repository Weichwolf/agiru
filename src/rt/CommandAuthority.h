#pragma once

namespace agiru {
class Connection;
}

namespace agiru::detail {

class CommandAuthority {
public:
  virtual ~CommandAuthority() = default;
  virtual void Check() = 0;
  virtual void LockCommit(const Connection &connection) = 0;
};

void CheckCommandAuthority();
void LockCommandCommit(const Connection &connection);

}
