Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-sessionsettingshandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[SessionSettingsHandler]` answers a requested session update

```al
[SessionSettingsHandler]
procedure SessionSettingsHandler(var SessionSettings: SessionSettings) : Boolean
```

It handles `RequestSessionUpdate` statements: AL asks the client to change the session's settings --
company, profile, language, locale -- and under a test runner this handler stands in for the client,
populates the `SessionSettings` object and returns whether the update is accepted.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**8 `[SessionSettingsHandler` declarations** -- the smallest handler population of the twelve.

## The IST-state

`include/type/SessionSettings.h` exists as a door header with refusing bodies. The attribute parses
into the raw list and is dropped.

## The choice

A table entry with kind `SessionSettings`, no object id.
`SessionSettings.RequestSessionUpdate` consults it, hands the settings object by reference and takes
the Boolean.

**What the handler writes must reach the SESSION** (board:0006), and this is where the two items
meet: a handler that sets the company is asking for board:0060's company switch, with everything
that switch has to reset -- the work date among them. So the handler is cheap and what it triggers
is not, and this item is deliberately ranked behind board:0060 rather than in front of it.

## Ordering

Needs 0199's table and board:0060's company switch for the settings that matter. 8 sites, none in
the milestone.

## Gate, and its negative control

A `RequestSessionUpdate` whose handler sets a language and returns `true`: the session must carry it
afterwards. **The negative control is a handler returning `false`** -- the session must be unchanged,
which a runtime that applies the settings before reading the return value gets wrong.
