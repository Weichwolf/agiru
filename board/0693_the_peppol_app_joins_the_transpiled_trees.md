# 0693 The PEPPOL app joins the transpiled trees

**The finding.** 14 UT cases in `Incoming Doc. To Data Exch.UT` stop on
`the .NET member PEPPOL30Setup.GetSetup is named by AL and not rebuilt here`. `PEPPOL30Setup` is
not a .NET type at all -- it is a TABLE in `src/Apps/W1/PEPPOL/App`, an app `apps.json` does not
name, so the variable fell through to the absent-type stub. The app is 53 AL files.

**It is business functionality and not a cloud bridge**, which is the line `scope.json` draws:
PEPPOL is how a BC installation exchanges e-documents, the W1 test suite depends on it, and
CLAUDE.md's "THE COMPLETE BC BUSINESS FUNCTIONALITY, NOT A SUBSET" reaches it. So the answer is
that it joins, and this item is what that costs.

**Measured on a copy (2026-09-10), the app transpiles: 2 tables, 22 codeunits, 1 page, 2 enums,
58 files written.** Twelve of its sources compile alone but for two generic gaps, and BOTH are
about a new app rather than about PEPPOL:

- **The shared option types are gathered per app.** `options::OptionSalesService` is missing,
  because `NoteOptions` walks the apps the transpiler knew about; a new app's option variables
  have to reach the same shared table -- the same thing the xmlport round had to do
  (board:0065).
- **A generated file that names a table from ANOTHER app must include its header**, and one
  codeunit here names `Service Line` with only a forward declaration.

**Not done in this round**, because the ranked list has three larger shapes that need a probe
rather than a build, and because adding an app changes every stage of the chain -- transpile,
slice growth, link. The include path of the check script needed the new app too, which is how the
first reading of "the header is not written" turned out to be "the header is not on the path".
