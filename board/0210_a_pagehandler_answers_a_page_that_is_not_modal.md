Type:     task
Status:   open
Parent:   0054
Area:     rt, gen
Source:   developer/attributes/devenv-pagehandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[PageHandler]` answers a page that is NOT run modally, and returns nothing

```al
[PageHandler]
procedure PageHandler(var Page: TestPage <id>)
```

The difference from board:0206 is the whole reason both exist: a page run with `Run()` returns
nothing, so this handler has no `Action` to set. It sees the page, may drive it, and returns.

The dispatch key is the pair (kind, page id), taken from the handler's parameter TYPE. The two
declaration rules -- `Subtype = Test` codeunit, global method -- are the same and are
`static_assert`s.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**1 659 `[PageHandler` declarations.** Over the milestone's 78 UT codeunits: 31 declarations in 11
codeunits, against 147 `ModalPageHandler` -- so the non-modal case is the minority and the two must
not be collapsed into one kind.

## The IST-state

The attribute parses into the raw list and is dropped. `TestPage` exists as a harness whose bodies
all reach `Unopened()`.

## The choice

The same table as 0206 with kind `Page`. **The kinds stay separate even though the signatures look
alike**: collapsing them would let a `[PageHandler]` answer a modal run and return a default
`Action` the test never wrote, which is a silent pass. The lookup fails when the kind does not
match, and that failure is the point.

## Ordering

Blocked on the page runtime (board:0030), like 0206.

## Gate, and its negative control

Code that runs a page non-modally with a `[PageHandler]` registered: the handler runs and the
caller continues. **The negative control is registering only a `[ModalPageHandler]` for the same
page id and running it non-modally -- it must FAIL**, because the kinds are different keys.
