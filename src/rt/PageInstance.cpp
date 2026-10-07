#include "runtime/PageInstance.h"

#include "meta/Ids.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageWindow.h"

#include <cstddef>
#include <memory>

namespace agiru {

PageWindowState PageInstance::OpenWindow([[maybe_unused]] PageOpenMode mode,
                                         [[maybe_unused]] std::size_t limit,
                                         [[maybe_unused]] PageWindowReceiver &receiver) {
  throw Error("The page has no qualified list window adapter.", "PageWindowProvider");
}

PageWindowState PageInstance::ReadWindow([[maybe_unused]] PageWindowPosition position,
                                         [[maybe_unused]] std::size_t limit,
                                         [[maybe_unused]] PageWindowReceiver &receiver) {
  throw Error("The page has no qualified list window adapter.", "PageWindowProvider");
}

bool PageInstance::SelectWindowRecord([[maybe_unused]] const RecordId &record) {
  throw Error("The page has no qualified list window adapter.", "PageWindowProvider");
}

std::unique_ptr<PageInstance> MakeInstalledPage(PageId page) {
  const PageEntry *entry = FindPage(page);
  if (entry == nullptr) { throw Error("The page is not installed.", "PageMissing"); }
  if (entry->makeSession == nullptr) {
    throw Error("The page has no linked interactive factory.", "PageFactoryMissing");
  }
  std::unique_ptr<PageInstance> instance{entry->makeSession()};
  if (instance == nullptr || instance->IsOpen() || &instance->Declaration() != entry->page) {
    throw Error("The page factory returned an invalid instance.", "PageFactoryMismatch");
  }
  return instance;
}

}
