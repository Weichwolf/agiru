#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/NativeService.h"
#include "runtime/PageCommandHost.h"
#include "runtime/SecureToken.h"

#include "Check.h"
#include "JsonEngine.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <sys/stat.h>

namespace {

using Node = agiru::detail::JsonNode;

std::string Template() {
  std::ifstream file(std::string(AGIRU_SOURCE_DIR) + "/deploy/dev/agiru.json");
  if (!file) { throw std::runtime_error("server template is required"); }
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

bool Refuses(std::string_view text) {
  try {
    static_cast<void>(agiru::ParseNativeServiceOptions(text));
  } catch (const agiru::Error &error) {
    return error.Code() == "ServerConfiguration" &&
           std::string_view(error.what()) == "invalid server configuration";
  }
  return false;
}

void Defaults() {
  const auto options =
      agiru::LoadNativeServiceOptions(std::string(AGIRU_SOURCE_DIR) + "/deploy/dev/agiru.json");
  const agiru::HttpServerOptions http;
  const agiru::PageHostOptions pages;
  CHECK_TRUE("template explicitly declares every HTTP default",
             options.http.port == http.port && options.http.loopback == http.loopback &&
                 options.http.workers == http.workers && options.http.queue == http.queue &&
                 options.http.connections == http.connections &&
                 options.http.timeoutSeconds == http.timeoutSeconds &&
                 options.http.bodyBytes == http.bodyBytes &&
                 options.http.totalBodyBytes == http.totalBodyBytes &&
                 options.http.responseBytes == http.responseBytes);
  CHECK_TRUE("template explicitly declares every page and transaction default",
             options.pages.contexts == pages.contexts &&
                 options.pages.navigationDepth == pages.navigationDepth &&
                 options.pages.commands == pages.commands &&
                 options.pages.receiptBytes == pages.receiptBytes &&
                 options.pages.lifetime == pages.lifetime &&
                 options.pages.session.disableWriteInsideTryFunctions ==
                     pages.session.disableWriteInsideTryFunctions);
  CHECK_TRUE("deployment examples contain no password or synthetic credentials",
             !options.pages.database.contains("password") &&
                 options.pages.company == "CRONUS International Ltd" &&
                 options.pages.origin == "http://127.0.0.1:8080");
  auto root = Node::parse(Template());
  root["http"]["workers"] = Node(2);
  root["transactions"]["disable_write_inside_try_functions"] = Node(true);
  root["pages"]["receipt_bytes"] = Node::Number("9007199254740993");
  root["company"] = Node(std::string("Original + Gesellschaft 東京"));
  const auto selected = agiru::ParseNativeServiceOptions(root.dump());
  CHECK_TRUE("trusted explicit worker and transaction policies are retained",
             selected.http.workers == 2 && selected.pages.session.disableWriteInsideTryFunctions);
  CHECK_TRUE("integer budgets never travel through binary floating point",
             selected.pages.receiptBytes == 9007199254740993ULL);
  root["company"] = Node(std::string("Changed"));
  CHECK_TEXT("configuration owns unchanged Unicode authority strings",
             selected.pages.company,
             "Original + Gesellschaft 東京");
}

void Schema() {
  const auto original = Node::parse(Template());
  for (const auto &member : original.Members()) {
    auto missing = original;
    missing.erase(member.first);
    CHECK_TRUE("missing root authority or settings never silently default",
               Refuses(missing.dump()));
  }
  for (const std::string_view section : {"http", "pages", "transactions"}) {
    for (const auto &member : original[section].Members()) {
      auto missing = original;
      missing[section].erase(member.first);
      CHECK_TRUE("missing nested settings never silently default", Refuses(missing.dump()));
    }
    auto extra = original;
    extra[section]["unrecognized_secret"] = Node(std::string("NEVER-ECHO-SECRET"));
    CHECK_TRUE("unknown nested fields refuse without echoing values", Refuses(extra.dump()));
  }
  auto extra = original;
  extra["unrecognized_secret"] = Node(std::string("NEVER-ECHO-SECRET"));
  CHECK_TRUE("unknown root fields refuse without echoing values", Refuses(extra.dump()));
  for (const std::string_view text : {"", "[]", "null", "{", "{}", "{} trailing"}) {
    CHECK_TRUE("malformed or nonobject configuration refuses", Refuses(text));
  }
  for (const std::string_view field : {"database", "company", "origin"}) {
    for (const auto &invalid :
         {Node{}, Node(false), Node(1), Node(std::string()), Node(std::string("a\0secret", 8))}) {
      auto root = original;
      root[field] = invalid;
      CHECK_TRUE("authority strings must be nonempty unchanged text without NUL",
                 Refuses(root.dump()));
    }
  }
  auto version = original;
  version["schema"] = Node(2);
  CHECK_TRUE("unknown configuration versions refuse", Refuses(version.dump()));
  version["schema"] = Node(std::string("1"));
  CHECK_TRUE("configuration version is not coerced from text", Refuses(version.dump()));
  for (const std::string_view value : {"host=localhost",
                                       "dbname=''",
                                       "service=missing",
                                       "NEVER-ECHO-SECRET=value",
                                       "postgresql://%broken"}) {
    auto database = original;
    database["database"] = Node(std::string(value));
    CHECK_TRUE("database syntax requires an explicit identity without contacting SQL",
               Refuses(database.dump()));
  }
  auto publicListener = original;
  publicListener["http"]["loopback"] = Node(false);
  CHECK_TRUE("native ERP listener cannot bypass Caddy with a public bind",
             Refuses(publicListener.dump()));
  publicListener["http"]["loopback"] = Node(1);
  CHECK_TRUE("booleans are not coerced from numbers", Refuses(publicListener.dump()));
  auto policy = original;
  policy["transactions"]["disable_write_inside_try_functions"] = Node(std::string("false"));
  CHECK_TRUE("transaction policy is not coerced from text", Refuses(policy.dump()));
}

void DuplicatesAndNumbers() {
  const auto original = Node::parse(Template());
  const auto compact = original.dump();
  for (const std::string_view anchor : {"\"schema\":1",
                                        "\"port\":18080",
                                        "\"contexts\":64",
                                        "\"disable_write_inside_try_functions\":false"}) {
    auto duplicate = compact;
    const auto at = duplicate.find(anchor);
    if (at == std::string::npos) { throw std::runtime_error("duplicate control anchor absent"); }
    duplicate.insert(at, std::string(anchor) + ",");
    CHECK_TRUE("duplicate configuration keys refuse even when their values agree",
               Refuses(duplicate));
    CHECK_TRUE("ordinary AL JSON parsing retains its independent duplicate policy",
               Node::parse(duplicate).is_object());
  }
  auto escaped = compact;
  escaped.insert(escaped.find("\"schema\":1"), R"("\u0073chema":1,)");
  CHECK_TRUE("escaped duplicate key identity is rejected", Refuses(escaped));
  for (const std::string_view section : {"http", "pages"}) {
    for (const auto &member : original[section].Members()) {
      if (member.first == "loopback") { continue; }
      for (const auto &bad : {Node::Number("0"),
                              Node::Number("-1"),
                              Node::Number("1.0"),
                              Node::Number("1e0"),
                              Node::Number("18446744073709551616"),
                              Node(true),
                              Node(std::string("1"))}) {
        auto root = original;
        root[section][member.first] = bad;
        CHECK_TRUE("all bounds reject nonpositive overflow fractional exponent and coerced values",
                   Refuses(root.dump()));
      }
    }
  }
  for (const auto &[field, value] : {std::pair{"port", "65536"},
                                     {"workers", "257"},
                                     {"queue", "4097"},
                                     {"connections", "4294967296"},
                                     {"timeout_seconds", "121"},
                                     {"body_bytes", "16777217"},
                                     {"response_bytes", "1"},
                                     {"total_body_bytes", "1"}}) {
    auto root = original;
    root["http"][field] = Node::Number(value);
    CHECK_TRUE("server parser shares native transport resource validation", Refuses(root.dump()));
  }
  auto lifetime = original;
  constexpr auto kOneDay = std::chrono::hours(24);
  lifetime["pages"]["lifetime_seconds"] =
      Node(std::chrono::duration_cast<std::chrono::seconds>(kOneDay).count() + 1);
  CHECK_TRUE("server parser shares retained-page lifetime validation", Refuses(lifetime.dump()));
}

class Files {
public:
  Files() : directory_("/tmp/agiru-config-gate." + agiru::GenerateSecureToken()) {
    if (mkdir(directory_.c_str(), S_IRWXU) != 0) {
      throw std::runtime_error("config fixture creation failed");
    }
  }

  ~Files() {
    std::error_code error;
    std::filesystem::remove_all(directory_, error);
    CHECK_TRUE("owned temporary configuration files are removed", !error);
  }

  Files(const Files &) = delete;
  Files &operator=(const Files &) = delete;

  std::string Path(std::string_view name) const { return (directory_ / name).string(); }

  void Write(std::string_view name, std::string_view text) const {
    std::ofstream file(Path(name), std::ios::binary);
    file << text;
    file.close();
    if (!file) { throw std::runtime_error("config fixture writing failed"); }
  }

private:
  std::filesystem::path directory_;
};

bool RefusesFile(std::string_view path) {
  try {
    static_cast<void>(agiru::LoadNativeServiceOptions(path));
  } catch (const agiru::Error &error) {
    return error.Code() == "ServerConfiguration" &&
           std::string_view(error.what()) == "invalid server configuration";
  }
  return false;
}

void FileBounds() {
  const Files files;
  const auto text = Template();
  files.Write("valid.json", text);
  CHECK_TRUE("bounded regular files load without contacting the example database",
             agiru::LoadNativeServiceOptions(files.Path("valid.json")).http.port == 18080);
  CHECK_TRUE("missing files fail without disclosing their path", RefusesFile(files.Path("SECRET")));
  CHECK_TRUE("directories are not configuration files", RefusesFile(files.Path("")));
  CHECK_TRUE("device input is not configuration", RefusesFile("/dev/null"));
  std::filesystem::create_symlink(files.Path("valid.json"), files.Path("link.json"));
  CHECK_TRUE("symbolic links do not redirect trusted startup authority",
             RefusesFile(files.Path("link.json")));
  if (mkfifo(files.Path("pipe").c_str(), S_IRUSR | S_IWUSR) != 0) {
    throw std::runtime_error("fixture FIFO failed");
  }
  CHECK_TRUE("FIFO input refuses without waiting for a writer", RefusesFile(files.Path("pipe")));
  files.Write("empty.json", "");
  CHECK_TRUE("empty files do not select implicit server defaults",
             RefusesFile(files.Path("empty.json")));
  const std::string oversized(agiru::NativeServiceOptions::kConfigBytes + 1, ' ');
  files.Write("large.json", oversized);
  CHECK_TRUE("oversized regular files refuse", RefusesFile(files.Path("large.json")));
  CHECK_TRUE("inline parsing has the same byte ceiling", Refuses(oversized));
  const auto bounded =
      text + std::string(agiru::NativeServiceOptions::kConfigBytes - text.size(), ' ');
  files.Write("bounded.json", bounded);
  CHECK_TRUE("the exact byte ceiling is accepted without truncation",
             agiru::LoadNativeServiceOptions(files.Path("bounded.json")).http.port == 18080);
}

}

int main() {
  return gate::Run("NativeServiceConfig", [] {
    Defaults();
    Schema();
    DuplicatesAndNumbers();
    FileBounds();
  });
}
