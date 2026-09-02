#include "Check.h"
#include "runtime/Codeunit.h"

#include <utility>

namespace {

/// A stand-in for a generated object: it counts what the handle does to it, which is the only way
/// to prove laziness at all -- an eager handle and a lazy one are indistinguishable from outside.
constexpr int kInitial = 7;
constexpr int kWritten = 9;

struct Counted {
  static int made;
  static int gone;

  Counted() { ++made; }

  Counted(const Counted &) = delete;
  Counted(Counted &&) = delete;
  Counted &operator=(const Counted &) = delete;
  Counted &operator=(Counted &&) = delete;

  ~Counted() { ++gone; }

  /// What the handle reads and writes through, so that the gate can tell one instance from another.
  [[nodiscard]] int Value() const { return value_; }

  /// \param value What to write.
  void Value(int value) { value_ = value; }

private:
  int value_ = kInitial;
};

int Counted::made = 0;
int Counted::gone = 0;

void Reset() {
  Counted::made = 0;
  Counted::gone = 0;
}

/// AN OBJECT MEMBER IS MADE ON FIRST USE, and AL is the reason rather than C++.
///
/// `Background Error Handling Mgt.` holds an `Item Journal Errors Mgt.` and that codeunit holds the
/// first one back; `Currency Exchange Rate` declares a variable of its own type. An eager member is
/// a class of infinite size, in C++ and in AL alike, so AL cannot be constructing eagerly either
/// (board:0037).
void AHandleIsMadeOnFirstUse() {
  Reset();
  {
    agiru::Instance<Counted> handle;
    CHECK_TRUE("declaring it makes nothing", Counted::made == 0);
    CHECK_TRUE("and reaching through it makes one", handle->Value() == kInitial);
    CHECK_TRUE("exactly one", Counted::made == 1);
    handle->Value(kWritten);
    CHECK_TRUE("which is the same one every time after", handle->Value() == kWritten);
    CHECK_TRUE("still one", Counted::made == 1);
  }
  CHECK_TRUE("and it is freed when the holder goes", Counted::gone == 1);
}

/// A HANDLE THAT WAS NEVER USED FREES NOTHING, which is what makes the destructor work on an
/// incomplete type: the freeing function is captured where the instance is made, and `delete` needs
/// the definition only there.
void AnUnusedHandleFreesNothing() {
  Reset();
  {
    const agiru::Instance<Counted> handle;
    static_cast<void>(handle);
  }
  CHECK_TRUE("nothing was made", Counted::made == 0);
  CHECK_TRUE("and nothing was freed", Counted::gone == 0);
}

int Twice(Counted &counted) {
  return counted.Value() * 2;
}

/// IT DISAPPEARS AT EVERY USE BUT THE ONE C++ CANNOT HIDE. The handle is how agiru DECLARES the
/// member; AL code writes `Copy(TempBuffer, true)` and hands the object itself.
void AHandleConvertsToTheObject() {
  Reset();
  agiru::Instance<Counted> handle;
  CHECK_TRUE("passing it as the object works", Twice(handle) == kInitial * 2);
  CHECK_TRUE("and that is what made it", Counted::made == 1);
}

/// MOVING TAKES THE INSTANCE RATHER THAN SHARING IT. Two codeunit variables in AL are two
/// instances, so a copy that shared one pointer would free it twice.
void MovingTakesTheInstance() {
  Reset();
  {
    agiru::Instance<Counted> from;
    from->Value(kWritten);
    agiru::Instance<Counted> to = std::move(from);
    CHECK_TRUE("the instance moved with it", to->Value() == kWritten);
    CHECK_TRUE("and no second one was made", Counted::made == 1);
  }
  CHECK_TRUE("it is freed once", Counted::gone == 1);
}

} // namespace

int main() {
  return gate::Run("Instance", [] {
    AHandleIsMadeOnFirstUse();
    AnUnusedHandleFreesNothing();
    AHandleConvertsToTheObject();
    MovingTakesTheInstance();
  });
}
