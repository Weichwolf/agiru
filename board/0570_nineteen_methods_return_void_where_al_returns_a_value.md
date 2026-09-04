Type:     bug
Status:   open
Parent:   0035
Area:     net, rt
Source:   developer/methods-auto/ -- 19 method pages across 14 types
Verdict:  teilweise
Class:    silent-wrong-data

# Nineteen methods return `void` where AL returns a value

Found by the same mechanical pass as board:0569: the documented syntax block gives a return value and
**every declaration of the name in the door returns `void`.**

| method | AL returns |
|---|---|
| **`jsonobject.Keys`** | `List of [Text]` |
| **`jsonobject.Values`** | `List of [JsonToken]` |
| **`navapp.ListResources`** | `List of [Text]` |
| `httpheaders.Keys` | `List of [Text]` |
| `httprequestmessage.GetCookieNames` | `List of [Text]` |
| `httpresponsemessage.GetCookieNames` | `List of [Text]` |
| `navapp.GetCallstackModuleInfos` | `List of [ModuleInfo]` |
| `navapp.GetCallerCallstackModuleInfos` | `List of [ModuleInfo]` |
| `testhttprequestmessage.QueryParameters` | a dictionary of the query string |
| `page.GetBackgroundParameters` | `Dictionary of [Text, Text]` |
| `system.GetCollectedErrors` | `List of [Text]` |
| `dictionary.Set` | `Boolean` |
| `list.RemoveRange` | `Boolean` |
| `mediaset.Insert` | `Guid` |
| `instream.ResetPosition` | `Boolean` |
| `recordid.GetRecord` | `RecordId` |
| `errorinfo.CustomDimensions` | `Dictionary of [Text, Text]` |
| **`notification.Send`** | `Boolean` |
| **`notification.Recall`** | `Boolean` |

## Two different defects wearing one shape

**Eleven of the nineteen return a COLLECTION, and returning `void` makes the method useless rather
than merely inconvenient.** `JsonObject.Keys()` is the only way to enumerate a JSON object's members;
`void Keys()` (`include/type/JsonObject.h:286`) and `void Values()` (`:466`) mean a transpiled AL file
that walks a JSON object cannot be written at all. The same for `NavApp.ListResources`, whose page is
explicit -- *"`Result := NavApp.ListResources([Filter: Text])`, Type: `List of [Text]`"* -- against
`static void ListResources(std::string_view Filter)` at `include/type/NavApp.h:151`.

**Eight return a `Boolean` or an id, and those are CLAUDE.md's named `value context` trap.** AL decides
at consumption-versus-discard whether a failure raises or yields `false`; `void` can express only one
of the two. board:0562 filed `Notification.Send` and `Notification.Recall` from the concept page
before this pass ran, and the mechanical check finds the same two independently -- which is the first
time in this sweep that a hand-read finding and a mechanical one have met on the same defect.

## What was checked by hand

**Five of the nineteen**, and five confirmed: `JsonObject.Keys` and `Values`, `NavApp.ListResources`,
`Notification.Send`, `Dictionary.Set`. The rule and the reading agree.

**One caveat, stated because it survived the check rather than because it did not**: the test is "every
declaration of that name in the type's chosen headers returns `void`", so a name that is also declared
non-`void` elsewhere in `include/` would not appear here. That makes 19 a LOWER bound.

## The IST-state

All nineteen are declared and every one of them refuses or does nothing today (board:0035), so no AL
code observes the wrong type yet. **The defect is in the signature**, which is checked-in code that a
body will inherit -- the same reasoning board:0564 is filed under, and the reason this is a bug rather
than a task.

## The choice

**Return what the page says, and take the collection types from the door's own vocabulary.**

```cpp
[[nodiscard]] ::agiru::List<std::string> Keys() const;
[[nodiscard]] ::agiru::List<::agiru::JsonToken> Values() const;
static ::agiru::List<std::string> ListResources(std::string_view Filter = {});
::agiru::Boolean Send();
```

**Why `List<T>` by value and not an out parameter:** AL returns it, board:0078 already records that
`List` is a REFERENCE type in AL, and the door has `include/type/List.h`. An out parameter would be a
second spelling for a return, and CLAUDE.md's name-equality invariant says a reader who knows AL must
recognise the signature -- `Keys()` returning a list is what the page shows.

**The `Boolean` ones raise on discard**, which is board:0028's value-context machinery; this item only
changes the type so that machinery has something to work with.

**`ListResources` gets its default at the same time** -- the page's `[Filter: Text]` -- which is
board:0564's family, and doing both in one edit avoids touching the declaration twice.

## Ordering

**The eleven collection returns first**, because they are the ones where `void` makes AL
inexpressible: no amount of runtime work makes `void Keys()` usable, so every consumer is blocked
behind them. **The eight value returns second**, with board:0028's value context.

**`JsonObject.Keys` and `Values` are the very first**, since `JsonObject` is 31 documented methods
with 66 pages behind them and is the door's most-used composite type after `Record`.

## Gate, and its negative control

1. `JsonObject.Keys()` over an object with three members returns a list of three texts, in insertion
   order
2. `NavApp.ListResources()` compiles with no argument and returns a list
3. `if not MyNotification.Send() then` compiles and takes the `false` branch when sending fails
4. `MyNotification.Send()` as a statement, with the send failing, RAISES rather than returning quietly

**The negative control is case 4 against case 3.** Give `Send` a `Boolean` return and stop there:
cases 1, 2 and 3 pass while case 4 silently swallows the failure. It is the half of the value-context
rule that a return type alone does not deliver, and it is the half an accounting system cannot lose
-- CLAUDE.md's first invariant is that an error is never swallowed.

**Case 1's ORDER is the second control**: return the keys in an unspecified order and case 1 passes
whenever the object has one member and fails intermittently otherwise. The gate uses three members and
compares the sequence.

## Class

`silent-wrong-data` for the eight value returns -- a discarded failure is a posting that reported
success. The eleven collection returns are closer to `fehlt`: they cannot be called at all, so they
fail loudly, and they are in this item because they are the same edit and the same pass found them.
