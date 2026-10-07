#pragma once

#include <string>
#include <string_view>

namespace agiru::detail {

class RecordOrder;
struct Selection;

[[nodiscard]] std::string SqlRecordOrder(const RecordOrder &by, bool descending);
void SeekRecord(Selection &made, const RecordOrder &by, const void *record, std::string_view op);

}
