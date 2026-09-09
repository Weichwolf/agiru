Type:     task
Status:   open
Parent:   0014
Area:     gen
Source:   ~/Git/BCApps/src/Layers/W1/Tests/Data Exchange/PaymentExportXMLPortUT.Codeunit.al:646; devenv-al-simple-types (Array)
Verdict:  measured
Class:    silent-wrong-data

# An array parameter takes the shape of the array it is given

`PaymentExportXMLPortUT.CreateDataExchFieldForLine` declares `ExportText: array[10, 10] of
Text[250]` and every caller passes an `array[10, 100]`, then walks the parameter to
`MaxNoOfColumns = ArrayLen(Caller, 2) = 100`. BC runs it; the parameter's declared bounds are NOT
what the callee is bounded by, the argument's are. This tree generates the parameter as
`AlArray<AlArray<Text<250>, 10>, 10>`, converts the argument into that shape on the call, and
refuses at column 11 (`the array index 11 is outside 1..10`, 11 UT cases, 2026-09-09).

## What the references say

- The AL documentation's array page describes the dimensions on the DECLARATION and says nothing
  about a parameter declared narrower than its argument; the BaseApp's own test is the usage and
  it compiles and passes under BC, so a by-value array argument keeps its own shape in the callee.
- `~/Git/openerp`: Python lists carried their length, so the predecessor never met this (WI-1389
  is the `var` ARRAY LENGTH case, a different thing).

## The choice

A procedure with an array parameter becomes a member TEMPLATE over the array's dimensions, so the
callee is instantiated on the caller's `AlArray<AlArray<T, M>, N>` and `At` checks the real bounds.
The declared dimensions stay as the default the header names. The cost is one instantiation per
distinct argument shape, which the population makes small: 0 procedures in the UT codeunits are
called with two different shapes.

## Gate, and its negative control

A helper declared `array[2] of Integer` given an `array[4]` reads element 4. The negative control
is element 5, which refuses -- an implementation that stops checking bounds passes the first and
not the second.
