#pragma once

#include <string>
#include <string_view>

/// \file
/// \brief A database of a test runner's own, cloned from the master once.
///
/// \warning TESTS CANNOT SHARE ONE DATABASE, and that is not a matter of taste. The AL suite WRITES
///          to the tables it reads: two runners inserting the same key block each other across
///          transactions, and a number series re-seeds from a shared `MAX` and hands out a key the
///          other one already took. Snapshot isolation settles reads and leaves that untouched, so
///          the only primitive that works is physical write isolation -- one database per runner
///          (openerp WI-832, which measured it, and its counterpart WI-838).
///
/// \warning AND THE GATE MAY NOT USE THE MASTER EITHER. The predecessor ran its C++-equivalent gate
///          and its AL runner against the same database; the runner seeded CRONUS into it and the
///          gate then reported failures belonging to neither -- the same eleven with and without
///          the fix under measurement.

namespace agiru {

/// \brief The database one test runner works in.
///
/// \note IT IS MADE ONCE AND KEPT, and that follows from the isolation rather than from thrift.
///       `TestIsolation = Codeunit` takes every codeunit's writes back at its boundary, so a run
///       leaves the database where it found it and the next run has nothing to restore. Cloning per
///       run costs the master's BYTES, which is 530--643 ms at 73 MB and grows with the CRONUS load
///       it is meant to carry -- paid for a state that was already correct.
///
/// \note IT IS A `TEMPLATE` CLONE AND NEVER A REBUILT SCHEMA. PostgreSQL copies the template's
///       files, so the schema, its indexes and its seed arrive together and cost what the bytes
///       cost. Building the schema statement by statement instead is 1 412 ms against 530--643 ms
///       for the clone (measured 2026-09-03, 1 609 tables, 73 MB, PostgreSQL 17.11).
///
/// \note `STRATEGY` IS LEFT AT ITS DEFAULT ON PURPOSE. `FILE_COPY` reads as the faster route for a
///       large template and is 41 885 ms against `WAL_LOG`'s 530 ms on the same template (measured
///       2026-09-03): it forces two checkpoints, and on two cores those are the whole cost.
class RunnerDatabase {
public:
  /// \brief Finds the runner's database, or clones it from the master.
  ///
  /// \param master The connection string of the template.
  /// \param name   The name the runner's database carries.
  /// \param fresh  Drop what is there and clone again. The cost of a run that inherits a database
  ///               a crash left mid-write is a diagnosis aimed at the wrong tree, so the way back
  ///               is a flag and not a doubt.
  ///
  /// \throws DatabaseError When the maintenance database refuses. Cloning needs a template no
  ///         session holds open, which is why `scripts/pg_seal.sh` keeps the master unconnectable.
  RunnerDatabase(const std::string &master, std::string_view name, bool fresh = false);

  /// \brief Leaves the database where it is.
  ~RunnerDatabase() = default;

  RunnerDatabase(const RunnerDatabase &) = delete;
  RunnerDatabase &operator=(const RunnerDatabase &) = delete;
  RunnerDatabase(RunnerDatabase &&) = delete;
  RunnerDatabase &operator=(RunnerDatabase &&) = delete;

  /// \brief The connection string of the runner's database.
  /// \return The master's, with the database name replaced.
  [[nodiscard]] const std::string &Dsn() const { return dsn_; }

  /// \brief The name the database carries.
  [[nodiscard]] std::string_view Name() const { return name_; }

  /// \brief Whether this call had to clone it.
  /// \return `false` when it was already there, which is the ordinary case after the first run.
  [[nodiscard]] bool Cloned() const { return cloned_; }

  /// \brief Drops it.
  ///
  /// \note NOT WHAT A RUN DOES. A run keeps its database; this is for a caller that made one to
  ///       prove something about it.
  void Drop() const;

private:
  std::string maintenance_;
  std::string name_;
  std::string dsn_;
  bool cloned_ = false;
};

/// \brief The name of a database, so it cannot be passed where a connection string belongs.
///
/// \note TWO `std::string_view` PARAMETERS ARE ONE TRANSPOSITION AWAY FROM A CONNECTION TO THE
///       WRONG DATABASE, and nothing downstream would say so -- the run would simply write its rows
///       somewhere else. The type carries the difference the names alone could not.
struct DatabaseName {
  std::string_view value; ///< The database's name.
};

/// \brief Points a connection string at another database on the same server.
///
/// \param dsn      A libpq connection string, in URI or keyword form.
/// \param database The database to name instead.
/// \return The same string with its database replaced.
///
/// \throws DatabaseError When the string names no database at all, since then there is nothing to
///         replace and guessing which server was meant is worse than refusing.
[[nodiscard]] std::string PointedAt(std::string_view dsn, DatabaseName database);

}
