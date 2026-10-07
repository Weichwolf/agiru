#pragma once

#include "meta/Ids.h"
#include "runtime/PageWindow.h"

#include <cstddef>
#include <cstdint>
#include <memory>

/// \file
/// \brief Type-erased lifecycle for generated interactive pages, independent of clients.

namespace agiru {

struct PageDef;
class RecordId;
class PageCore;

/// \brief Explicit opening mode; unknown input never defaults to an editable page.
enum class PageOpenMode : std::uint8_t {
  Unknown,
  View,
  Edit,
  New,
};

/// \brief Movement within the page's current record view.
enum class PagePosition : std::uint8_t {
  Unknown,
  First,
  Next,
  Previous,
  Last,
};

/// \brief Owned interactive lifecycle and control binding for one generated page instance.
/// The caller serializes its authenticated session and supplies permission/transaction
/// boundaries. This interface neither grants access nor runs a headless Page.Run handler.
class PageInstance {
public:
  /// \brief Releases the owning production adapter through this interface.
  virtual ~PageInstance() = default;

  /// \brief The existing control kernel owned by this instance, without a test facade.
  /// \return A borrowed adapter with the same lifetime as this instance.
  [[nodiscard]] virtual PageCore &Controls() = 0;

  /// \brief Borrows the immutable declaration belonging to this typed instance.
  /// \return The declaration; it outlives the instance.
  [[nodiscard]] virtual const PageDef &Declaration() const = 0;

  /// \brief Opens a fresh AL page using its existing lifecycle/trigger kernel.
  /// \param mode Required opening mode.
  /// \throws Error for unknown modes, double opening or AL trigger failures.
  virtual void Open(PageOpenMode mode) = 0;

  /// \brief Saves pending edits, runs the existing close triggers and releases the page.
  /// \throws Error from save or close triggers; failures must not be reported as success.
  virtual void Close() = 0;

  /// \brief Saves the pending source row through the existing row-leave kernel.
  /// \throws Error from validation/insertion/modification; never collects test errors.
  virtual void Save() = 0;

  /// \brief Whether this handle currently owns or borrows an open typed page.
  [[nodiscard]] virtual bool IsOpen() const = 0;

  /// \brief Moves using the existing filtered record cursor and page trigger order.
  /// \param position The requested movement.
  /// \return Whether an existing row was reached.
  /// \throws Error for unknown movement or AL failures.
  [[nodiscard]] virtual bool Move(PagePosition position) = 0;

  /// \brief Selects a typed record identity through the page's existing positioning kernel.
  /// \param record Table identity and primary key, not a display caption or row offset.
  /// \return Whether the record was found.
  /// \throws Error when there is no source record or positioning fails.
  [[nodiscard]] virtual bool SelectRecord(const RecordId &record) = 0;

  /// \brief Reads the current source record's exact table/key identity.
  /// \return Its identity, or an empty identity for a source-less page.
  /// \note A pending new record's key is not proof of a persisted SQL row.
  [[nodiscard]] virtual RecordId CurrentRecord() const = 0;

  /// \brief Opens an ordinary SQL list without first running single-row navigation.
  /// \param mode View/Edit; unqualified new/empty editable rows refuse explicitly.
  /// \param limit Positive trusted server row bound, never supplied by an HTTP client.
  /// \param receiver Captures loaded rows and the selected row synchronously.
  /// \return Row/probe counts and continuation in the forward direction.
  /// \throws Error for unsupported page/providers or opening/loading failures.
  /// \note Implementations without a generated window adapter refuse by default.
  [[nodiscard]] virtual PageWindowState
  OpenWindow(PageOpenMode mode, std::size_t limit, PageWindowReceiver &receiver);

  /// \brief Loads another block, saving pending existing-row edits before leaving it.
  /// \param position First/Last or relative to the previous block's original SQL boundary.
  /// \param limit Trusted server row bound.
  /// \param receiver Captures exact row values after their triggers, then selected state.
  /// \return Bounded rows and continuation in the requested direction.
  /// \throws Error for missing boundaries, unsupported providers or AL failures.
  /// \note No client-side key comparison, implicit Commit or replay of AL opening occurs.
  /// An exhausted relative step presents no new rows and retains the previous block/selection.
  [[nodiscard]] virtual PageWindowState
  ReadWindow(PageWindowPosition position, std::size_t limit, PageWindowReceiver &receiver);
};

/// \brief Creates a closed interactive handle from the same installed page catalogue.
/// \param page The AL page identity.
/// \return An exclusively owned handle; creation does not execute AL opening triggers.
/// \throws Error for absent pages, missing/null factories or declaration mismatches.
/// \note The caller must authorize before Open and every later operation. No session,
/// permission implementation, command receipts or HTTP transport is installed here.
[[nodiscard]] std::unique_ptr<PageInstance> MakeInstalledPage(PageId page);

}
