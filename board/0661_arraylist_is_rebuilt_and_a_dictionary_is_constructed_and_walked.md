# `ArrayList` is rebuilt, and a .NET `Dictionary` is constructed and walked

**Finding (2026-09-10).** `Request Page Parameters Helper` did not compile: `TableList: DotNet
ArrayList` was an absent stub, so `foreach Table in TableList` handed `Refused` values to the
dictionary, and `Dict := Dict.Dictionary(Count)` named a constructor `GenericDictionary2` did not
carry; `foreach KeyValuePair in Dict` had no `begin`. 19 UT cases stopped at "its source is not
in the slice". `ArrayList` is 9 declarations in 4 objects (`Excel Buffer`, `Workflow Management`,
`Custom Layout Reporting` among them), all of `Add`, `Count`, `Contains`, the constructor and a
`foreach`.

**Reference.** .NET `System.Collections.ArrayList` and `Dictionary<K,V>`; the rebuilt-type rules
in this tree (full member set, class-named binder, no user-declared constructor).

**Choice.** `include/dotnet/Generic.h` gains `ArrayList` (a list of Variants, constructed empty or
from anything that iterates over Variants; an absent source refuses when run) and gives
`GenericDictionary2` its `Dictionary` binder, `std::string_view` keys so a Variant key reads as
its text, and `Entry` / `Iterator` with `.Key()` and `.Value()` for a `foreach`. Activation of a
codeunit that never compiled; measured by the milestone.
