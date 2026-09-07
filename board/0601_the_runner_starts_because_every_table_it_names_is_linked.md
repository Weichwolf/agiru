Type:     task
Status:   open
Area:     rt, gen, net
Source:   the first start of `agiru run-tests` after the namespace round, 2026-09-07
Class:    activation

**STANDING: 9 of the 13 tables link (2026-09-07). What is left is 4 -- `ICPartner`,
`WorkflowWebhookSubscription`, `ConfigSetup`, `OAuth20Setup` -- and two of them are board:0605's
`std::string_view` parameter, one is an ambiguous interface overload, one a `dotnet::XmlNode`
that will not bind to a reference. `PermissionSetBuffer` is board:0599 and out of the slice.**

# `agiru run-tests` starts, because every table the slice names is linked

**The tree compiles and links, and the runner still does not start.**

```
./build/agiru: symbol lookup error: libagiru_slice.so:
undefined symbol: agiru::Intercompany::Partner::kICPartnerTable
```

`ldd -r build/agiru` names **2 215 undefined symbols**. All but thirteen are FUNCTIONS, which the
loader binds lazily and which therefore cost nothing until a test walks into one. **The thirteen
that stop the start are DATA** -- the `TableDef` constant of a table whose `.cpp` is not in the
slice. A data symbol is bound at load, so one of them is the whole program.

## The thirteen, and why each is out of the slice (measured 2026-09-07)

| table | the first error in its own source |
|---|---|
| `ICPartner` | an interface overload is ambiguous: `Record` against `Text` for the same name |
| `AADApplication` | `platform::User_Table` has no `ApplicationID` |
| `RetentionPolicySetup` | no `TableID` in its own generated class |
| `ADCSUser` | `dotnet::Refused` has no `GetBytes` |
| `WorkflowWebhookSubscription` | `Refused` does not convert to `std::string_view` |
| `ConfigSetup` | `dotnet::XmlNode` will not bind to a non-const reference |
| `ALTestSuite` | the type `TestPermissions` is absent |
| `OAuth20Setup` | no `SetEncrypted` of that shape |
| `CALTestSuite` | no `SetDestination` on the table |
| `AttachmentEntityBuffer` | no `Write` of that shape |
| `MergeDuplicatesBuffer` | a call with three arguments where two are declared |
| `PermissionSetBuffer` | board:0599 -- the name collides and the header path collides with it |

**Six of the thirteen are closed (2026-09-07), and the data symbols stand at 7:**
`RetentionPolicySetup` by folding a record field's spelling to the table's own declaration;
`AADApplication` by `User."Application ID"`, field 13, measured in the restored demo database;
`ADCSUser` by the 73rd entry in `Refused`'s hand-kept member list; `MergeDuplicatesBuffer` and
`AttachmentEntityBuffer` by two door signatures the documentation carries and the door did not --
`RecordRef.Rename(Value1 [, Value2,...])` was capped at two of a primary key's sixteen, and
`OutStream.Write(Value [, Length])` had neither the length nor the `Written` return.

**Eleven causes for thirteen tables**, which is board:0598's classification arriving one table at a
time. Two of them are worth naming as their own mechanism:

- **`Refused` carries its chained members as a hand-kept LIST** of static members
  (`GetString`, `CreateOutStream`, `SelectSingleNode`, ...), filled once from a compiler census.
  `GetBytes` is not in it, and the next one will not be either: this is CLAUDE.md's "a list somebody
  has to remember to fill". The generic shape is for the GENERATOR to emit a call that carries the
  member's NAME when the base is a refusal, so no list can be short.
- **`Refused` refuses to convert to `std::string` and `std::string_view` ON PURPOSE**
  (`include/dotnet/Refused.h`): both would be viable beside the AL type and make the conversion
  ambiguous. Where a door method takes a `std::string_view` rather than an AL type, that exclusion
  turns into a compile error -- so the boundary that takes a standard spelling is the thing to
  change, not the refusal.

## What proves it

`ldd -r build/agiru` names no undefined DATA symbol, and `agiru run-tests --list` prints the
codeunits. The function symbols may stay unresolved until their own sources compile; a test that
walks into one gets a loader error, which is loud and which the run counts as a failure rather than
a crash.
