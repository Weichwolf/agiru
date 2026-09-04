Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/properties/devenv-namespaces-property.md, developer/properties/devenv-namespaceprefix-property.md, developer/properties/devenv-defaultnamespace-property.md, developer/properties/devenv-usedefaultnamespace-property.md
Verdict:  fehlt
Class:    activation

# An XMLport carries namespaces, and only the root element declares them

**Four pages, one item**: the XMLport declares the namespaces, an element names one by prefix, and two
more properties select the default namespace. Each page names the others; none is implementable alone.

> **Namespaces** (XmlPort): `Namespaces = bc = 'urn:...';` -- prefix = namespace, **separated by
> commas**. **"In the XML documents exported or imported by the XMLport, the namespace declarations
> are only supported in the `<root>` element."**
>
> `<Root xmlns:mybcprefix="mybcnamespace" xmlns="urn:bc:schema:all">`
>
> **To specify a default namespace, set the prefix to `""`.**
>
> **NamespacePrefix** (element and attribute nodes): **"You can only set the property to a prefix that
> is DECLARED in the `Namespaces` property of the XMLport."** And: **"This property only applies to
> ELEMENT node types and will be IGNORED if it is set in `textattribute` and `fieldattribute`
> nodes."**
>
> **DefaultNamespace** + **UseDefaultNamespace**: the namespace with no prefix. **"There can only be
> one default namespace. So if you specify a default namespace in `Namespaces`, you must set
> `UseDefaultNamespace` to FALSE."**

**Three rules, all decidable at translation time:**

1. A `NamespacePrefix` naming a prefix the XMLport does not declare -- `static_assert`.
2. A `NamespacePrefix` on an attribute node -- ignored by BC, so an assertion here is a deliberate
   deviation from its silence, taken for the same reason board:0442 takes it.
3. A default namespace declared in `Namespaces` together with `UseDefaultNamespace = true` -- two
   defaults, which the documentation says cannot both exist.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`NamespacePrefix =` **2 473** · `DefaultNamespace =` **29** · `UseDefaultNamespace =` **34** ·
`Namespaces =` **15**.

**2 473 elements name a prefix and only 15 XMLports declare one.** So a handful of namespace
declarations are referenced thousands of times, which is exactly what makes rule 1 worth asserting:
one undeclared prefix breaks a whole document and is invisible in the AL.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

The XMLport's prefix table is `constexpr`, and each element carries an INDEX into it rather than a
prefix string -- 2 473 references to 15 declarations is the case where interning is obviously right.
The root-only rule is the writer's: declarations are emitted once, on the root.

## Ordering

Inside board:0065, behind board:0442's format -- namespaces exist only for `Xml`.

## Gate, and its negative control

An export declares its namespaces on the root element and nowhere else, and an element declaring a
prefix is emitted in that namespace.

**The negative control is a nested element with a prefix** -- its declaration must NOT be repeated on
it, and an implementation that declares per element produces a valid document that differs from BC's
byte for byte, which no schema validation catches.
