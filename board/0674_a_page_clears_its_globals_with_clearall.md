# A page clears its globals with `ClearAll`

**Finding (2026-09-10).** `Phys. Invt. Order Statistics.OnAfterGetRecord` starts with
`ClearAll()`; a codeunit gets a generated `ClearAll()` over its globals, a page did not, so the
call fell to the door's `System.ClearAll()` refusal (6 UT cases of Phys. Invt. Order PAG UT).

**Reference.** `system-clearall-method.md`: clears all internal variables of the object.

**Choice.** The page writer declares and defines `ClearAll()` over the page's globals unless the
page declares a procedure of that name (3 do), the way the codeunit writer does. No gate: the
page-generation gates still have no page fixture (board:0665 names the same debt).
