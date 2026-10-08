#include "runtime/PageInstance.h"

#include "meta/Ids.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageWindow.h"
#include "type/Action.h"

#include <cstddef>
#include <memory>

namespace agiru {

void PageInstance::PrepareBorrowed(void *object, PageId identity) {
  static_cast<void>(object);
  static_cast<void>(identity);
  throw Error("The page factory has no borrowed AL adapter.", "UiModalUnsupported");
}

Action PageInstance::CloseModal(Action action) {
  static_cast<void>(action);
  throw Error("The page factory has no modal close adapter.", "UiModalUnsupported");
}

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
