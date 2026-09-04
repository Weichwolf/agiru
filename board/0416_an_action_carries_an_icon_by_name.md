Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-image-property.md, developer/properties/devenv-images-property.md
Verdict:  fehlt
Class:    activation

# An action carries an icon, named from a closed platform list

**Two pages, one item**: `Image` names one icon, `Images` names several for the same element, and
both resolve against the same platform icon list.

> **Image**: Specifies the icon that you want to associate with a field in a CueGroup control.
> Applies to: **Page Field, Page Action, Page Action Group, Page File Upload Action.**
> `Image = Report;`
>
> **NOTE: You can only use images on FIELDS that have an integer data type.**
>
> On **RoleCenter** pages, the property doesn't apply to actions in the navigation bar or top-level
> actions in the action bar -- **these can't be assigned an icon**. The property only applies to
> subgroups and child actions.

**The name is from a closed list** -- BC ships a fixed icon set -- so `Image = Report` is a member of
an enumeration the platform owns, not a file name. That makes it a `static_assert` candidate: an
unknown icon name is a translation error, provided the list is available.

**The list is the open question.** The page links to an external icon reference rather than
enumerating it, so the set has to come from somewhere -- the AL symbol package, or the measured set
of names actually used. Measuring 46 008 declarations gives the used set, which is a lower bound and
not the list.

Two rules that are decidable and therefore assertions: an `Image` on a non-Integer field, and an
`Image` on a role-centre top-level action.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Image =` **46 008** · `Images =` **6**.

**46 008 is the fourth-largest population in this sweep**, after `Caption`, `ApplicationArea` and
`ToolTip` -- nearly every action in the BaseApp names an icon. `Images` is six.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

An interned icon id on the element descriptor -- **not a string**, at 46 008 declarations -- with the
generator resolving the name against the icon list and failing on an unknown one. The renderer maps
the id onto whatever the icon set is here.

**The icon ASSETS are a separate question** and this item does not decide it: what it delivers is that
the declaration survives translation and is checkable.

## Ordering

With board:0030's action metadata. The icon list has to be obtained first, and where from is this
item's first task.

## Gate, and its negative control

An action declaring `Image = Report` renders that icon; an unknown name fails to transpile.

**The negative control is the unknown name** -- an implementation that carries the string through
renders nothing for a typo and nobody finds out, which is what 46 008 silent misses look like.
