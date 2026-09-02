import re, sys, pathlib
sys.path.insert(0, "/tmp/claude-1000/-home-cosmo-Git-agiru/94c34270-14d1-470d-916f-8c83426f799c/scratchpad")
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
    seen, decls, bodies = set(), [], []
    for m in methods:
        ps = [(n, door.cpp_param(t, v), t) for n, t, v in m["params"]]
        sig = (m["method"], tuple(p for _, p, _ in ps))
        if sig in seen:
            continue
        seen.add(sig)
        ret = door.cpp_return(m["ret"])
        args = ", ".join(f"{p} {n}" for n, p, _ in ps)
        al = f"{m['owner']}.{m['method']}(" + ", ".join(t for _, _, t in ps) + ")"
        doc = [f"/// \\brief AL `{al}`." + (f" {m['brief']}" if m["brief"] else "")]
        for n, _, t in ps:
            doc.append(f"/// \\param {n} The AL `{t}`.")
        if ret != "void":
            doc.append(f"/// \\return The AL `{m['ret']}`.")
        doc.append("/// \\throws Error always -- the surface is declared, the behaviour is not"
                   " (board:0035).")
        decls.append("\n".join(doc) + f"\n{ret} {m['method']}({args});")
        discard = "".join(f"  static_cast<void>({n});\n" for n, _, _ in ps)
        bodies.append(f"{ret} {m['method']}({args}) {{\n{discard}  RefuseDoor(\"{al}\");\n}}")
    return decls, bodies
