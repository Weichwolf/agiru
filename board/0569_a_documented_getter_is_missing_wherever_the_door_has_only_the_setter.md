Type:     bug
Status:   open
Parent:   0035
Area:     net, rt
Source:   developer/methods-auto/ -- 50 method pages across 22 types
Verdict:  teilweise
Class:    silent-wrong-data

# A documented getter is missing wherever the door has only the setter

**AL's property-access shape is one name with an optional argument**:

```AL
[Title := ]  ErrorInfo.Title([NewTitle: Text])
```

Call it with an argument and it SETS; call it without and it GETS. **The door has only the setting
form, 50 times**, so the reading form does not compile at all.

```cpp
std::string Title(std::string_view Title);                      // include/type/ErrorInfo.h:188
std::string Company(std::string_view NewCompanyName);           // include/type/SessionSettings.h:39
::agiru::Duration Timeout(::agiru::Duration SetTimeout);        // include/type/HttpClient.h:139
std::string Value(std::string_view NewValue);                   // include/type/XmlAttribute.h:177
```

Each returns the right type and takes the setter's argument. **None of them can be called to read.**

## The population, measured 2026-09-04 over `methods-auto/`

Found mechanically -- the documented syntax block's minimum argument count is 0 and its maximum is 1,
and every declaration of the name in the type's headers takes exactly one argument. **50 methods over
22 types:**

| type | n | methods |
|---|---:|---|
| `errorinfo` | **12** | `Collectible`, `ControlName`, `CustomDimensions`, `DataClassification`, `DetailedMessage`, `ErrorType`, `FieldNo`, `PageNo`, `SystemId`, `TableId`, `Title`, `Verbosity` |
| `sessionsettings` | **7** | `Company`, `LanguageId`, `LocaleId`, `ProfileAppId`, `ProfileId`, `ProfileSystemScope`, `TimeZone` |
| `page` | 4 | `LookupMode`, `ObjectId`, `PromptMode`, `Update` |
| `file` | 3 | `CreateTempFile`, `TextMode`, `WriteMode` |
| `requestpage` | 3 | `LookupMode`, `ObjectId`, `Update` |
| `xmldeclaration` | 3 | `Encoding`, `Standalone`, `Version` |
| `navapp` | 2 | `IsUnlicensed`, `ListResources` |
| `textbuilder` | 2 | `AppendLine`, `Capacity` |
| eleven types with one each | 11 | `datatransfer.UpdateAuditFields`, `dialog.HideSubsequentDialogs`, `filterpagebuilder.PageCaption`, `httpclient.Timeout`, `httprequestmessage.Method`, `system.GetLastErrorText`, `testhttpresponsemessage.IsSuccessfulRequest`, `xmlnamespacemanager.NameTable`, `xmlreadoptions.PreserveWhitespace`, `xmlwriteoptions.PreserveWhitespace`, plus `Value` on `xmlattribute`, `xmlcdata`, `xmlcomment` and `xmltext` |

**Five were checked by hand and five confirmed** -- `ErrorInfo.Title`, `SessionSettings.Company`,
`HttpClient.Timeout`, `XmlAttribute.Value`, `SessionSettings.LanguageId`. The mechanical rule and the
hand check agree, which is why this family is filed and the 78 other arity gaps are not.

## Why it is `silent-wrong-data` and not merely missing

**Most of the 50 are not read by AL at all today**, because nothing runs. The classification is for
what a reader does with the door: **`std::string Title(std::string_view)` LOOKS like a getter with a
mandatory argument**, and the natural way to make AL's `ErrorInfo.Title` work against it is to pass
the CURRENT value back in -- which round-trips and appears to work while writing the field it was
supposed to read.

**`ErrorInfo` makes that concrete.** board:0517 and board:0518 own the error shapes; an error handler
that reads `ErrorInfo.Message`, `ErrorInfo.DetailedMessage` and `ErrorInfo.Verbosity` to decide what
to show cannot read any of them. Twelve of the 50 are on that one type, and it is the type the whole
`Collectible` error mechanism hangs on.

**`SessionSettings` is the other cluster and it is worse in kind**: `Company`, `LanguageId`,
`TimeZone` are read to DECIDE something -- `SessionSettings.Company()` before switching, then
`RequestSessionUpdate`. A setter-only surface makes the read impossible and the natural workaround
sets the value the caller was trying to inspect.

## The choice

**Two overloads per name, not a defaulted argument.**

```cpp
[[nodiscard]] std::string Title() const;               // get
std::string Title(std::string_view NewTitle);          // set, returns the previous value
```

**Why not a default argument:** `Title({})` and `Title()` would be the same call, so setting the title
to the empty string becomes unreachable -- the exact collision board:0564 records for
`GlobalLanguage()` against `GlobalLanguage(0)`. Two overloads have no such ambiguity, and AL's
`[NewTitle: Text]` notation means "two forms", which is what two overloads are.

**What the setter returns is the documented value and it is worth stating**: the syntax block is
`[Title := ] ErrorInfo.Title([NewTitle: Text])`, one return for both forms. The AL pages do not say
whether the setter returns the OLD or the NEW value, and **that is left open here rather than
guessed** -- it is one gate case per type and it is the first thing to settle when the item is
pulled.

**`const` on the getter and not on the setter**, which is what makes the two overloads resolve without
ambiguity on a non-`const` object: the compiler prefers the non-`const` setter only when an argument
is supplied, because the arity differs. So the pair is unambiguous by arity alone and `const` is
correctness rather than a trick.

## Ordering

**`errorinfo` first, at 12 of the 50**, and because board:0517's error shapes read those fields.
**`sessionsettings` second, at 7**, because the reads decide a switch. The remaining 31 are one or two
per type and follow their own subjects.

**Before any of the runtime work that reads them** -- this is a header change with a `-Werror` blast
radius and no run-time behaviour of its own, so it is cheap now and expensive once callers exist.

## Gate, and its negative control

1. `ErrorInfo.Title()` compiles and returns the current title
2. `ErrorInfo.Title('x')` compiles, sets it, and case 1 then returns `'x'`
3. `ErrorInfo.Title('')` sets the title to the EMPTY STRING, and case 1 returns empty
4. the same three for `SessionSettings.Company`

**The negative control is case 3.** Implement the pair as one method with a defaulted argument --
the obvious shortcut -- and cases 1, 2 and 4 all stay green while case 3 silently becomes a GET. It
is the case that distinguishes two overloads from one default, and it is the same control board:0564
needs for `GlobalLanguage`.

## Class

`silent-wrong-data`. The failure is not a compile error in the runtime -- it is a caller who cannot
read and therefore writes what they meant to inspect. Nothing throws, and the value that comes back is
the value that was just stored.
