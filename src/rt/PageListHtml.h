#pragma once

#include "runtime/PageHtml.h"
#include "runtime/PageWindow.h"
#include "type/RecordId.h"

#include <cstddef>
#include <string>
#include <vector>

namespace agiru {

class PageCore;
class PageAuthorization;
struct PageDef;

struct PageListRow {
  std::string handle;
  RecordId identity;
  PageHtmlResult presentation;
  std::size_t controls = 0;
};

struct PageListView {
  std::size_t limit;
  PageWindowState state{};
  bool previous = false;
  bool next = false;
  bool backwards = false;
  RecordId selected{};
  std::vector<PageListRow> rows{};
};

class PageListLoader final : public PageWindowReceiver {
public:
  PageListLoader(const PageDef &declaration,
                 PageAuthorization &authorization,
                 PageListView &view,
                 PageHtmlLimits limits = {});
  void Row(const RecordId &identity, PageCore &controls) override;
  void Current(const RecordId &identity, PageCore &controls) override;

private:
  PageListRow Capture_(const RecordId &identity, PageCore &controls);
  const PageDef &declaration_;
  PageAuthorization &authorization_;
  PageListView &view_;
  PageHtmlLimits limits_;
  std::size_t bytes_ = 0;
  std::size_t controls_ = 0;
};

[[nodiscard]] PageHtmlResult RenderPageListHtml(const PageDef &declaration,
                                                PageCore &page,
                                                PageAuthorization &authorization,
                                                const PageHtmlContext &context,
                                                const PageListView &view,
                                                PageHtmlLimits limits = {});

}
