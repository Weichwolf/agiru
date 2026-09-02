#include "Check.h"
#include "ResourceCost.h"
#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Transaction.h"

#include <cstddef>
#include <string>

using agiru::Error;
using agiru::Session;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostType;

namespace {

const agiru::TableDef &Table() {
  return agiru::TableTraits<ResourceCost>::kTable;
}

void Fresh() {
  DropTable(Session::Current().Database(), Table());
  CreateTable(Session::Current().Database(), Table());
}

ResourceCost Row(const char *code) {
  ResourceCost rec;
  rec.Type = ResourceCostType::Resource;
  rec.Code = code;
  return rec;
}

std::size_t Rows() {
  const agiru::Result found =
      Session::Current().Database().Execute(R"(SELECT count(*) FROM "Resource Cost")");
  const auto text = found.Value(0, 0);
  return text ? static_cast<std::size_t>(std::stoul(std::string(*text))) : 0;
}

/// A BOUNDARY THAT ROLLS BACK, which is the half a try/catch would not do. AL does not catch an
/// error; it abandons the write set and reports.
void AnErrorInsideABoundaryDiscardsWhatItWrote() {
  Fresh();
  Row("KEEP").Insert();
  CHECK_TRUE("the row before the boundary is there", Rows() == 1);

  agiru::AssertError([] {
    Row("GONE").Insert();
    throw Error("something went wrong");
  });

  CHECK_TRUE("the row written inside the boundary is gone", Rows() == 1);
  CHECK_TRUE("and the row before it is untouched", Rows() == 1);
  CHECK_TEXT("the message is where GetLastErrorText reads it",
             agiru::GetLastErrorText(),
             "something went wrong");
}

/// THE NEGATIVE CONTROL. A boundary that discarded everything would pass the case above too.
void ABoundaryThatDoesNotRaiseKeepsWhatItWrote() {
  Fresh();
  bool raised = false;
  try {
    agiru::AssertError([] { Row("KEPT").Insert(); });
  } catch (const Error &) { raised = true; }
  CHECK_TRUE("asserterror over a statement that does NOT raise is itself an error", raised);
  CHECK_TRUE("and the statement's row survives, because nothing rolled it back", Rows() == 1);
}

/// AL `Commit` MOVES THE BOUNDARY RATHER THAN RELEASING IT. Releasing would leave the block with
/// nothing to roll back to, so everything written after the Commit would survive an error that
/// should have discarded it -- the defect the predecessor records, in both directions.
void ACommitSurvivesALaterRollbackAndWhatFollowsItDoesNot() {
  Fresh();
  agiru::AssertError([] {
    Row("BEFORE").Insert();
    agiru::Commit();
    Row("AFTER").Insert();
    throw Error("after the commit");
  });

  CHECK_TRUE("exactly one row is left", Rows() == 1);
  const agiru::Result left =
      Session::Current().Database().Execute(R"(SELECT "Code" FROM "Resource Cost")");
  const auto code = left.Value(0, 0);
  CHECK_TEXT("and it is the one written BEFORE the commit",
             code ? std::string(*code) : std::string("<none>"),
             "BEFORE");
}

/// Boundaries nest, and an inner rollback must not take the outer one with it.
void AnInnerBoundaryRollsBackAloneable() {
  Fresh();
  agiru::AssertError([] {
    Row("OUTER").Insert();
    agiru::AssertError([] {
      Row("INNER").Insert();
      throw Error("the inner one");
    });
    throw Error("the outer one");
  });
  CHECK_TRUE("both are gone once the outer boundary rolls back", Rows() == 0);

  Fresh();
  Row("OUTER").Insert();
  agiru::AssertError([] {
    Row("INNER").Insert();
    throw Error("the inner one");
  });
  CHECK_TRUE("but an inner rollback leaves the outer row standing", Rows() == 1);
}

} // namespace

int main() {
  return gate::Run("Transaction", [] {
    const Session session(AGIRU_TEST_DSN);
    AnErrorInsideABoundaryDiscardsWhatItWrote();
    ABoundaryThatDoesNotRaiseKeepsWhatItWrote();
    ACommitSurvivesALaterRollbackAndWhatFollowsItDoesNot();
    AnInnerBoundaryRollsBackAloneable();
  });
}
