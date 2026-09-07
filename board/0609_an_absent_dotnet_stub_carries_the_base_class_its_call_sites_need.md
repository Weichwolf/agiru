Type:     task
Status:   open
Area:     gen, net
Source:   the last data symbol that stops `agiru run-tests`, 2026-09-07
Class:    activation

# An absent .NET stub carries the base class its call sites need

**`kConfigSetupTable` IS THE LAST DATA SYMBOL, and one inheritance edge is what is missing.**

```
apps/base/system/io/table/ConfigSetup.cpp:266: error: non-const lvalue reference to type
'dotnet::XmlNode' cannot bind to a value of unrelated type 'dotnet::XmlElement'
```

In .NET, `XmlElement` derives from `XmlLinkedNode`, which derives from `XmlNode`, so AL passes one
where the other is declared and the compiler there agrees. Here both are generated stubs and both
derive from `::agiru::dotnet::AbsentType` and from nothing else, so they are siblings and the
conversion does not exist.

## What the reference says

`~/Git/BCApps` writes `ConfigXMLExchange.GetAttribute(Name, DocumentElement)` where the parameter is
declared `DotNet XmlNode` and the argument is a `DotNet XmlElement`. The platform documentation says
nothing about .NET's own hierarchy, because it is .NET's and not BC's. `~/Git/openerp/` mapped these
onto Python's `xml.etree` and lost the distinction entirely -- its board records the bleeding on the
semantic difference, which is one of the three reasons CLAUDE.md gives for leaving Python.

## The three ways, and none is free

1. **THE STUBS INHERIT.** The generator would have to know .NET's class hierarchy, which no input it
   reads carries. A table of edges in the generator is an AL-object-shaped fact in `src/gen/` --
   the thing this tree forbids -- unless it is sourced, and the source would be .NET's own metadata.
2. **EVERY ABSENT STUB CONVERTS TO EVERY OTHER.** One templated conversion on `AbsentType` makes all
   of them interchangeable. It is honest about what the tree knows (nothing) and it is a hole with a
   number: a refusal that converts to anything is board:0608's finding one level up, and the same
   overload-resolution catch-all follows.
3. **THE .NET XML CLASSES BECOME REAL DOOR CLASSES.** `XmlNode`, `XmlElement`, `XmlDocument`,
   `XmlAttribute`, `XmlNodeList` under `include/dotnet/`, with the hierarchy .NET declares. That is
   what CLAUDE.md says the .NET types are -- "REBUILT, one C++ class per .NET class" -- and it is
   the only one of the three that also makes the code RUN. It is also the largest.

**The order that follows:** 3 for the XML family, because `ConfigSetup` is not the only caller and
because reports, imports and e-documents all wait on the same classes; 2 never, because it trades a
compile error for the class of defect board:0608 measured.

## What proves it

`ldd -r build/agiru` names no undefined data symbol, and `agiru run-tests --list` prints the 78 UT
codeunits. The negative control is `ConfigSetup` itself: with the inheritance in place its source
compiles, and with it removed the same line fails again with the same message.
