#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/test/RunnerDatabase.h"

#include "Check.h"

#include <string>

using agiru::Connection;
using agiru::DatabaseError;
using agiru::DatabaseName;
using agiru::Error;
using agiru::PointedAt;
using agiru::RunnerDatabase;

namespace {

// A CONNECTION STRING IS TWO FORMS AND libpq TAKES BOTH. The tree's own setting is the URI, but a
// keyword string is what a container hands out, and a runner that silently connected to the wrong
// database would look like a test defect for as long as anyone cared to look.
void TheDsnIsPointedElsewhere() {
  CHECK_TEXT("the URI's database is replaced",
             PointedAt("postgresql://u:p@h:5433/agiru_master", DatabaseName{"agiru_test_0"}),
             "postgresql://u:p@h:5433/agiru_test_0");
  CHECK_TEXT("and its parameters survive",
             PointedAt("postgresql://u:p@h:5433/agiru_master?sslmode=require",
                       DatabaseName{"agiru_test_0"}),
             "postgresql://u:p@h:5433/agiru_test_0?sslmode=require");
  CHECK_TEXT("the keyword form's database is replaced",
             PointedAt("host=h port=5433 dbname=agiru_master user=u", DatabaseName{"agiru_test_0"}),
             "host=h port=5433 dbname=agiru_test_0 user=u");

  // AND A STRING THAT NAMES NO DATABASE IS REFUSED. Defaulting to the server's own idea of one
  // would put the runner's writes wherever libpq happened to land.
  bool refused = false;
  try {
    (void)PointedAt("host=h port=5433 user=u", DatabaseName{"agiru_test_0"});
  } catch (const DatabaseError &) { refused = true; }
  CHECK_TRUE("a string naming no database is refused", refused);
}

// THE CLONE CARRIES THE TEMPLATE'S CONTENT AND THE TEMPLATE NEVER SEES THE CLONE'S. That is the
// whole claim: physical write isolation. The predecessor ran the AL suite and its gate against one
// database and spent a session chasing eleven failures that belonged to the seed (openerp WI-832).
void TheCloneCarriesTheTemplateAndTheTemplateStaysClean() {
  // THE GATE MAKES ITS OWN TEMPLATE AND DOES NOT CLONE THE MASTER. Two reasons, and the second is
  // the one that decides: a case that reads the master's state fails for what an earlier case left
  // there, and PostgreSQL refuses to copy a database ANY session is connected to -- so a template
  // is a database nobody opens, which the master, being the gate's own target, is not.
  {
    const Connection maintenance(PointedAt(AGIRU_TEST_DSN, DatabaseName{"postgres"}));
    maintenance.Run("SET client_min_messages = warning");
    maintenance.Run("DROP DATABASE IF EXISTS agiru_runner_template");
    maintenance.Run("CREATE DATABASE agiru_runner_template");
  }
  {
    const Connection seed(PointedAt(AGIRU_TEST_DSN, DatabaseName{"agiru_runner_template"}));
    seed.Run("CREATE TABLE runner_gate (who text NOT NULL)");
    seed.Run("INSERT INTO runner_gate (who) VALUES ('template')");
  }

  const std::string tpl = PointedAt(AGIRU_TEST_DSN, DatabaseName{"agiru_runner_template"});
  {
    const RunnerDatabase runner(tpl, "agiru_runner_gate", true);
    CHECK_TRUE("the runner's dsn is not the template's", runner.Dsn() != tpl);
    CHECK_TRUE("the first call clones it", runner.Cloned());
    const Connection clone(runner.Dsn());
    const agiru::Result carried = clone.Execute("SELECT who FROM runner_gate");
    CHECK_TRUE("the clone carries what the template held",
               carried.Rows() == 1 && carried.Value(0, 0) == "template");
    clone.Run("INSERT INTO runner_gate (who) VALUES ('clone')");
    const Connection reader(tpl);
    const agiru::Result untouched = reader.Execute("SELECT count(*) FROM runner_gate");
    CHECK_TRUE("and what the clone writes does not reach the template",
               untouched.Value(0, 0) == "1");
  }

  // AND A SECOND RUN TAKES THE ONE THAT IS THERE. `TestIsolation = Codeunit` hands the database
  // back the way it was found, so there is nothing for a re-clone to restore -- and a clone costs
  // the master's BYTES, which is the CRONUS dataset once that is loaded.
  {
    const RunnerDatabase again(tpl, "agiru_runner_gate");
    CHECK_TRUE("a second call does not clone again", !again.Cloned());
    const Connection kept(again.Dsn());
    CHECK_TRUE("and it is the database that was there, not a new one",
               kept.Execute("SELECT count(*) FROM runner_gate").Value(0, 0) == "2");
  }

  // WHICH IS WHY `--fresh` EXISTS. A run that inherits what a crash left mid-write points the
  // diagnosis at the wrong tree, and the way back has to be a flag rather than a doubt.
  const RunnerDatabase forced(tpl, "agiru_runner_gate", true);
  CHECK_TRUE("--fresh clones it again", forced.Cloned());
  {
    const Connection reborn(forced.Dsn());
    CHECK_TRUE("and what it carries is the template's again",
               reborn.Execute("SELECT count(*) FROM runner_gate").Value(0, 0) == "1");
  }
  forced.Drop();

  const Connection maintenance(PointedAt(AGIRU_TEST_DSN, DatabaseName{"postgres"}));
  maintenance.Run("SET client_min_messages = warning");
  maintenance.Run("DROP DATABASE IF EXISTS agiru_runner_template");
}

} // namespace

int main() {
  return gate::Run("RunnerDatabase", [] {
    TheDsnIsPointedElsewhere();
    try {
      TheCloneCarriesTheTemplateAndTheTemplateStaysClean();
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}
