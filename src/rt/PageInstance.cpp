#include "runtime/PageInstance.h"

#include "meta/Ids.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"

#include <memory>

namespace agiru {

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
