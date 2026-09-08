Type:     root
Status:   open
Area:     rt, db
Source:   the last of three crashing UT codeunits, 2026-09-08
Class:    activation

# A cursor does not outlive the transaction that opened it

**A `FindSet` LEFT A CURSOR ON A RECORD VARIABLE, THE SESSION MOVED ON, AND THE NEXT `FindSet`
CLOSED IT THROUGH A CONNECTION THAT NO LONGER EXISTED.** The frames say it exactly:

```
Cursor::~Cursor -> Connection::Run -> Connection::Execute -> PQexecParams -> SIGSEGV
  <- detail::Close(OpenCursor *) <- RuntimeFindSet
```

`RuntimeFindSet` calls `state->open.Forget()` first, and forgetting DESTROYS the cursor, whose
destructor issues `CLOSE <name>`. When the connection that opened it is gone, that is a call into
libpq with a dangling `PGconn`.

## What it is really about, and it is board:0012's rule

**"A SESSION'S CONNECTION IS PINNED FOR ITS TRANSACTION."** A server-side cursor is a TRANSACTION's
object -- `DECLARE ... CURSOR` lives until `COMMIT` or `ROLLBACK` and not a statement longer -- so a
cursor held on a record variable across the end of its transaction is already dead in the database.
Closing it afterwards is not merely unsafe, it is meaningless.

**The record variable outlives the transaction and the cursor does not.** A codeunit's global record
survives every test in the codeunit; each test is its own scope. So the state holding an
`OpenCursor` is exactly the case, and it is the ordinary one rather than an edge.

## What is standing, and what it is not

**STANDING: A CURSOR WHOSE CONNECTION IS NOT THE SESSION'S CURRENT ONE CLOSES NOTHING.**
`Cursor::~Cursor` compares its connection against `Session::Current().Database()` and returns when
they differ or no session is current. That turns the crash into nothing at all, and the codeunit
reports again: `ERM Table Fields UT` went from a segmentation fault to `5 of 41`.

**IT IS A GUARD AND NOT THE DESIGN.** Two things are still wrong and both are this item's work:

1. **Nothing forgets the cursor when the TRANSACTION ends**, only when the connection is replaced.
   Inside one connection, a `ROLLBACK` between two tests leaves a record variable holding an
   `OpenCursor` whose server-side cursor is gone -- and the next `Next()` will `FETCH` a name the
   database no longer has. The scope end has to walk the session's open cursors and forget them.
2. **The session does not know what it lent.** There is no registry, so the guard has to ask the
   question backwards -- from the cursor. A list of open cursors on the session, dropped at every
   transaction boundary, is what makes the answer local.

**AND `Connection::Execute` REFUSES A CLOSED HANDLE** rather than handing `nullptr` to libpq. That
was written first and did NOT fix this, which is what proved the connection is not closed but GONE:
the pointer is dangling, not null. It stays, because a moved-from connection is a real state and a
null `PGconn` in `PQexecParams` is a crash rather than an error.

## What proves it

Every UT codeunit reports a line and none exits on a signal. The negative control is the guard
itself: removed, `ERM Table Fields UT` dies in `Cursor::~Cursor` again. When the registry lands, the
control is a test that opens a `FindSet`, ends the transaction, and calls `Next()` -- which must
answer 0 or refuse, and today would FETCH a name the database has forgotten.
