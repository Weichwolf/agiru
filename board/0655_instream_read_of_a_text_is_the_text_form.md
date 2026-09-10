# `InStream.Read(var Text)` is the text form and not a typed read

**Finding (2026-09-10).** `My Notifications.GetFiltersAsText` reads its filter BLOB with
`FiltersInStream.Read(Filters)` where `Filters: Text`, and the door sent it to the typed refusal
("a typed Read expects the platform's own binary layout"): 17 UT cases (`ERM VAT Tool - UT`,
`Registration UT`). The text overload was constrained on `std::assignable_from<T &,
std::string_view>`, and that concept also demands a common reference type, which `Text` and
`std::string_view` do not have.

**Reference.** `instream-read-text-method.md`: reads text up to the terminator.

**Choice.** A requires-expression concept, `TextAssignable`, on both text overloads and negated on
the typed one (`type/Stream.h`); gate case `ReadOfATextTakesTheTextForm`. Silent-wrong-data
turned refusal -- the only visible effect is that a refusal becomes a read.
