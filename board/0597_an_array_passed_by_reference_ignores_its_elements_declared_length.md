Type:     task
Status:   open
Parent:   0085
Area:     gen, net
Source:   measured over the generated tree, 2026-09-07
Verdict:  offen
Class:    silent-wrong-data

# An array passed by reference ignores its element's declared length, because AL does

`MatrixManagement.Codeunit.al:396` declares

```al
procedure GeneratePeriodMatrixData(...; var CaptionSet: array[32] of Text[80]; ...)
```

and `BankCatPostedPayableBills.Page.al:150` calls it with

```al
MatrixColumnCaptions: array[32] of Text[1024];
```

**AL compiles that and BC ships it.** The DIMENSION matches and the element's declared LENGTH does
not, and the AL compiler does not object -- so the length of an array's element is not part of the
type AL checks at a `var` parameter, whatever it is at an assignment.

C++ does object: `AlArray<Text<80>, 0> &` cannot bind to `AlArray<Text<1024>, 32>`, and the
diagnostic is `non-const lvalue reference to type ... cannot bind to a value of unrelated type`.

## Population, measured 2026-09-07

**4 of a 1 483-source sweep**, so on the order of 16 over the whole generated tree. It is a small
population and a real one, and every one of them is a body that cannot compile.

## The choice, and it is not made

- **Erase the element length from the array's TYPE and carry it at run time.** `array[32] of
  Text[1024]` becomes `AlArray<Text<0>, 32>` whose elements know their cap. Every text array of one
  dimension is then one type and the reference binds. It costs `Text<0>` a runtime maximum, which is
  a change to `StringValue` and reaches every text in the tree.
- **Erase the DIMENSION too**, which the door already does on the parameter side (`AlArray<E, 0>`),
  and let the array carry both bounds at run time. Same cost, wider.
- **Leave it and refuse.** 16 bodies stay dirty, and the finding is that AL's own type rule is
  looser than the one this tree derived from it.

**What decides it is a measurement nobody has made**: whether AL TRUNCATES on the callee's shorter
element or ignores the declaration entirely. `array-data-type.md` and the `var` parameter page are
the sources; the predecessor's board is worth a grep before anything is built, because a dictionary
of descriptors would have hidden this defect rather than raising it.
