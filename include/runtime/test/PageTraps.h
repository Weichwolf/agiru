#pragma once

#include <cstdint>

/// \file
/// \brief Test-only page trapping; not part of the production control interface.

namespace agiru::detail {

/// \brief AL `TestPage.Trap()`: the next non-modal run of the page lands in this harness.
/// \param page    The page's number.
/// \param harness The `TestPage` that trapped it.
/// \param adopt   Hands the page object to the harness, saying whether the harness owns it now
///                (a page `Page.Run` made) or only drives it (a page VARIABLE's `Run()`, which
///                the variable keeps).
void TrapPage(std::int32_t page,
              void *harness,
              void (*adopt)(void *harness, void *page, bool owned));

/// \brief Gives a page just run to the harness that trapped it, if one did.
/// \param page   The page's number.
/// \param object The page object.
/// \param owned  Whether the harness owns it afterwards; false for a page variable's own object.
/// \return Whether a trap took it.
[[nodiscard]] bool ReleaseTrap(std::int32_t page, void *object, bool owned);

/// \brief Whether a `Trap` is waiting for the page, so a runner can decide what to hand it.
/// \param page The page's number.
/// \return True when the next non-modal run of that page would be taken.
[[nodiscard]] bool TrapPending(std::int32_t page);

/// \brief Forgets every trap; the runner does this between cases.
void ClearTraps();

/// \brief Forgets the traps one harness set, which its destructor does: a trap that outlived its
///        `TestPage` variable adopted a page into freed memory.
/// \param harness The harness.
void WithdrawTraps(const void *harness);

}
