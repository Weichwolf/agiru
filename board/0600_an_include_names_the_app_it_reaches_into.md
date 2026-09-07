Type:     bug
Status:   open
Area:     gen, build
Source:   the first full build over the grown slice, 2026-09-07
Class:    silent-wrong-data

# An include names the app it reaches into, and no file gets another app's header by accident

**A generated file included another app's header and the compiler could not have said so.**
`apps/base/.../PermissionSetBuffer.cpp` asks for

```
#include "system/security/access_control/table/PermissionSetBuffer.h"
```

and gets `apps/system/...` -- a DIFFERENT object -- because the two apps carry the same relative
path and `-Iapps/system` stands before `-Iapps/base` on the command line. The redefinition that
stopped the build is the lucky case: the two objects happened to declare the same class name. Where
they do not, the file compiles and is wrong.

## Measured over the transpiled tree, 2026-09-07

| | |
|---|---:|
| relative header paths carried by two or more apps | **846** |
| of those, carrying DIFFERENT content | **2** |

The 846 are the report, query and xmlport stubs: every app writes its own copy of every one it
references, byte for byte the same, so the include order picks a copy and nothing is lost. The two
that differ are `system/security/access_control/table/PermissionSetBuffer.h` (base and system) and
`system/test_tools/code_coverage/xmlport/CodeCoverageDetailed.h` (base against eight test apps).

**The count that matters is not 2, it is 846.** Each is a path where an app's include resolves by
`-I` ORDER rather than by what the file meant, and the day one of them starts to differ nothing
reports it.

## The choice

**The app goes into the include path** -- one `-Iapps`, and a generated file writes
`#include "base/system/security/access_control/table/PermissionSetBuffer.h"`. That is BC's own unit
(CLAUDE.md: an app is a library) and it is what AL declares in `app.json`: an app names the apps it
depends on, so naming one in the include is saying what the dependency already says.

**The alternative -- keeping one `-I` per app and making the paths unique some other way -- is
rejected**: it would have to encode the app in the FILE name, which is the app boundary written
somewhere it cannot be read.

**And the stub duplication should stop with it.** 844 identical copies of the same stub exist
because each app writes its own; one shared tree (`apps/shared`, which the include path already
carries) is one file each. That is a separate change and it is what makes the 846 fall to nearly 0.
