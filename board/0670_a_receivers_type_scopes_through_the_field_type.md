# `FieldRef.Type::Member` scopes through `FieldType`, on an array element too

**Finding (2026-09-10).** `Find Record Management` writes `SearchFieldRef[1].Type <>
SearchFieldRef[1].Type::Code`; the generator resolved `X.Member::Value` only through a record
field's enumeration or three named methods, and emitted `RefusedOption(".::Code")` -- 6 cases of
Record Set UT.

**Reference.** `fieldref-type-method.md`: `Type` answers a `FieldType`; AL scopes an enum member
through any expression of that type.

**Choice.** `Type` joins the method-option table (`FieldType`), which the scoping on a member
receiver already consults after the field enumerations, for a plain or an indexed receiver.
Gate `AFieldRefsTypeScopesThroughFieldType`.
