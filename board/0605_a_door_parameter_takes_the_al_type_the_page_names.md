Type:     task
Status:   open
Area:     net, rt
Source:   three of the four tables that still stop the runner, 2026-09-07
Class:    silent-wrong-data

**STANDING: measured, not started. 433 parameters; it closes 2 of the 4 tables in board:0601.**

# A door parameter takes the AL type its page names, not the standard spelling underneath it

**433 door parameters are spelled `std::string_view` where the documentation says `Text`**, and that
is not a detail of style: it decides what AL code may pass.

```
filterpagebuilder-addtable-method.md:  [Name := ] FilterPageBuilder.AddTable(Name: Text, TableNo: Integer)
include/type/FilterPageBuilder.h:      std::string AddTable(std::string_view Name, ::agiru::Integer TableNo)
```

`Text` and `Code` reach `std::string_view` through their own conversion, so the majority compiles
and the deviation stays invisible. It becomes visible where AL's own conversions do not survive it:

| the table that stops | what AL passes | what the door will not take |
|---|---|---|
| `WorkflowWebhookSubscription` | `TableMetadata.Caption()`, a refused .NET member | `Refused` excludes `std::string` and `std::string_view` BY DESIGN, so the value that refuses everywhere refuses here too |
| `OAuth20Setup` | a `Guid` to `IsolatedStorage.SetEncrypted(Key: String, ...)` | `Guid` has no conversion to a standard string, and needs none in AL |

**And the Guid case is documented, not inferred.** `methods-auto/guid/guid-data-type.md`:

> The GUID data type is compatible with the existing textual representation. **You can assign and
> compare the Text data type and the GUID data type.**

So AL passing a `Guid` at a `Text` parameter is the language working as specified, and the door
refusing it is the door being wrong.

## The choice

**MEASURED 2026-09-07, and it moves the choice**: `StringValue` carries no constructor from a
literal or a `std::string_view` -- it has `explicit StringValue(std::size_t max)` and the defaulted
ones, because it is the BASE of `Text<N>` and `Code<N>` and a length is part of it. A parameter
typed `const StringValue &` would therefore take `Text` and `Code` and refuse `"Name"`, which is
what a great many call sites pass. So this is not a 433-site type swap; it needs a parameter type
that accepts a literal, an AL string, a `Guid` and a refusal, and that type does not exist yet.

**~~The parameter takes `const StringValue &`~~**, which is what `Text` and `Code` already are and what
a temporary can bind to. Then `Guid` gains the conversion the page above describes, and `Refused`
keeps its deliberate exclusion of the STANDARD spellings while still converting to the AL type --
which is what its own `\warning` says it wants (`Code<50> = Obj.Member` had two viable conversions
and that is why the standard ones were excluded).

**Not `std::string_view` with more overloads**: every AL type that reaches a text parameter would
need one at each of the 433 sites, and the count is the argument.

## What it costs and what it buys

433 parameters across `include/type/` and `include/runtime/`, mechanical, one type. It closes two of
the four tables that stop `agiru run-tests` from starting, and it removes the class rather than the
two cases -- board:0603 named the same thing from the other side, where an `Integer` is compared
against a `std::size_t` because the door carried the standard type there too.
