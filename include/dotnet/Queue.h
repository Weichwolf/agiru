#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <cstddef>
#include <deque>
#include <utility>

namespace agiru::dotnet {

/// \brief .NET `System.Collections.Queue`, rebuilt over Variants: first in, first out.
///
/// `Workflow.CreateInstance` walks a workflow's steps breadth-first through three of them,
/// enqueuing each step's `ToString()` and dequeuing it into `FindByAttributes` (2 cases of
/// Workflow Engine UT, 2026-09-12). An element goes in as whatever AL handed over and comes out
/// as a `Variant`, which AL unwraps into the parameter it reaches -- the way `object` does in .NET.
class Queue {
public:
  /// \brief The binder behind `Q := Q.Queue()`.
  struct Binder {
    /// \brief `new Queue()`. \return An empty queue.
    [[nodiscard]] class Queue operator()() const { return {}; }
  };

  /// \brief The constructor, spelled the way AL spells it: `Queue := Queue.Queue()`.
  Binder Queue; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief `Queue.Enqueue(Object)`: adds an element at the end.
  /// \param item The element.
  void Enqueue(const Variant &item) { items_.push_back(item); }

  /// \brief `Queue.Dequeue()`: removes and returns the element at the beginning.
  /// \return The element.
  /// \throws Error when the queue is empty, which is .NET's `InvalidOperationException`.
  [[nodiscard]] Variant Dequeue() {
    if (items_.empty()) { throw Error("Queue empty."); }
    Variant first = std::move(items_.front());
    items_.pop_front();
    return first;
  }

  /// \brief `Queue.Peek()`: the element at the beginning, left in place.
  /// \return The element.
  /// \throws Error when the queue is empty.
  [[nodiscard]] Variant Peek() const {
    if (items_.empty()) { throw Error("Queue empty."); }
    return items_.front();
  }

  /// \brief `Queue.Count`: how many elements are queued. \return The count.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(items_.size()); }

  /// \brief `Queue.Clear()`: removes every element.
  void Clear() { items_.clear(); }

  /// \brief AL `foreach`: the first element. \return The iterator.
  [[nodiscard]] std::deque<Variant>::const_iterator begin() const { return items_.begin(); }

  /// \brief AL `foreach`: past the last element. \return The iterator.
  [[nodiscard]] std::deque<Variant>::const_iterator end() const { return items_.end(); }

private:
  std::deque<Variant> items_;
};

}
