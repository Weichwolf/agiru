Type:     task
Status:   open
Parent:   0026
Area:     gen
Verdict:  fehlt
Class:    compile root

# A local that shadows a record method yields to the call

`DefaultDimension` declares a `Text` named `TableCaption` in a procedure and calls
`TableCaption()` in the same body. AL resolves the invocation to the method; the generated C++
resolves the name to the local and `Text<250>` has no call operator (bulk run, 2026-09-06).

## The choice

Where a Call's callee is a bare name that a LOCAL shadows and the door declares as a record
method, the generator spells `this->Name(...)`, which the member function answers regardless of
the local -- the shape `ShadowedByALocal` already uses for a field.

## Gate

A procedure with a local named like a record method calls the method when it writes parentheses
and reads the local when it does not.
