Type:     task
Status:   open
Parent:   0061
Area:     rt, gen
Source:   developer/devenv-handling-errors-using-try-methods.md
Verdict:  fehlt
Class:    activation

# A `[TryFunction]` catches only when its return value is used

board:0061 is "a `TryFunction` contains its error". **This page is the specification**, and its central
rule is the value-context failure mode CLAUDE.md inherited -- here in its purest and most consequential
form.

> **"If a try method call DOESN'T USE the return value, the try method OPERATES LIKE AN ORDINARY
> METHOD, and errors are exposed as usual."**
>
> **"If a try method call USES the return value in an `OK :=` statement or a conditional statement such
> as `if-then`, ERRORS ARE CAUGHT."**
>
> ```AL
> [TryFunction] procedure DoWork() begin Error(''); end;
>
> DoWork();               // Fails -- the call ISN'T a try function
> Result := DoWork();     // Will not fail, returns false
> if DoWork() then        // Will not fail, returns false
> ```

**So the SAME procedure is a try function or not depending on the CALL SITE.** The attribute does not
make it one; using the return value does. That is not a C++ shape at all -- a function either has a
`try` in it or does not -- so the generator must emit the decision AT THE CALL SITE, which means it
must know the value context of every call to every `[TryFunction]`.

**And the contexts are exactly CLAUDE.md's named list**: assignment, `if`/`while`, `exit`, argument,
`case` selector. This page names two of them explicitly and the guard already exists.

## The arguments are evaluated INSIDE the try function

> **"Arguments for a try function are EVALUATED INSIDE the try function."**
>
> ```AL
> DoWork(DoInsert(Rec));            // Allowed -- DoWork's return value isn't used, so not a try function
> Result := DoWork(DoInsert(Rec));  // NOT allowed -- DoInsert calls Insert and is evaluated INSIDE
> If DoWork(DoInsert(Rec)) then     // Not allowed as above
> ```

**Argument evaluation moves inside the protected region**, which is the opposite of C++'s order: in
C++ the arguments are evaluated at the call site, before the callee's `try` block exists. So a
generated try call cannot be `if (DoWork(DoInsert(rec)))` with a `try` inside `DoWork` -- the `try`
has to wrap the whole expression including its arguments.

**That is a real code-shape consequence and it is the item.** The generator emits, at the call site,
something like a lambda wrapping argument evaluation and the call, inside one `try`.

## Write transactions inside a try function

> **"Changes to the database that are made with a try method AREN'T ROLLED BACK."**
>
> "Because of that, **you shouldn't include database write transactions within a try method.** For BC
> ONLINE there are no restrictions. For BC **ON-PREMISES, the server PREVENTS database write
> transactions within try methods BY DEFAULT.** If a try method contains a write, **a runtime error
> occurs.** You can allow them by setting `DisableWriteInsideTryFunctions` to `false`."

**Two different behaviours, and agiru is on-premises**, so the default is: **a write inside a try
function raises.** That is the stricter and safer of the two, it matches the deployment, and it is what
this item takes -- with the configuration switch named and not implemented until something needs it.

**And "changes aren't rolled back" is the reason the restriction exists**: a try function that wrote
and then failed leaves the write behind, and CLAUDE.md's first invariant makes that unacceptable
inside a posting. board:0514's isolated events reach the same place from the other side and answer it
with savepoints; here BC's own answer is to forbid the write.

## Two more restrictions

> **"A try method CAN'T HAVE A USER-DEFINED RETURN VALUE."** Its return is the Boolean.
>
> **"The return value ISN'T ACCESSIBLE WITHIN the try method itself."**

Both are `static_assert`s: a `[TryFunction]` with a declared return type, or one that assigns to its
own return variable, is a translation error.

## Getting the error afterwards

> `GetLastErrorText` for the message; `GetLastErrorObject` **"to inspect the `Exception.InnerException`
> property"** for platform and .NET exceptions.

`src/rt/Transaction.cpp:84` -- `ClearLastError` exists, and `include/runtime/Error.h:67` declares it.
So there is already a last-error slot; whether `GetLastErrorText` fills it is board:0061's check.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0207's attribute family owns `[TryFunction]`'s count. **Stated rather than guessed** -- and the
number that actually sizes this item is the count of CALL SITES in a value context, which the
generator can produce once it resolves them.

## The IST-state

board:0061 records it. `src/rt/Transaction.cpp:84` and `include/runtime/Error.h:67` have the last-error
surface. `src/al/Parser.cpp:926`'s `HasAttribute` can see `TryFunction`; whether `BodyWriter` acts on it
is board:0061's check.

## The choice

**The `try` is emitted at the call site, not in the callee**, wrapping argument evaluation and the
call. A `[TryFunction]` compiles to an ordinary procedure returning `void`; the call site decides.

A write inside an active try scope raises -- one flag on the session, checked by `RuntimeInsert`,
`RuntimeModify`, `RuntimeDelete`.

The two restrictions are `static_assert`s.

## Ordering

board:0061's core. Behind the generator's value-context resolution, which CLAUDE.md already names as
the guard for this failure mode.

## Gate, and its negative control

`Result := DoWork()` returns false and continues; a bare `DoWork()` propagates the error;
`Result := DoWork(DoInsert(Rec))` raises because the insert is inside the try.

**The negative control is the bare call** -- an implementation that puts the `try` inside the callee
swallows the error there too, so the procedure never fails and the caller carries on. That is the
worst failure shape in this tree: an error that was raised, caught by nobody's request, and discarded.
