Type:     task
Status:   open
Parent:   0034
Area:     gen, rt
Source:   developer/properties/devenv-horizontalshrink-property.md, developer/properties/devenv-horizontalstretch-property.md, developer/properties/devenv-verticalshrink-property.md, developer/properties/devenv-verticalstretch-property.md, developer/properties/devenv-minimumwidth-property.md, developer/properties/devenv-maximumwidth-property.md, developer/properties/devenv-minimumheight-property.md, developer/properties/devenv-maximumheight-property.md, developer/properties/devenv-requestedwidth-property.md, developer/properties/devenv-requestedheight-property.md
Verdict:  fehlt
Class:    activation

# A control add-in's size is ten properties over two axes

**Ten pages, one item.** They apply to one object kind -- Control Add In -- and they form one sizing
model over two axes, with each page naming the others as its dependency. The documentation's own
example is the whole shape:

```AL
RequestedWidth = 600;
HorizontalShrink = true;
MinimumWidth = 100;
```

> **HorizontalShrink / VerticalShrink** (default **false**): the add-in may be made smaller. **"If
> `HorizontalShrink` is true but `MinimumWidth` is not set, the control add-in can shrink to
> nothing."**
>
> **HorizontalStretch / VerticalStretch** (default false): the add-in may be made larger. **"If
> `VerticalStretch` is true but `MaximumHeight` is not set, the control add-in can expand
> indefinitely."**
>
> **MinimumWidth / MinimumHeight / MaximumWidth / MaximumHeight**: the bounds. **"This setting only
> applies if the [corresponding] setting is specified."** The maximum defaults to the integer's
> maximum value.
>
> **RequestedWidth / RequestedHeight**: the starting size.

So each axis is `{ requested, may shrink, min, may stretch, max }` and a bound without its flag is
dead. **Every one of those relations is decidable from the declarations**, so the whole model
collapses into `static_assert`s plus five numbers per axis.

**And two of the four flags carry a context restriction**: "`VerticalShrink`/`VerticalStretch` is only
supported when the control add-in is displayed on a CardPart on Role Center pages or when it is the
only content displayed on a Card page." That is not decidable from the add-in alone -- it depends on
where it is embedded -- so it is a render-time rule, unlike the rest.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`HorizontalShrink =` **8** · `HorizontalStretch =` **17** · `VerticalShrink =` **8** ·
`VerticalStretch =` **17** · `MinimumWidth =` **6** · `MinimumHeight =` **6** ·
`MaximumWidth =` **0** · `MaximumHeight =` **0** · `RequestedWidth =` **13** ·
`RequestedHeight =` **16**.

**Ninety-one declarations across ten properties, and two of them are zero.** Both maxima are never
declared -- so every stretching add-in in the BaseApp can stretch indefinitely, by the
documentation's own rule. That is worth knowing before implementing a clamp nobody asked for.

## The IST-state

`ControlAddIn` is one of the twelve AL object kinds and has no generator (board:0034), so none of the
ten has anywhere to land.

## The choice

Two `constexpr` axis descriptors on the add-in, `{ requested, min, max, flags }`, with the bounds
folded in by the generator and the dead-bound relations asserted. The renderer maps them onto CSS
`min-`/`max-` constraints.

**The unset maximum is `std::numeric_limits<int>::max()` by the documentation's own words**, not a
`std::optional` -- the page states the default and it is cheaper to hold.

## Ordering

Inside board:0034's control-add-in generator, which does not exist.

## Gate, and its negative control

An add-in declaring the documentation's example renders at 600 wide and does not shrink below 100.

**The negative control is `HorizontalShrink = true` with no `MinimumWidth`** -- it must be able to
shrink to nothing, which is what the documentation says and what an implementation that invented a
sensible floor would prevent.
