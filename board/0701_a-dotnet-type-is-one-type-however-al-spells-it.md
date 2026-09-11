# 0701 A DotNet type is one type, however AL spells it

**The finding.** `dotnet::CRMHelper` and `dotnet::CrmHelper` are two C++ structs in
`apps/absent/absent/Types.h` (lines 456 and 874) and one .NET type. AL is case-insensitive, so
`CDSIntegrationImpl` declares the variable one way and a procedure takes it the other; the
generated call then has `no known conversion from 'dotnet::CRMHelper' to 'dotnet::CrmHelper &'`
and the whole translation unit stays out of the slice -- 6 UT cases report
`CDSIntegrationImpl::IsIntegrationEnabled() is declared and its source is not in the slice`.

**This is the casing trap CLAUDE.md already names** ("AL is case-insensitive; diverging casing
produces two symbols -- collapse match, once, in the generator"), unfixed for DotNet SUBTYPES.
`agiru::gen::NoteDotNet` keys `DotNetUse` on `Identifier(declared.subtype)` as AL spelled it.

**Why it is not fixed here: it needs two passes and this round had none to spare.** The canonical
spelling can only be chosen once every object has been scanned, and the bodies that name the type
are written during that same scan. The shape of the fix: gather the case-folded names first, choose
one spelling per name DETERMINISTICALLY (most frequent, ties lexicographic -- never "the first file
read", which makes the output depend on directory order), and spell every DotNet identifier through
that table.

**The population before building for it:** `grep` over `apps/absent/absent/Types.h` for names that
differ only in case says how many types this is, and the same collapse has to be checked for the
rebuilt `include/dotnet/` names, where a second spelling would be a door-spelling ambiguity instead
(memory: rebuilt-dotnet-type-rules, rule 3).
