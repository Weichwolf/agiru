# 0695 A door method returns AL's own Text

**The finding.** `Config. XML Exchange` writes `FileSize := PackageXML.OuterXml().Length()` and
`Config. Validate Management` writes `DurationVar := FieldRef.Value`. Neither compiled, and both
are the name-equality invariant failing on a TYPE rather than on a name:

- **`std::string` is not AL's `Text`**, so none of AL's text methods are on it -- `Length`,
  `Contains`, `Substring`. The .NET XML family returned `std::string` from eleven methods; they
  return `::agiru::Text<0>` now, which is what `.NET string` is in AL's eyes. The same sweep is
  owed to the AL `Xml*` types in `include/type/`, and that is its own round: those definitions live
  in three other sources and the blast radius is every XML user.
- **A Variant converting to a Duration was AMBIGUOUS with `Duration`'s own converting
  constructor**, which takes anything an Integer converts from -- and a Variant does. The Variant
  is the one that knows what it holds, so the constructor stands aside for it: the constraint reads
  `!requires { typename D::Held; }`, which is how the door already recognises a Variant.

**Beside it:** the runtime's own XmlPort reader took `child.Name()` as a `std::string`, which is
the cost of the change and the reason it is worth making once rather than per call site.

**Measured.** Chain 120, A/B against chain 119's 1 729; the two codeunits behind it carry 23 cases.
