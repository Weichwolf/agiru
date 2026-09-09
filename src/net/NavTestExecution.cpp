#include "dotnet/NavTestExecution.h"

#include "runtime/test/Handlers.h"
#include "type/Boolean.h"

namespace agiru::dotnet {

::agiru::Boolean NavTestExecution::IsInTestMode() {
  return HandlerTable::Installed();
}

}
