Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-promoted-property.md, developer/properties/devenv-promoted-action-property.md, developer/properties/devenv-promotedcategory-property.md, developer/properties/devenv-promotedisbig-property.md, developer/properties/devenv-promotedonly-property.md, developer/properties/devenv-promotedactioncategories-property.md
Verdict:  fehlt
Class:    activation

# The legacy promoted syntax is five properties, and `actionref` replaced it

**Six pages, one item**: an overview page and the five properties of the LEGACY promotion mechanism.
They are one mechanism, they are all deprecated together, and each page carries the same replacement
notice.

> **"The `Promoted` property is part of the LEGACY SYNTAX for promoted actions. We recommend that you
> use the `actionref` syntax introduced in 2022 release wave 2."**
>
> **Promoted** (Page Action, default false): whether the action is added to a promoted category.
>
> **PromotedCategory**: `New`, `Process`, `Report`, `Category4` .. `Category12` -- **twelve menus,
> nine of them named only by number.**
>
> **PromotedActionCategories** (Page): the CAPTIONS for those numbered categories --
> `'New caption,Process caption,Report caption,Category4 caption'`.
>
> **PromotedIsBig**: the action appears **before** other promoted actions **regardless of its position
> in the AL code**. "If there is more than one, they appear before the others in the order they are
> defined." **"In the Windows client this property behaves differently: it displays a BIGGER ICON and
> does not reposition the action."**
>
> **PromotedOnly**: the action appears **only** on the Home tab and not on the container where it is
> defined. **"Only applicable when `Promoted` is true."** **"Not relevant on tablet and phone because
> only promoted actions are displayed on these clients."**
>
> **"Removing the `Promoted` property from a published action is a BREAKING CHANGE"** -- AppSourceCop
> AS0031.

**Two facts decide this item.** First, a page's category CAPTIONS are a comma-separated list positioned
against `Category4`..`Category12` -- so the caption for `Category7` is the seventh element of a
string, which is a positional binding across two properties on two objects. Second, `PromotedIsBig`
means two different things in two clients and only the web one applies here.

**And the modern `actionref` syntax is not a property**, so it is not in this family at all -- it is
page syntax, and board:0030's action rendering has to support both because the BaseApp still uses the
old one 1 228 times.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Promoted =` **1 228** · `PromotedCategory =` **983** · `PromotedOnly =` **572** ·
`PromotedIsBig =` **488** · `PromotedActionCategories =` **64**.

**1 228 legacy promotions are still live**, so "deprecated" does not mean "absent". And only 64 pages
caption their categories, so `Category4`..`Category12` render with default names on the rest.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

The five properties resolve in the GENERATOR into board:0425's single placement enumerator plus a
category id and a caption -- so the renderer sees the same descriptor whether the page used the legacy
properties or `actionref`, and the two syntaxes converge before rendering.

The positional caption binding is folded at translation time: `PromotedActionCategories` is split and
indexed against the category enumeration, and a mismatch in length is a `static_assert`.

`PromotedIsBig` is the web meaning -- reposition, not resize.

## Ordering

With board:0425's placement resolution and board:0030's action rendering.

## Gate, and its negative control

An action declaring `Promoted` and `PromotedCategory = Category7` appears in the seventh category with
the seventh caption from the page's list; one declaring `PromotedOnly` appears only there.

**The negative control is the seventh caption** -- an implementation that pairs captions with
categories by name rather than position gets `Category4`..`Category12` wrong for every page that
declares fewer captions than categories.
