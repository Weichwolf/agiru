Type:     task
Status:   open
Parent:   0076
Area:     gen
Verdict:  fehlt
Class:    compile root

# Two options with one member list are one type, so a field binds to a `var Option` parameter

`ChangeGlobalDimHeader.CalcChangeType(var ChangeType: Option None,Blank,Replace,New; ...)` is
called with the table's own field `"Change Type 1"`, declared with the same four members. The
codeunit-local option is named by content (`ChangeGlobalDimHeaderOptionNoneBlankReplaceNew`),
the FIELD's option by table and field -- two C++ types, and a `var` parameter cannot bind across
them (bulk run over 388 table sources, 2026-09-06). By value it converts; by reference it is a
compile error, and it is the table's own field.

## The choice

A field's option enumeration is named by content too, the way the local ones already are
(`OptionNameOf`), so every option with one member list in one owner is one type. The header's
`enum class` and its `OptionTraits` move under that name; the field keeps its identifier. The
board:0076 ordinal assertion is unchanged because the ordinals are the member list.

## Gate

A table whose field and procedure parameter share a member list emits one enumeration; the
negative control is two fields with different lists staying two.
