#include "dotnet/Queue.h"
#include "runtime/Error.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "Check.h"

#include <string>

using agiru::Variant;
using agiru::dotnet::Queue;

namespace {

/// .NET's `Queue` IS FIRST IN, FIRST OUT, and its constructor is spelled `Queue.Queue()` the way
/// AL spells it. `Workflow.CreateInstance` enqueues step descriptions and dequeues them into a
/// `Text` parameter, so what comes out must read as the text that went in; an empty queue refuses
/// a `Dequeue` the way .NET does rather than answering nothing.
void FirstInFirstOut() {
  Queue queue;
  queue = queue.Queue();
  CHECK_TRUE("a new queue is empty", queue.Count() == 0);
  queue.Enqueue(Variant(std::string("step 1")));
  queue.Enqueue(Variant(std::string("step 2")));
  queue.Enqueue(Variant(agiru::Integer{3}));
  CHECK_TRUE("three enqueued", queue.Count() == 3);
  const agiru::Text<0> first{queue.Dequeue()};
  CHECK_TEXT("the first in is the first out", std::string(std::string_view(first)), "step 1");
  CHECK_TEXT("Peek shows the next without taking it",
             std::string(std::string_view(queue.Peek())),
             "step 2");
  CHECK_TRUE("Peek left it in place", queue.Count() == 2);
  static_cast<void>(queue.Dequeue());
  CHECK_TRUE("an Integer element reads back as one", queue.Dequeue().Get<agiru::Integer>() == 3);
  std::string refusal;
  try {
    static_cast<void>(queue.Dequeue());
  } catch (const agiru::Error &e) { refusal = e.what(); }
  CHECK_TEXT("an empty queue refuses a Dequeue with .NET's text", refusal, "Queue empty.");
}

}

int main() {
  return gate::Run("Queue", [] { FirstInFirstOut(); });
}
