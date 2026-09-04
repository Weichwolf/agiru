Type:     task
Status:   open
Parent:   0482
Area:     gen, rt
Source:   developer/devenv-profile-object.md, developer/devenv-page-customization-object.md, developer/devenv-profile-ext-object.md, developer/devenv-design-profiles.md, developer/devenv-role-customization.md, developer/devenv-assign-user-profile.md
Verdict:  fehlt
Class:    activation

# A page customization is layout and actions, and nothing executable

**Six pages, one item**: the profile object, the page customization it points at, the profile extension
and the three pages about designing and assigning profiles. board:0482 filed the profile's six
properties; **this is what a `Customizations` entry actually contains**, and it is narrower than a page
extension.

## What a page customization may and may not contain

> "The page customization object ... allows you to add changes to the **layout and actions** on a page
> ... **you CAN'T ADD VARIABLES, PROCEDURES, OR TRIGGERS.** You can add **actions, fields, and
> groups**."

**Three additions, three prohibitions, all decidable from the declaration.** So a page customization is
pure DATA -- board:0537's control tree with a delta applied -- and nothing in it compiles to code.

**That is what makes board:0482's design work**: a per-profile delta applied at render time costs
nothing at run time, because there is no code to bind. A page EXTENSION, which may add procedures and
triggers, cannot be applied that way and must be merged at translation time (board:0033).

**Two mechanisms, two merge points, and the reason is in this restriction.**

> **"A single page customization can be USED WITH MULTIPLE PROFILES within the same extension."**
>
> **"Page customizations ONLY APPLY TO THE ROLE CENTERS they're specified for."**

**So a customization is scoped to a role centre, not to the page it customises** -- the same
`pagecustomization MyCustomization customizes "Customer List"` applies to the Customer List **only when
the user's profile has that role centre**. board:0482 measured `Customizations` at **4** and
`RoleCenter` at **59**, so in the BaseApp this is nearly unused, and board:0482's conclusion -- that
the pages × profiles explosion is not triggered -- holds.

## The profile object validates its references

> "The Profile object **performs a VALIDATION to check whether the specified role center page EXISTS,
> and page customization objects EXIST**, when you define a new profile object."

**Two `static_assert`s named by the platform itself**: a `RoleCenter` naming a page that does not exist,
and a `Customizations` entry naming a customization that does not. Both are declarations.

**And a `RoleCenter` naming a page whose `PageType` is not `RoleCenter` is a third**, which follows from
board:0429 and which the page does not state.

## Two restrictions the page repeats from elsewhere

> **"Extension objects can have a name with a MAXIMUM LENGTH OF 30 CHARACTERS."** -- board:0545 read the
> same on the table-extension page.
>
> **"Modifying actions in Cue groups on page extensions ISN'T SUPPORTED."** -- board:0538 read the same
> on the cues page.

**Both are stated on at least two pages**, which is worth recording: a rule repeated across pages is
more likely to be load-bearing than one stated once, and both are `static_assert`s.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0482: `RoleCenter` **59**, `Customizations` **4**, `ProfileDescription` **55**.

**The `profile` and `pagecustomization` object counts are declarations, not `Name = Value` properties**
-- stated rather than guessed. 59 role centres and 4 customization lists is the shape: profiles in the
BaseApp are role-centre pointers.

## The IST-state

`Profile` has no generator (board:0034); `src/gen/PageWriter.cpp` consumes `SourceTable` alone.
board:0033 merges extensions; **whether page customizations are distinguished from page extensions
there is board:0482's open question** and this item's first check.

## The choice

A page customization becomes a `constexpr` DELTA -- a list of `{ target control, operation, value }`
entries with no code -- attached to the profile and applied by board:0537's renderer over the merged
page.

**Applied at render time, not merged at translation time**, because it is data and because it is
per profile: merging would multiply 6 891 pages by 59 profiles.

Page extensions stay in board:0033's translation-time merge, because they carry code.

The three validations become `static_assert`s.

## Ordering

Behind board:0537's control tree and board:0482's profile object. With board:0542's views, which a page
customization may also declare and which are the same kind of delta.

## Gate, and its negative control

A user signed in under a profile sees its customization applied to the Customer List; a second user
under a different profile sees the same page unchanged; a profile naming a non-existent customization
fails to transpile.

**The negative control is the second user** -- an implementation that merges customizations at
translation time changes the page for everyone, and a single-profile gate cannot see it.

## A PROFILE ROUND-TRIPS THROUGH THE CLIENT

`devenv-design-profiles-using-client.md` (read 2026-09-04, routed here): a profile and its page
customizations are DESIGNED in the client, exported as a package, and imported into an AL project --
so the `pagecustomization` objects this item describes are usually generated by the client rather than
written by hand.

That changes nothing about what they contain, and it explains their shape: a customization exported
from the client is a layout delta and nothing else, because the client has no way to author a
variable, a procedure or a trigger. **The restriction this item quotes is not only a rule, it is what
the editor can produce.**
