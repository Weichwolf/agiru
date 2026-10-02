#include "fixture/page/CollisionHost.h"
#include "runtime/Session.h"
#include "Check.h"

int main() {
  return gate::Run("PageCollisionExecution", [] {
    const agiru::Session session("postgresql://agiru:agiru@localhost:5433/agiru_gate");
    agiru::Fixture::CollisionHost_Page page;
    CHECK_TRUE("both procedures keep their own implementation", page.Invoke() == 18);
    CHECK_TRUE("the first AL part receives its own call", page.DispatchWork_3.Page().GetCalls() == 1);
    CHECK_TRUE("the second AL part receives its own call", page.DispatchWork_4.Page().GetCalls() == 1);
    CHECK_TRUE("original AL name selects the first instance",
               page.PartInstance("Dispatch Work") == &page.DispatchWork_3.Held());
    CHECK_TRUE("original AL name selects the second instance",
               page.PartInstance("Dispatch-Work") == &page.DispatchWork_4.Held());
    CHECK_TRUE("separate AL controls never alias", &page.DispatchWork_3.Held() != &page.DispatchWork_4.Held());
  });
}
