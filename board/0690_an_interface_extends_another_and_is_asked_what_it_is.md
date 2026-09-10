# 0690 An interface extends another, and is asked what it is

**The finding.** `Inventory Adjustment Handler` -- 25 UT cases -- did not compile, and neither did
12 other sites, because AL's `is` and `as` on an interface variable reached the generator
untranslated: `if InventoryAdjustment is "Cost Adjustment With Params" then ... := InventoryAdjustment
as "Cost Adjustment With Params"`. Four rules were missing behind that one line.

- **`is` and `as` are OPERATORS.** The parser knew `in` at that precedence and not these two, so
  the expression came out as juxtaposed names. They are now read as operators and written as
  `Handle.Is<Face>()` and `Handle.As<Face>()`.
- **`as` yields a REFERENCE and not a copy.** `Implementation<J>::Borrowed` refers to the instance
  the first variable owns, so a call through either reaches the same object -- which is what AL's
  `as` means and what the BaseApp relies on when it sets properties through the narrower
  interface and then runs through the wider one.
- **AN INTERFACE MAY EXTEND ANOTHER** (11 do). The parser read the names after `extends` and threw
  them away, so the narrower interface carried none of the wider one's methods.
- **The inheritance is VIRTUAL, in both places.** A codeunit's `implements` list names both the
  wider and the narrower interface -- BC's own `Inventory Adjustment` codeunit does -- and with
  ordinary inheritance the wider one is then a base twice, which `-Winaccessible-base` refuses.
  Virtual bases also settle the diamond before it is drawn.
- **AL overloads ACROSS the extension**: `MakeMultiLevelAdjmt(var CostAdjustmentParamsMgt)` beside
  the base's `MakeMultiLevelAdjmt()`. In C++ the derived declaration hides the base's, so the
  generated class brings it back with `using Base::Name;`.

**Beside it: every table carries the system fields, the platform's own included**
(`devenv-table-system-fields.md`). Five of the twenty platform tables declared them and fifteen
did not, so `User.SystemId` -- what the Email module relates a message to -- was not a member.

**Measured.** Chain 112, A/B against chain 111's 1 647.

**The virtual bases cost one thing and chain 112 named it, 240 times:** an enum's implementation
map clones the codeunit behind an enumerator, and it went from the interface pointer to the
codeunit with `static_cast` -- which C++ refuses through a VIRTUAL base. `dynamic_cast` is the
one that works there, and it cannot fail: the switch has already picked the implementation by its
enumerator. That is the whole price of virtual inheritance here, paid once in the generator.

**Beside it: .NET `System.IO.File` is rebuilt** (`Exists`, `Delete`, `Copy`, `Move`,
`ReadAllText`, `WriteAllText`, `AppendAllText`, `ReadAllLines`), which is how a test says "the
export was written" -- 12 UT cases stop on `File.Exists` alone. It is the STATIC class beside
AL's own `File`, which opens one file rather than asking about one, and `ReadAllLines` hands back
a `dotnet::Array` because that is what AL declares for what it assigns.

**And the same cast lived in the handle itself.** `Implementation<I>` carried
`static_cast<const C *>(held)` in its clone and `static_cast<C *>(held)` in its free -- both cross
the now-virtual base, so every interface variable made from a codeunit stopped compiling (chain
113, 44 errors). Both are `dynamic_cast` now, and neither can fail: the pointer was made from a
`C` two lines above.
