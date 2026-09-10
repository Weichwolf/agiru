# A cursor from a rolled-back boundary is not closed later

**Finding (2026-09-10).** `Incoming Doc. To Data Exch.UT` lost 11 cases to "current transaction
is aborted": a test opened a cursor (`FindSet`) inside its boundary, the boundary rolled back on
an error, the next test's boundary opened at the same depth, and the record variable that still
held the cursor was destroyed there -- its `CLOSE` failed with "cursor does not exist" and
PostgreSQL aborted the boundary, so the next `DECLARE` failed with it.

**Reference.** PostgreSQL: `ROLLBACK TO SAVEPOINT` destroys all cursors created after the
savepoint. The predecessor's WI-858 names the same trap from the other side (a swallowed error
without a savepoint leaves the transaction aborted).

**Choice.** `Boundaries` counts its rollbacks; a `Cursor` records the count it was declared under
and skips its `CLOSE` when the count has moved (`src/rt/Cursor.cpp`, gate case
`ACursorFromARolledBackBoundaryIsNotClosedLater`). A cursor declared BEFORE the savepoint
survives the rollback in PostgreSQL and is then left to the transaction's end -- a leak of a
cursor name until commit, never a wrong answer. Silent-wrong-data turned from an aborted
transaction into a running one.
