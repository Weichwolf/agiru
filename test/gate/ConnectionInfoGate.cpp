#include "runtime/ConnectionInfo.h"
#include "runtime/Database.h"
#include "runtime/test/RunnerDatabase.h"

#include "Check.h"

#include <string>
#include <string_view>

namespace {

void ParsedNamesAndReplacementAgreeWithLibpq() {
  const agiru::ConnectionInfo uri(
      "postgresql://u:p@h:5433/path?dbname=actual%20source&sslmode=require");
  CHECK_TEXT("the query database overrides the URI path", uri.Database(), "actual source");
  const std::string replacement = uri.AtDatabase("quoted'\\target");
  CHECK_TEXT("a replacement name survives keyword escaping",
             agiru::ConnectionInfo(replacement).Database(),
             "quoted'\\target");
  CHECK_TRUE("the host is preserved", replacement.contains("host='h'"));
  CHECK_TRUE("the port is preserved", replacement.contains("port='5433'"));
  CHECK_TRUE("the user is preserved", replacement.contains("user='u'"));
  CHECK_TRUE("the password is preserved", replacement.contains("password='p'"));
  CHECK_TRUE("the SSL policy is preserved", replacement.contains("sslmode='require'"));
  const agiru::ConnectionInfo keyword("dbname = 'old name' dbname='final\\'name' user='a b'");
  CHECK_TEXT("repeated and quoted keyword settings follow libpq", keyword.Database(), "final'name");
  CHECK_TEXT("the runtime wrapper replaces the actual query database",
             agiru::ConnectionInfo(agiru::PointedAt("postgresql://h/path?dbname=actual",
                                                    agiru::DatabaseName{"scratch"}))
                 .Database(),
             "scratch");
}

void InvalidInputsRefuseWithoutDisclosingCredentials() {
  for (const std::string_view source : {"host=h password=credential",
                                        "dbname='' password=credential",
                                        "dbname=db invalid_option=credential"}) {
    bool refused = false;
    try {
      const agiru::ConnectionInfo info(source);
    } catch (const agiru::DatabaseError &error) {
      refused = true;
      CHECK_TRUE("parse diagnostics do not repeat credentials",
                 !std::string_view(error.what()).contains("credential"));
    }
    CHECK_TRUE("a missing database or invalid option is refused", refused);
  }
  const std::string embedded("dbname=db\0 password=credential",
                             sizeof("dbname=db\0 password=credential") - 1);
  bool refused = false;
  try {
    const agiru::ConnectionInfo info(embedded);
  } catch (const agiru::DatabaseError &) { refused = true; }
  CHECK_TRUE("NUL cannot truncate the connection identity", refused);
}

}

int main() {
  return gate::Run("ConnectionInfo", [] {
    ParsedNamesAndReplacementAgreeWithLibpq();
    InvalidInputsRefuseWithoutDisclosingCredentials();
  });
}
