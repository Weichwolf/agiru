# A `foreach` over a .NET collection fills the declared loop variable

**Finding (2026-09-10).** `Library - Report Validation` declares `CellData: DotNet CellData` and
walks `foreach CellData in WorksheetReader`; the generator bound a fresh `auto &` to each element,
so the body's `CellData.RowNumber` reached the element's stand-in and not the declared variable's
members -- the unit did not compile and 8 UT cases stopped at "its source is not in the slice".

**Reference.** `devenv-al-control-statements.md`: `foreach` assigns each element to the loop
variable, which is an ordinary declared variable.

**Choice.** Where the loop variable is a declared `DotNet` variable, the generated loop assigns
each element into it (`CellData = Element_Block;`) and the body reads the variable. Gate
`AForeachOverADotNetCollectionFillsTheDeclaredVariable`.
