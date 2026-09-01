Type: arc
State: open
Area: gen

# The field table is derived from the class rather than written beside it

A generated table names each field's identifier twice: once as the member, once inside `offsetof`.
Everything else it says once. That second mention is not a design choice -- it is a missing language
feature, and it goes when the feature arrives.

## Reference

**Measured 2026-09-01**: neither `clang-19` nor `g++-14` has C++26 reflection. `__cpp_reflection`
and `__cpp_lib_reflection` are both undefined under `-std=c++26`, and there is no
`<experimental/meta>`. What does exist is structured bindings over an aggregate, which reach a
class's members but not their NAMES -- and the name is most of what an AL field declares.

**What is irreducible without reflection**, and why:

| property | where it comes from today |
|---|---|
| the member's type | the member declaration |
| AL field number | declared -- no C++ type carries it |
| AL name, `"Work Type Code"` | declared -- the identifier is `WorkTypeCode` |
| `Caption` | declared -- it differs from the name in general, and XLIFF will replace it |
| field type tag | DERIVED, `agiru::FieldTypeOf` |
| declared length | DERIVED from `Code<20>::kMaxLength` |
| option member names | DERIVED from `OptionTraits<E>::kMembers` |
| offset | `offsetof(Class, member)` -- and this is the one that repeats the identifier |

No member pointer yields a `constexpr` offset in standard C++, so the identifier cannot be written
once. A macro could hide the repetition, and one was built here and thrown away: it moved the cost
onto the reader, and a reader who has to expand a macro to know how to write the next table is
exactly the reader this tree is written for (CLAUDE.md, the naming invariant's second reason).

**Why the repetition is acceptable meanwhile.** The rule "declare each thing once" guards against
DRIFT between two hand-maintained copies. There is no hand here: a generator writes both from one
AST node, and the compiler holds them together -- a `static_assert` on the field count and on the
sort order, plus the type coming from the member pointer rather than from a second spelling. A
repetition a machine emits and a compiler checks is a checksum, not a duplication.

## What will be true

- [ ] When a reference compiler ships P2996, the field table is derived from the class and the
      `offsetof` line disappears; what remains declared is the number, the AL name and the caption.
- [ ] The generator emits one form or the other from the same AST node, chosen by a feature test
      rather than by a flag somebody sets.
- [ ] Proof: the generated output for table 202 is byte-identical in behaviour under both forms --
      the same field table, checked by the same gate cases.
- [ ] **Negative control**: remove one field from the class and require the derived form to fail to
      compile. A derivation that silently produces a shorter table is worse than the repetition it
      replaced.
