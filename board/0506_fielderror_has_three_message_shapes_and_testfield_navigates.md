Type:     task
Status:   open
Parent:   0055
Area:     rt
Source:   developer/devenv-calcfields-calcsums-fielderror-fieldname-init-testfield-and-validate-methods.md
Verdict:  teilweise
Class:    silent-wrong-data

# `FieldError` has three message shapes, and one `TestField` overload navigates

**This page is split into two items** -- board:0507 takes `CalcFields` and `CalcSums` -- because the
seven methods it covers are two unrelated subjects and one WI is one theme. This half is the error
wording, which is board:0055's.

## The three `FieldError` messages, verbatim

> - a text or code field with a **non-matching, non-empty** value:
>   **`Class must not be OTHER in Item No. ='70000'.`**
> - a text or code field that **contains the empty string**:
>   **`You must specify Class in Item No.='70000'.`**
> - a **numeric** field that is empty -- **"it's treated as if it contains the value 0"** --
>   **`Class must not be 0 in Item No.='70000'.`**
> - with a custom text, `Item.FieldError(Class,'must be greater than 4711')`:
>   **`Class must be greater than 4711 in Item No.='70000'.`**

**Four shapes from one method, and the empty case is a DIFFERENT SENTENCE** -- not "must not be blank"
but "You must specify". A test comparing error text sees the difference.

**The numeric empty case is a rule, not a rendering**: an empty Integer is 0, so `FieldError` on it
says `must not be 0`, never `must not be blank`. That follows from AL having no null, and it is worth
writing down because a C++ implementation with `std::optional` anywhere in a field would produce the
other sentence.

**One unresolved detail, recorded rather than smoothed over**: the first example renders
`in Item No. ='70000'` with a space before the `=` and the other three render `in Item No.='70000'`
without one. Two spellings of one clause on one page. A test comparing text needs the right one, and
the AL source or a running BC decides -- **not this item, and not by picking the prettier.**

## `FieldName` exists so a message survives a rename

> "You could just use the name of the field. However, using `FieldName` lets you create messages that
> always contain the name of the field, **even if the name of the field is changed.**"
>
> `FieldError(Quantity,'must not be less than ' + FieldName("Quantity Shipped"));`

So `FieldName` returns the field's NAME, and board:0382's `Caption` is a different string -- the page
does not say which `FieldError` itself uses, and its examples show the name. That belongs in
board:0055's wording work.

## `TestField` discards the record's changes, and one overload navigates

> "If the test fails ... an error message is displayed and a run-time error is triggered. **This means
> that ANY CHANGES THAT WERE MADE TO THE RECORD ARE DISCARDED.**"
>
> **"If the value you test against is an empty string, the field must have a value other than blank or
> 0."**
>
> | overload | dialog |
> |---|---|
> | `TestField(Field)` | **displays a "Show [Record]" button that navigates to the CARD PAGE of the related record. This automatic navigation is BUILT INTO THE PLATFORM and doesn't require `ErrorInfo`** |
> | `TestField(Field, Value)` | **only an OK button. No automatic navigation.** |
>
> **"The platform resolves the navigation target by finding a CARD-TYPE PAGE whose `SourceTable`
> MATCHES the record's table."**

**Two overloads, two dialogs** -- CLAUDE.md's warning about overload filenames arriving in a concept
page: behaviour hangs off the argument count.

**And the navigation target is RESOLVED FROM METADATA** -- the first `PageType = Card` page whose
`SourceTable` is this table. board:0429 puts the page type in the metadata and board:0431 the source
table, so the lookup is a `constexpr` map the generator builds: one card page per table, resolved at
translation time, no search at run time. **That is a runtime that knows no AL object doing an
AL-object lookup correctly**, which is the distinction CLAUDE.md draws.

> "The automatic navigation **doesn't highlight the specific field**. If you want field highlighting,
> a custom button caption, or navigation to a non-default page, use one of the overloads that accepts
> an `ErrorInfo`."

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Method calls; board:0028 owns the census. **Stated rather than guessed.**

## The IST-state

`src/rt/Record.cpp:128` -- `TestField` exists (board:0319 cites it). `include/runtime/Table.h` carries
`FieldError`. **Whether the three message shapes are produced, and whether the one-argument overload
navigates, are this item's first checks and are not measured here** -- hence `teilweise`.

## The choice

Three message templates as declared labels (CLAUDE.md: "a diagnostic is a declared label, never a free
literal"), selected by field type and emptiness. The card-page lookup is a `constexpr` map from
`TableId` to `PageId`, emitted from board:0429's page types.

**`TestField`'s discard is the transaction's**, not a manual undo: the error propagates and whatever
restores the before-image on a failed `Validate` (`include/runtime/Table.h:1373`) does the same here.

## Ordering

Inside board:0055. The card-page map needs board:0429 and board:0431; the three messages need nothing.

## Gate, and its negative control

`FieldError` on an empty Code field produces `You must specify ...`; on an empty Integer field
`... must not be 0 ...`; on a non-empty mismatched Code field `... must not be OTHER ...`.

**The negative control is the empty INTEGER** -- an implementation with one "is blank" branch produces
the "You must specify" sentence for it, which is the right sentence for the wrong type and passes any
gate that only tests text fields.
