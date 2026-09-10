#include "runtime/Codeunit.h"

#include "Check.h"

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

// A RECORD IS COPIED CONSTANTLY and a table with a `var` block holds an `Instance`, so the handle
// must be copyable. A copy HOLDS A COPY of what the other made -- `Rec2 := Rec` copies the fields
// -- and until 2026-09-09 it held nothing: the assignment released the left side, and a codeunit
// instance cloned for an interface came back with every record global blank. What cannot be copied
// (this gate's `Counted`) is the one case where the copy still holds nothing.
namespace {

struct Copyable {
  static int made;
  int value = 0;

  Copyable() { ++made; }

  Copyable(const Copyable &o) : value(o.value) { ++made; }

  Copyable &operator=(const Copyable &) = default;
  Copyable(Copyable &&) = delete;
  Copyable &operator=(Copyable &&) = delete;
  ~Copyable() = default;
};

int Copyable::made = 0;

void ACopyHoldsACopyOfWhatTheOtherMade() {
  Copyable::made = 0;
  agiru::Instance<Copyable> first;
  first->value = 7;
  CHECK_TRUE("the first handle made one", Copyable::made == 1);
  agiru::Instance<Copyable> second(first);
  CHECK_TRUE("the copy holds a copy", Copyable::made == 2 && second->value == 7);
  second->value = 8;
  CHECK_TRUE("which is its own", first->value == 7);
  agiru::Instance<Copyable> third;
  third = first;
  CHECK_TRUE("and assignment copies too", third->value == 7 && Copyable::made == 3);
}

void ACopyOfWhatCannotBeCopiedHoldsNothing() {
  Counted::made = 0;
  Counted::gone = 0;
  {
    agiru::Instance<Counted> first;
    static_cast<void>(*first);
    CHECK_TRUE("the first handle made one", Counted::made == 1);
    agiru::Instance<Counted> second(first);
    CHECK_TRUE("the copy made nothing", Counted::made == 1);
    static_cast<void>(*second);
    CHECK_TRUE("and makes its own on first use", Counted::made == 2);
  }
  CHECK_TRUE("both are freed exactly once", Counted::gone == 2);
}

/// A RECORD'S GLOBALS ARE THE VARIABLE'S OWN. `Rec := Other` copies fields; the `Var_Block` handle
/// keeps what this variable made, and a copy starts unmade (`Globals`, see its \warning).
void AssigningKeepsTheVariablesOwnGlobals() {
  Reset();
  {
    agiru::Globals<Counted> mine;
    mine->Value(kWritten);
    agiru::Globals<Counted> other;
    mine = other;
    CHECK_TRUE("assigning from an unmade handle keeps mine", mine->Value() == kWritten);
    CHECK_TRUE("and makes nothing", Counted::made == 1);
    other->Value(kWritten + 1);
    mine = other;
    CHECK_TRUE("assigning from a made one keeps mine too", mine->Value() == kWritten);
    agiru::Globals<Counted> copy(mine);
    CHECK_TRUE("a copy starts unmade", Counted::made == 2);
    CHECK_TRUE("and makes its own on first use", copy->Value() == kInitial && Counted::made == 3);
  }
  CHECK_TRUE("each is freed exactly once", Counted::gone == 3);
}
}

int main() {
  return gate::Run("Instance", [] {
    ACopyHoldsACopyOfWhatTheOtherMade();
    ACopyOfWhatCannotBeCopiedHoldsNothing();
    AHandleIsMadeOnFirstUse();
    AnUnusedHandleFreesNothing();
    AHandleConvertsToTheObject();
    MovingTakesTheInstance();
    AssigningKeepsTheVariablesOwnGlobals();
  });
}
