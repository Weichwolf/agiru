#pragma once

#include "meta/Ids.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>

namespace agiru {
class Connection;
}

namespace agiru::detail {

class RecordRead;

class RecordChanges {
public:
  [[nodiscard]] std::size_t ActiveTables() const { return tables_.size(); }

private:
  friend class RecordRead;
  friend void RecordWritten(const Connection &connection, TableId table);
  friend void RefreshRecordReads(std::optional<TableId> table);

  struct Revision {
    std::uint64_t value = 0;
    std::uint64_t refreshed = 0;
    std::size_t readers = 0;
  };

  std::map<TableId, Revision> tables_;
};

class RecordRead {
public:
  explicit RecordRead(TableId table, bool locked = false);
  ~RecordRead();
  RecordRead(const RecordRead &) = delete;
  RecordRead &operator=(const RecordRead &) = delete;
  RecordRead(RecordRead &&) = delete;
  RecordRead &operator=(RecordRead &&) = delete;
  [[nodiscard]] bool Current() const;

private:
  std::shared_ptr<RecordChanges> owner_;
  std::map<TableId, RecordChanges::Revision>::iterator revision_;
  std::uint64_t observed_;
  std::uint64_t refreshed_;
  bool locked_;
};

void RecordWritten(const Connection &connection, TableId table);
void RefreshRecordReads(std::optional<TableId> table);

}
