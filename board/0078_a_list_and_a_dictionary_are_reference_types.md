Type: root
State: open
Area: net, gen
Tags: navision, semantics

# AL's collection, HTTP, JSON, XML and TextBuilder types are REFERENCE types, and two variables that name one share it

Both pages say it in the same sentence, and it is the only sentence in either that changes what the
C++ must be:

> A List is a **reference type**, so assigning an instance of a list to another variable or passing
> as a method parameter by value (for example without `var`), creates a second variable that
> **reads/writes the same list**. *It does not create a new list.*
> -- `list-data-type.md`, and `dictionary-data-type.md` word for word for a dictionary

`include/type/List.h:197` holds `std::vector<T> values_` and `include/type/Dictionary.h` holds its
entries the same way. **Both copy on assignment and on a by-value parameter.** So AL that hands a
list to a procedure and expects the callee's `Add` to be visible afterwards gets nothing back, and
nothing raises.

## Why it is the dangerous kind

The two shapes AL writes are ordinary and neither is exotic:

```al
procedure Collect(Names: List of [Text])      // no var -- and it STILL shares the list
begin
    Names.Add('John');                        // the caller sees this in BC and not here
end;
```

```al
l2 := l1;
l2.Add(x);                                    // l1.Count is 2 in BC and 1 here
```

**And the documentation gives the COPY idiom, which is the tell**: `l2 := l1.GetRange(1, l1.Count)`
is how AL asks for a shallow copy, and `GetRange` exists for that reason. A tree whose assignment
already copies makes that idiom a no-op and hides the difference in both directions -- the code that
wanted sharing is silently wrong, and the code that wanted a copy is silently right.

## AND IT IS NOT TWO TYPES, IT IS TWENTY-NINE

Every `jsonarray`, `jsonobject`, `jsontoken` and `jsonvalue` page carries the same note, and it
names the whole family:

> For performance reasons **all HTTP, JSON, TextBuilder, and XML types are reference types, not
> value types.** Reference types hold a pointer to the data elsewhere in memory, whereas value
> types store its own data.

| family | types | pages | in the door |
|---|---:|---:|---|
| Json (`JsonArray`, `JsonObject`, `JsonToken`, `JsonValue`) | 4 | 211 | classes with their own storage, all refusing |
| Http (`HttpClient`, `HttpContent`, `HttpHeaders`, `HttpRequestMessage`, `HttpResponseMessage`) | 5 | 66 | the same |
| Xml (`XmlDocument`, `XmlElement`, `XmlNode`, `XmlAttribute`, and the rest) | 17 | ~290 | the same |
| `TextBuilder` | 1 | 13 | the same |
| `List`, `Dictionary` (from their own pages) | 2 | 30 | **built, and copying** |

**`Clone()` is the tell on the Json side.** `JsonToken.Clone()` and `JsonValue.Clone()` are
documented as "creates a deep-copy" -- a method that only makes sense on a type whose ordinary
assignment does NOT copy. A tree where assignment copies makes `Clone` redundant and the sharing
impossible, which is the same double error `GetRange` shows on the List side.

**Where it bites today is `List` and `Dictionary`**, because they are the two that are built. The
other 27 refuse, so the wrong storage shape is invisible until they are written -- which is exactly
when it is cheapest to get right, and why this item is filed before them.

## The population, measured 2026-09-04

| | |
|---|---:|
| `List of [...]` and `Dictionary of [...]` declarations under `Layers/W1` | **1 184** |
| files declaring at least one `List of [...]` | 278 |
| documented pages belonging to a type the note covers | **~610 of 1 741** |

## What the predecessor made of it

Python's lists and dicts are references, so `~/Git/openerp` got this for free and its board says
nothing about it. That is the same shape as board:0041's case conversion: a semantic the predecessor
never had to think about is one this tree has to buy, and its silence is not evidence of ease.

## The choice

- **The value class holds a `std::shared_ptr` to its store**, so a copy shares and the semantics
  are the language's rather than a convention. `List<T>` stays a value type in C++ -- AL declares
  `Names: List of [Text]` as a variable, not a pointer -- and the SHARING lives one level down.
- **The store is created on first use, not on declaration.** An AL list variable that is never
  touched costs a null pointer, which is the same budget rule board:0018 applied to filters and
  board:0006 measures.
- **`GetRange` is the copy**, exactly as the page says, and it is the ONLY thing that makes a new
  store. That keeps one rule in one place instead of a `Copy()` this tree would have invented.
- **A `var` parameter needs nothing extra**: a C++ reference to a shared handle behaves the same,
  which is why the `var` half of AL's surface is already right and only the by-value half is wrong.
- **Deep copy stays the caller's job**, as the page's own nested-list example shows. Nothing in the
  runtime should try to be clever about a `List of [List of [Integer]]`.

## Gate

A list passed to a procedure WITHOUT `var` and appended to there: the caller sees the new element. A
list assigned to a second variable and appended to: both count the same. `GetRange(1, Count)`
followed by an append: the original does NOT change. The same three for a dictionary.

**Negative control**: make the store a plain member again and require the first case to go red. It
is red today, which makes this the one gate in the sweep that can be written before the fix and
watched to change.
