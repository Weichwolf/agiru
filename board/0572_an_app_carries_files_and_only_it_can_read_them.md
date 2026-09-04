Type:     task
Status:   open
Parent:   0033
Area:     gen, rt
Source:   developer/devenv-app-resources.md
Verdict:  deklariert
Class:    activation

# An app carries files, and only it can read them

An extension may package arbitrary FILES and read them at run time. `app.json` names the folders:

```json
"resourceFolders": ["SampleImages", "ConfigurationFiles"]
```

and AL reads them by a path relative to that folder:

```al
NavApp.GetResource('Templates/Template1.txt', resourceStream, TextEncoding::UTF8);
```

**This is how a report layout, a sample dataset or a configuration file travels with an app**, and it
is the mechanism board:0547's Word and RDLC layout files sit on.

## Four methods, and the door has all four with the wrong shapes

| AL | door |
|---|---|
| `NavApp.GetResource(Name, var Stream [, Encoding])` | `static void GetResource(std::string_view, InStream &, const TextEncoding &)` -- `include/type/NavApp.h:104` |
| `NavApp.GetResourceAsText(Name [, Encoding]): Text` | `static std::string GetResourceAsText(std::string_view, const TextEncoding &)` -- `:123` |
| `NavApp.GetResourceAsJson(Name [, Encoding]): JsonObject` | `static JsonObject GetResourceAsJson(std::string_view, const TextEncoding &)` -- `:114` |
| `NavApp.ListResources([Filter]): List of [Text]` | **`static void ListResources(std::string_view)`** -- `:151` |

**Three of the four make `Encoding` mandatory where AL makes it optional**, and the fourth returns
`void` where AL returns a list. Both are already filed -- board:0570 owns the `void` and the ledger's
methods-auto pass counts the three among its 78 arity gaps -- so **this item does not re-file them**;
it is the STORE those signatures read from, which nothing owns.

## The limits, and they are hard numbers

| limit | value |
|---|---:|
| any single resource file | **16 MB** |
| all resource files together | **256 MB** |
| number of resource files in an extension | **256** |

board:0081 is "every documented limit is a `static_assert` beside the object that could break it", and
these three qualify exactly: the transpiler knows every file it packages, so **a 17 MB resource is a
translation error rather than a run-time surprise.** The page adds *"these limits are subject to
change"*, which is why they belong in one named place rather than inline.

## The isolation rule is the app boundary again

> "Resources can only be accessed WITHIN the extension that includes them; it isn't possible for
> extensions to access resources that are packaged with other extensions ... **if two apps declare
> resources with the same name, they're only able to access their own versions.**"

So the resource name space is per app, and two apps may both carry `Templates/Template1.txt`.
**That makes the store a per-app map and not a global one**, which is the same direction CLAUDE.md's
app-is-a-library rule points and the same boundary board:0033 owns.

**It also decides the C++ shape**: `NavApp.GetResource` is a STATIC method with no app argument, so
the app has to come from the CALLER's context. The caller is a generated object in exactly one app, so
the app is known at translation time -- and this is the one place in the door where a `static` method
needs to know which app called it.

## Population

**Not measured, and the reason is stated rather than a number invented.** `resourceFolders` lives in
`app.json`, not in `.al`, so the `(^|[{;])\\s*<Name>\\s*=` pattern does not reach it; and
`NavApp.GetResource` is 2026 release wave 2 (25.2), so the call sites in a tree pinned at BCApps
`main` may be few or none. **The one number that matters is countable when the item is pulled** --
`grep -c resourceFolders` over the `app.json` files -- and it is named here as undone.

## The choice

**The resource store is `constexpr` metadata plus the bytes, and both are emitted by the transpiler.**

```cpp
struct ResourceDef { std::string_view name; std::span<const std::byte> bytes; };
struct AppDef { AppId id; std::span<const ResourceDef> resources; };
```

**Why the bytes in `.rodata` and not files on disk:** CLAUDE.md's object metadata rule -- demand-paged,
shared between processes, zero startup cost -- applies to a resource for the same reason it applies to
a field table, and 256 MB is the documented ceiling for a whole app. **A file on disk would also make
the app boundary a path convention**, which is exactly what the isolation rule forbids.

**Three `static_assert`s, one per documented limit**, beside the app rather than beside the object:
16 MB per resource, 256 MB per app, 256 resources per app.

**`GetResource` resolves the app from the calling object**, which the generator knows. A resource name
that no resource in that app carries is an error and not an empty stream -- the page gives no
alternative and a silent empty stream is the failure mode CLAUDE.md's loud-failure rule exists for.

**`ListResources`' filter is a WILDCARD over the full name**, per the page's own examples:
`ListResources('Images')` returns the two under `Images/` and `ListResources('*.png')` returns all
three PNGs wherever they are. So it matches the path, not the folder.

## Ordering

**After board:0547's report layouts**, which is the consumer that makes this necessary rather than
merely documented: a Word layout is a file, and this is how a file reaches the runtime.
**Before board:0035's stream work touches it**, since `GetResource` hands back an `InStream`.

The signature corrections belong to board:0570 and the methods-auto pass and are not repeated here.

## Gate, and its negative control

1. an app declaring `"resourceFolders": ["Resources"]` with `Resources/Templates/T.txt` emits one
   `ResourceDef` named `Templates/T.txt`
2. `NavApp.GetResourceAsText('Templates/T.txt')` returns the file's content, with no encoding argument
3. `ListResources('*.txt')` returns exactly that one name
4. a second app carrying a file of the SAME name reads its own
5. a 17 MB resource fails to transpile

**The negative control is case 4.** Put the resources in one global map keyed by name -- the obvious
implementation -- and cases 1, 2, 3 and 5 all stay green while case 4 silently returns the other app's
file. It is the case that proves the store is per app rather than merely named per app.

**Case 3 is the second control**, against a filter that matches the FOLDER instead of the path: it
passes for `ListResources('Images')` and fails for `'*.png'`, and only one of those two is in the
gate unless both are.

## Class

`activation`. All four methods refuse today (board:0035), so no AL reads a resource and nothing can
regress. What it activates is small and specific -- and it is a precondition for board:0547 rather
than a subject of its own, which is why it is filed here at its true size instead of being folded into
the layout work where it would be invisible.
