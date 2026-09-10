# 0682 The door spells the types AL spells, so an overload set resolves

**The finding.** Six BaseApp codeunits cannot be compiled, and 55 UT cases die on
`... is declared and its source is not in the slice (board:0612)` because of it. Four of the six
are door defects of one kind: a door name that is not spelled the way AL spells the TYPE, so the
call cannot resolve.

- **`Session.StartSession(var Integer, Integer [, Text] [, var Record])`** -- the brackets are the
  documentation's own title. The door carried only the three record-carrying overloads, so the
  two-argument form the BaseApp uses most (`StartSession(SessionNo, Codeunit::"...")`) matched
  nothing. 33 cases, all of `UpdateAnalysisView`.
- **`SecretText.SecretStrSubstNo(Text, [SecretText, ...])`** substitutes as many values as
  `StrSubstNo` does; the door took exactly one. AL passes seven.
- **`SecretText.Unwrap()` returns a `Text`** (`methods-auto/secrettext/secrettext-unwrap-method.md`),
  and the door returned a `std::string`, which converts to `Text` and to `SecretText` alike -- so
  an AL overload set declaring both (`GenerateHashAsBase64String`) was ambiguous at every call
  site. A type that is neither of AL's types is worse than the wrong one: it fits both.
- **`Dictionary of [Text, X]` IS `Dictionary<Text<0>, X>`.** Nine door declarations spelled the key
  `std::string`, which is not what the generator writes, so `JsonObject.WriteWithSecretsTo`,
  `ErrorInfo.CustomDimensions` and `TestHttpRequestMessage.QueryParameters` were unreachable from
  any generated body. 223 `Dictionary<Text<0>, Text<0>>` in the tree say which spelling is AL's.

**The rule behind all four.** Name equality with AL is not only about the NAME: a parameter or
return type that is not AL's own type breaks the same check, silently, and it breaks it for every
overload in the set rather than for one call.

**The two that are not this.** `dotnet::Refused` carries a hand-maintained list of 75 chained
member names -- "a list somebody has to remember to fill" -- and `OpenXMLManagement` and
`WebRequestHelper` name members it lacks. The generic answer is that a member reached ON a refused
result is itself refused, which the GENERATOR can see and emit, deleting the list. That is its own
item. `InventoryProfileOffsetting` increments a `Boolean` and holds an incomplete table by value.

**Measured.** Chain 99, A/B against chain 98.

**Beside it, the xmlport round's own three (chain 98's make):** `XMLPORT.Run(Number, ReqWindow,
Import, Record)` reaching a KNOWN xmlport becomes the instance, so the instance carries the
arguments the static form documents (`Run(Boolean, Boolean [, Record])`, `Export(OutStream
[, Record])`, `Import(InStream [, Record])`); `StreamReader` takes a file name, a byte-order-mark
flag and the encoding-plus-flag form; and `EndOfStream` is a .NET PROPERTY, read without
parentheses -- a slot that holds nothing and finds its reader by its own offset, because a class
carrying a member named after itself may declare no constructor and a stored pointer could not
survive `R := R.StreamReader(...)`.
