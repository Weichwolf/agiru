#pragma once

#include "runtime/ConnectionInfo.h"
#include "runtime/Database.h"

#include "Check.h"

#include <exception>
#include <string>
#include <string_view>

#include <unistd.h>

namespace gate {

class OwnedDatabase {
public:
  explicit OwnedDatabase(std::string_view suffix)
      : name_("agiru_owned_gate_" + std::to_string(getpid()) + "_" + std::string(suffix)),
        dsn_(agiru::ConnectionInfo(AGIRU_TEST_DSN).AtDatabase(name_)),
        drop_("DROP DATABASE " + name_),
        maintenance_(agiru::ConnectionInfo(AGIRU_TEST_DSN).AtDatabase("postgres")) {
    maintenance_.Run("SET client_min_messages = warning");
    maintenance_.Run("CREATE DATABASE " + name_);
  }

  OwnedDatabase(const OwnedDatabase &) = delete;
  OwnedDatabase &operator=(const OwnedDatabase &) = delete;
  OwnedDatabase(OwnedDatabase &&) = delete;
  OwnedDatabase &operator=(OwnedDatabase &&) = delete;

  ~OwnedDatabase() noexcept {
    try {
      maintenance_.Run(drop_);
    } catch (const std::exception &error) {
      CHECK_SILENT("the gate's owned database is removed", error.what());
    }
  }

  const std::string &Dsn() const { return dsn_; }

private:
  std::string name_;
  std::string dsn_;
  std::string drop_;
  agiru::Connection maintenance_;
};

}
