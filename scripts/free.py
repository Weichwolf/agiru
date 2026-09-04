import re, sys, pathlib
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import door

MARK = "can be invoked without specifying the data type name"


def free(owner):
    """Every documented method of `owner` that AL lets a body call without naming the type."""
    d = door.DOC / owner
    out = []
    for md in sorted(d.glob("*-method.md")):
        if MARK not in md.read_text(errors="replace"):
            continue
        m = door.parse(md)
        if m:
            out.append(m)
    return out


def emit(owner, methods, known):
    # A DEFAULT AND A SHORTER OVERLOAD CANNOT BOTH BE THERE. `GetUrl` is documented with six
    # parameters and again with seven; defaulting the seventh makes a six-argument call match both,
    # which is ambiguous rather than convenient. Where the page already gives the short form, the
    # long one keeps every parameter required.
    shortest = {}
    for m in methods:
        n = len(m["params"])
        shortest[m["method"]] = min(shortest.get(m["method"], n), n)
    seen, decls, bodies = set(), [], []
    for m in methods:
        # A `var Any` PARAMETER IS A TEMPLATE. AL's `Clear(var Any)` takes a record, a text, a
        # list -- anything at all, by reference -- and a `Variant &` takes none of them: a Variant
        # is a VALUE that carries a type, not a reference to one.
        ps = []
        generic = 0
        for n, t, v in m["params"]:
            if v and door.cpp_type(t) == "::agiru::Variant":
                generic += 1
                ps.append((n, f"Any{generic} &", t))
            else:
                ps.append((n, door.cpp_param(t, v), t))
        sig = (m["method"], tuple(p for _, p, _ in ps))
        if sig in seen:
            continue
        seen.add(sig)
        ret = door.cpp_return(m["ret"])
        required = m.get("required", len(ps))
        # A `var` PARAMETER CANNOT HAVE ONE. AL's optional out-parameter has no C++ spelling --
        # a non-const reference does not bind to a default -- so an overload would be needed and
        # the signature would stop matching the page. It stays required, and a call that leaves it
        # out fails loudly rather than writing into a temporary.
        # A DEFAULT IS A TAIL, so once one parameter cannot carry one, nothing after it may. A
        # `var` parameter cannot: a non-const reference does not bind to a default, and an overload
        # would stop the signature matching the page.
        overloaded = shortest.get(m["method"], len(ps)) < len(ps)
        defaults = [i >= required and not (p.endswith("&") and not p.startswith("const")) and not overloaded
                    for i, (_, p, _) in enumerate(ps)]
        for i in range(len(defaults) - 2, -1, -1):
            defaults[i] = defaults[i] and defaults[i + 1]
        args = ", ".join(f"{p} {n}" + (" = {}" if defaults[i] else "")
                         for i, (n, p, _) in enumerate(ps))
        plain = ", ".join(f"{p} {n}" for n, p, _ in ps)
        al = f"{m['owner']}.{m['method']}(" + ", ".join(t for _, _, t in ps) + ")"
        doc = [f"/// \\brief AL `{al}`." + (f" {m['brief']}" if m["brief"] else "")]
        for n, _, t in ps:
            doc.append(f"/// \\param {n} The AL `{t}`.")
        if ret != "void":
            doc.append(f"/// \\return The AL `{m['ret']}`.")
        doc.append("/// \\throws Error always -- the surface is declared, the behaviour is not"
                   " (board:0035).")
        head = ""
        if generic:
            names = ", ".join(f"typename Any{i + 1}" for i in range(generic))
            doc = doc[:1] + [f"/// \\tparam Any{i + 1} What AL handed it." for i in range(generic)] \
                  + doc[1:]
            head = f"template <{names}> "
        decls.append("\n".join(doc) + f"\n{head}{ret} {m['method']}({args}) {{\n" +
                     "".join(f"  static_cast<void>({n});\n" for n, _, _ in ps) +
                     f'  RefuseDoor("{al}");\n}}' if generic
                     else "\n".join(doc) + f"\n{ret} {m['method']}({args});")
        discard = "".join(f"  static_cast<void>({n});\n" for n, _, _ in ps)
        if not generic:
            bodies.append(f"{ret} {m['method']}({plain}) {{\n{discard}"
                          f'  RefuseDoor("{al}");\n}}')
    return decls, bodies
