Type: bug
State: open
Area: net, rt

# A `Decimal` formats itself the way AL's `Format` does, locale and all

`Decimal` can be read and written, but only in an invariant notation, and the method is called
`ToInvariantString` for exactly that reason. AL's `ToText` is something else and does not exist yet.

## Reference

**Platform documentation**: `methods-auto/decimal/decimal-totext--method.md` defines
`Decimal.ToText()` as "Equivalent to calling `Format(value, 0, 0)`".
`devenv-format-property.md` then shows what Standard Format 0 is -- and it is neither invariant nor
separator-free:

| region | Format 0 |
|---|---|
| Europe | `-76.543,21` |
| US | `-76,543.21` |

So `ToText` needs a locale AND grouping. And `<Precision,2:3>` in the same document ties the number
of decimal places to a field's `DecimalPlaces` property, which the value type cannot know.

**The choice:** the value type stays locale-free and keeps `ToInvariantString` for round-tripping,
persistence and gate cases. `Format` is a RUNTIME service that takes a locale and a field's
`DecimalPlaces`, and it carries the AL name. Putting it into the value type would give `Decimal` a
dependency on session state, and a value that depends on a session is not a value.

**Why this matters more than it looks:** AL error texts are compared by tests. A number formatted
one separator off makes a message unequal, and the failure looks like a semantic defect rather than
a formatting one.

## What will be true

- [ ] `Format(value, length, formatNumber)` exists on the runtime, with the standard formats 0-4 and
      9 from the documentation's table.
- [ ] The locale comes from the session, the decimal places from the field, and neither from a
      constant.
- [ ] Proof: the documentation's own table -- both regions, all six standard formats, the same
      number -- as gate cases.
- [ ] **Negative control**: swap the thousands separator for the decimal separator and require the
      cases to go red.
