Type:     task
Status:   open
Parent:   0334
Area:     gen, rt
Source:   developer/devenv-field-groups.md, developer/devenv-lists-as-tiles.md
Verdict:  fehlt
Class:    activation

# A field group names what a dropdown and a tile show

**Two pages, one item**: field groups and the tile view they feed. board:0334 filed `LookupPageId` and
board:0331 the relation; **this is the third piece of the lookup** and neither of them names it.

> "A field group in table or table extension objects **defines the fields to display in a DROP-DOWN
> CONTROL on pages that use the table.**"
>
> ```AL
> fieldgroups
> {
>   fieldgroup(DropDown; Field1, Field2) { }
>   fieldgroup(Brick; Field1, Field2) { }
> }
> ```
>
> **"`<Name>` can be either `DropDown` ... or `Brick` to display data as TILES."**
>
> **"IMPORTANT: The syntax for using a `DropDown` must be spelled `DropDown` with THE RIGHT
> CAPITALIZATION."**
>
> **"The `fieldgroups` keyword CAN'T BE INSERTED BEFORE THE `key` CONTROL."**
>
> In a table extension, **`addlast(<name>; <field>)`** adds more fields. **"The server REMOVES THE
> DUPLICATES if multiple extensions attempt to add the same field more than once. A field can only be
> added to the field group once."**

**Two group names and both are fixed strings** -- `DropDown` and `Brick`. So this is not an open
namespace; it is two well-known groups, which makes them two `constexpr` field spans on `TableDef`.

**The capitalisation note is the odd one.** AL is case-insensitive everywhere else in this sweep --
board:0349's bug came from forgetting that -- and here the documentation says the spelling matters. **A
contradiction with the language's own rule, recorded rather than resolved**: either the compiler special-
cases these two names, or the note is advice rather than a rule. The AL source decides, and the answer
changes whether the generator matches case-sensitively in exactly one place.

**The duplicate removal is a merge rule** -- board:0033 merges extensions at translation time, so
`addlast` from two extensions naming one field must yield one entry. Deduplication in the generator,
not at run time.

## What a field group is FOR

board:0334's `LookupPageId` names the PAGE a lookup opens. **A field group names the COLUMNS the
dropdown shows** when there is no page -- which is the common case, since board:0334 measured 2 294
`LookupPageId` declarations against board:0331's 40 221 relations. **So for 37 927 relations the
dropdown's columns come from here and nowhere else.**

That is the number that makes this item load-bearing rather than cosmetic.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`fieldgroup` is a declaration block, not a `Name = Value` property, so this sweep's pattern does not
count it. **The count of `fieldgroup(DropDown` and `fieldgroup(Brick` declarations belongs to this item
and is its first task** -- stated rather than guessed, and it sizes both the dropdown and the tile
view.

## The IST-state

`include/meta/TableDef.h` -- `TableDef` carries `id`, `name`, `caption`, `fields`, `keys`
(`src/gen/TableWriter.cpp:549`). **No field groups.** `src/gen/TableWriter.cpp` consumes `Caption`,
`OptionCaption`, `OptionMembers` and `InitValue` on a field and `Clustered` on a key -- the
`fieldgroups` block is not among them, so whether the PARSER reads it at all is this item's first
check.

## The choice

Two `constexpr std::span<const FieldNo>` members on `TableDef`, one per group name, emitted by the
generator with extension additions merged and deduplicated.

**Two named members rather than a map**, because there are exactly two group names and a map would
invite a third.

The renderer reads `DropDown` for a lookup's columns (board:0334, board:0336) and `Brick` for
board:0030's tile view.

## Ordering

With board:0331's relation and board:0334's lookup -- the three together are what a dropdown is.
Behind board:0033 for the extension merge.

## Gate, and its negative control

A lookup on a field whose target table declares `fieldgroup(DropDown; No., Name)` shows those two
columns; a table extension adding `Name` again yields two columns, not three.

**The negative control is the duplicate** -- an implementation that concatenates the extension's list
shows `Name` twice, which looks like a rendering glitch and is a merge defect.
