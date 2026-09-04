Type:     task
Status:   open
Parent:   0034
Area:     gen, rt
Source:   developer/properties/devenv-rolecenter-property.md, developer/properties/devenv-profiledescription-property.md, developer/properties/devenv-customizations-property.md, developer/properties/devenv-enabled-profile-property.md, developer/properties/devenv-promoted-profile-property.md, developer/properties/devenv-profile-properties.md
Verdict:  fehlt
Class:    activation

# A profile names a role centre and the customizations it applies

**Six pages, one item**: everything a `profile` object declares. `Profile` is one of the twelve AL
object kinds and the documentation's own example declares all of them together, so the object and its
property set are one task.

> **RoleCenter**: the Role Center page for this profile.
>
> **ProfileDescription** (runtime 4.0): the description users see.
>
> **Customizations**: **"the Page Customizations which are applied with this profile."** A
> comma-separated list of page customization object names.
>
> **Enabled** (runtime 4.0, default **true**): whether users can use the profile.
>
> **Promoted** (runtime 4.0, default false): **"whether the profile is available to users in Role
> Explorer. The profile MUST ALSO BE ENABLED."**
>
> `devenv-profile-properties.md` is the object's property index.

**The `Customizations` list is what makes a profile more than a role-centre pointer**: a profile
carries a set of page customizations, so the SAME page renders differently for two users depending on
which profile they signed in with. board:0033 merges customizations at translation time -- and that
merge is per PROFILE, not per app.

**That is a real consequence and it belongs here**: a page's effective layout is a function of
(page, extensions, profile), and if the merge produces one page per profile the count is
pages x profiles.

**`Promoted` needs `Enabled`**, so the conjunction is a `static_assert`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`RoleCenter =` **59** · `Customizations =` **4** · `ProfileDescription =` **55**.

**59 profiles and 4 declared customization lists**, so almost every BaseApp profile is a role-centre
pointer and a caption -- which means the pages x profiles explosion above is not one the BaseApp
actually triggers, and the merge can stay simple until an extension makes it otherwise.

That is the measurement that decides the design, and it is why it is taken before the design.

## The IST-state

`Profile` has no generator (board:0034). board:0033 merges extensions; whether it distinguishes
customizations by profile is not measured, **and that is said rather than assumed.**

## The choice

A profile descriptor with a `PageId`, two `string_view`s and a span of customization ids, resolved by
the generator. The session carries a profile and the renderer reads its customizations.

**Not one generated page per profile** -- 4 declarations do not justify multiplying 6 891 pages. The
customization is applied at render time from a small per-profile delta.

## Ordering

Inside board:0034's profile generator. Behind board:0033 for the customization merge and board:0030
for role-centre rendering.

## Gate, and its negative control

Signing in under a profile applies its customizations and opens its role centre; a profile declaring
`Enabled = false` cannot be selected.

**The negative control is a second profile without the customization** -- the same page must render
unchanged for it, and an implementation that merges customizations globally passes the first gate and
changes the page for everyone.
