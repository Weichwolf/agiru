#include "RecordChanges.h"

#include "meta/Ids.h"
#include "runtime/Session.h"

#include "SessionState.h"

#include <memory>

namespace agiru::detail {

RecordRead::RecordRead(TableId table) {
  SessionState &state = SessionState::Current();
  if (state.recordChanges == nullptr) { state.recordChanges = std::make_shared<RecordChanges>(); }
  owner_ = state.recordChanges;
  revision_ = owner_->tables_.try_emplace(table).first;
  ++revision_->second.readers;
  observed_ = revision_->second.value;
}

RecordRead::~RecordRead() {
  if (--revision_->second.readers == 0) { owner_->tables_.erase(revision_); }
}

bool RecordRead::Current() const {
  return observed_ == revision_->second.value;
}

void RecordWritten(const Connection &connection, TableId table) {
  const SessionState *state = SessionState::Peek();
  if (state == nullptr || state->recordChanges == nullptr ||
      &Session::Current().Database() != &connection) {
    return;
  }
  const auto found = state->recordChanges->tables_.find(table);
  if (found != state->recordChanges->tables_.end()) { ++found->second.value; }
}

}
