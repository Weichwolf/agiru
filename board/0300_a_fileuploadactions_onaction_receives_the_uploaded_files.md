Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/fileuploadaction/devenv-onaction-fileuploadaction-trigger.md
Verdict:  fehlt
Class:    activation

# A file upload action's `OnAction` receives the uploaded files as a list

```al
trigger OnAction(Files: List of [FileUpload])
```

Introduced in runtime 13.0. It is the only trigger whose parameter is a `List of [...]`, and the
list is a **reference type** (board:0078) -- so the same object identity question applies: the
trigger receives the platform's list, not a copy.

`AllowedFileExtensions` and `AllowMultipleFiles` (board:0067) are the properties that decide what
reaches it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAction(Files:`: **13 declarations** -- new, and small.

## The IST-state

No page runtime. `include/type/FileUpload.h` exists as a door header with refusing bodies, and
`include/type/List.h:197` holds `std::vector<T> values_` BY VALUE, which board:0078 records as the
reference-type defect.

## The choice

The page's upload path builds the list and calls the trigger. **Whatever board:0078 decides for
`List` decides this too**: if a `List` is a value here and a reference in AL, a trigger that clears
its list leaves the platform's copy full.

**The bytes are the other half.** A `FileUpload` carries a stream, and board:0074's encoding rules
apply at the boundary -- an uploaded file is read in the declared encoding and converted to Unicode,
which is the direction the page states for imports.

## Ordering

Blocked on board:0030 and board:0078. Low: 13 sites, and none in the milestone's 78 UT codeunits.

## Gate, and its negative control

An upload action given two files with `AllowMultipleFiles` set: the trigger receives a list of two.

**The negative control is `AllowMultipleFiles = false`** -- the second file must be refused before
the trigger runs, not filtered inside it, or the property is decorative.
