#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>

extern "C" void xmlSetStructuredErrorFunc([[maybe_unused]] void *ctx,
                                          [[maybe_unused]] xmlStructuredErrorFunc handler) {
  std::fputs("xml-reader: fixture global-error-handler-write\n", stderr);
  std::abort();
}

namespace {

xmlExternalEntityLoader Previous() {
  static const xmlExternalEntityLoader held = ::xmlGetExternalEntityLoader();
  return held;
}

xmlParserInputPtr Trap(const char *url, const char *identifier, xmlParserCtxtPtr context) {
  if (url != nullptr && std::strstr(url, "/tmp/agiru-xml-entity-") != nullptr) {
    std::fputs("xml-reader: fixture external-resource-request\n", stderr);
    std::abort();
  }
  return Previous()(url, identifier, context);
}

[[gnu::constructor]] void Install() {
  static_cast<void>(Previous());
  ::xmlSetExternalEntityLoader(&Trap);
}

}
