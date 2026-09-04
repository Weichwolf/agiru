Type:     task
Status:   open
Parent:   0083
Area:     gen, rt
Source:   developer/properties/devenv-additionalsearchterms-property.md
Verdict:  fehlt
Class:    activation

# `AdditionalSearchTerms` widens what Tell Me finds

> **Version**: runtime 3.0. Applies to: **Page, Report.**
>
> Specifies search terms (words and phrases) for the page. **In addition to the page caption**, the
> terms are used by the **search feature** in the Web client and mobile apps. **Separate terms with a
> comma.**
>
> Parameters: **Locked** (Boolean) -- if true the value is locked and should not be translated;
> **Comment** (Text); **MaxLength** (Integer).

**The parameter list is the shape to notice.** This is not a bare string: AL string properties carry
optional named arguments -- `Locked`, `Comment`, `MaxLength` -- and the same three appear on `Caption`
and its relatives. A parser that read the value up to the semicolon would take the arguments as part
of the text.

That is a fact about the whole caption family and not only this property, so it belongs recorded
here where the documentation states it explicitly: **the value ends at the first comma that separates
it from a named argument, and `Locked`, `Comment` and `MaxLength` are not content.**

board:0083 owns Tell Me and this property is one of its two inputs; the other is the page caption
(board:0382).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AdditionalSearchTerms =` **665** · `AdditionalSearchTermsML =` **0** (board:0386).

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; board:0083 records that the object catalogue
has no search over it.

## The choice

A `constexpr` span of `string_view` -- **split at translation time**, so the runtime never parses a
comma-separated list, and the terms sit beside the caption in the same searchable array board:0083
needs.

`Locked` and `Comment` are translation metadata and are dropped by the generator, not carried.
`MaxLength` is a declaration the transpiler can check: a term list longer than it is a translation
error.

## Ordering

Behind board:0083's catalogue search. The metadata half with board:0030.

## Gate, and its negative control

Tell Me finds a page by a term that is not in its caption.

**The negative control is a page with no terms** -- it must still be found by its caption, or the
search has become a term lookup and 2 717 pages without the property disappear from it.
