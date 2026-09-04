Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-hyperlinkhandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[HyperlinkHandler]` captures the URL instead of opening it

```al
[HyperlinkHandler]
procedure HyperlinkHandler(Message: Text[1024])
```

`Hyperlink(Url)` would ask the client to open a link. Under a test runner the handler receives the
URL as text and the link is never followed -- which is what makes a suite that touches
`Hyperlink` deterministic and offline.

**Two rules the page states and the generator can check at translation time:**

- "The **HyperlinkHandler** attribute can only be set inside codeunits with the **SubType** property
  set to **Test**."
- "The above signature requires the HyperlinkHandler method to be **global**."

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**24 `[HyperlinkHandler` declarations.** Over the milestone's 78 UT codeunits: 1 declaration in 1
codeunit. It is the smallest handler population and the cheapest to finish.

## The IST-state

`Hyperlink` is a door refusal; the attribute parses into the raw list and is dropped.

## The choice

One entry in the codeunit's `constexpr` handler table, kind `Hyperlink`, no object id -- the same
mechanism as 0194 and 0205. `Hyperlink(Url)` consults it by kind and calls the handler with the URL.
**With no handler and a runner active it FAILS**, because the predecessor made an unhandled
`Hyperlink` a no-op and the test then failed further downstream with "Queue underflow" instead of at
the call (openerp `_system.py`).

The two declaration rules become `static_assert`s beside the generated codeunit: the attribute on a
non-`Test` codeunit, or on a `local procedure`, is a translation error naming the procedure.

## Ordering

Needs 0199's table. Needs no page runtime. With 24 sites and one milestone codeunit it is the
smallest slice of board:0054 that can be closed on its own.

## Gate, and its negative control

A test whose code calls `Hyperlink` and whose handler asserts the URL. **The negative control is the
same test with the handler removed: it must fail AT the `Hyperlink` call** -- a no-op passes the
first case and hides every later assertion.
