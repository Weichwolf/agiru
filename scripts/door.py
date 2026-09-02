import re, pathlib, sys, collections

DOC = pathlib.Path.home()/"Git/dynamics365smb-devitpro-pb/dev-itpro/developer/methods-auto"
ROOT = pathlib.Path("/home/cosmo/Git/agiru")

SCALAR = {
 "Integer":"Integer","BigInteger":"BigInteger","Decimal":"Decimal","Boolean":"Boolean",
 "Date":"Date","Time":"Time","DateTime":"DateTime","Duration":"Duration","Guid":"Guid",
 "Byte":"Byte","Char":"Char","DateFormula":"DateFormula","RecordId":"RecordId",
}
TEXTY = {"Text","Code","String","Label","TextConst"}

GENERIC = re.compile(r"^(List|Dictionary|Array) of \[(.*)\]$")

def cpp_type(t):
    """The C++ spelling of an AL type NAME, without reference or constness."""
    t = t.strip()
    while t.endswith("]") and t.count("]") > t.count("["):
        t = t[:-1].strip()
    g = GENERIC.match(t)
    if g:
        if g.group(1) == "Array": return "::agiru::Variant"
        parts, depth, cur = [], 0, ""
        for c in g.group(2):
            if c == "[": depth += 1
            if c == "]": depth -= 1
            if c == "," and depth == 0:
                parts.append(cur); cur = ""
            else:
                cur += c
        if cur.strip(): parts.append(cur)
        inner = [cpp_type(x) for x in parts]
        return "::agiru::" + g.group(1) + "<" + ", ".join(inner) + ">"
    if t == "Record": return "::agiru::RecordRef"
    if t == "Option": return "::agiru::Integer"
    if t in SCALAR: return "::agiru::" + SCALAR[t]
    if t in TEXTY: return "std::string"
    if t in ("Any", "Variant"): return "::agiru::Variant"
    if re.fullmatch(r"[A-Za-z][A-Za-z0-9]*", t) and (t in EXISTING or t in GENERATED):
        return "::agiru::" + t
    UNKNOWN.add(t)
    return "::agiru::Variant"

def cpp_param(altype, isvar):
    t = altype.strip()
    while t.endswith("]") and t.count("]") > t.count("["):
        t = t[:-1].strip()
    c = cpp_type(t)
    if c == "std::string":
        return "std::string &" if isvar else "std::string_view"
    if t in SCALAR or t == "Option":
        return f"{c} &" if isvar else c
    return f"{c} &" if isvar else f"const {c} &"

def cpp_return(altype):
    t = (altype or "").strip()
    if not t: return "void"
    return cpp_type(t)

def parse(md):
    text = md.read_text(errors="replace")
    h = re.search(r"^# ([A-Za-z]+)\.([A-Za-z][A-Za-z0-9]*)\(([^)]*)\) Method", text, re.M)
    if not h: return None
    owner, method = h.group(1), h.group(2)
    brief = ""
    m = re.search(r"Method\n> \*\*Version\*\*:[^\n]*\n\n(.+?)\n\n", text, re.S)
    if m: brief = " ".join(m.group(1).split())
    syn = re.search(r"```AL\n(.+?)\n```", text, re.S)
    if not syn: return None
    sig = syn.group(1).strip()
    call = re.search(r"\.%s\((.*)\)\s*$" % re.escape(method), sig)
    if call is None: return None
    inner = call.group(1)
    # AL marks an OPTIONAL parameter with a bracket group that starts at the comma: `A: Text [, B:
    # Integer]`. `List of [Text]` uses the same bracket for something else entirely, so only a `[`
    # that a comma follows is an optionality marker, and its match closes it.
    cleaned = []
    drop = []
    depth = 0
    i = 0
    while i < len(inner):
        c = inner[i]
        if c == "[":
            depth += 1
            if not "".join(cleaned).rstrip().endswith("of"):
                drop.append(depth)
                i += 1
                continue
        if c == "]":
            if drop and drop[-1] == depth:
                drop.pop()
                depth -= 1
                i += 1
                continue
            depth -= 1
        cleaned.append(c)
        i += 1
    inner = "".join(cleaned)
    params = []
    depth = 0; cur = ""
    for c in inner:
        if c == "[": depth += 1
        if c == "]": depth -= 1
        if c == "," and depth == 0:
            params.append(cur); cur = ""
        else:
            cur += c
    if cur.strip(): params.append(cur)
    out = []
    for p in params:
        p = p.strip()
        if p.endswith("]") and p.count("]") > p.count("["): p = p[:-1].strip()
        # `Value1: Any,...` is AL's varargs and C++ has no such parameter.
        if p.endswith(",...") or p.endswith("..."): continue
        if not p: continue
        isvar = p.lower().startswith("var ")
        if isvar: p = p[4:]
        if ":" in p:
            name, altype = p.split(":", 1)
        else:
            name, altype = p, "Variant"
        out.append((name.strip(), altype.strip(), isvar))
    ret = ""
    r = re.search(r"## Return Value\n\*\[?O?p?t?i?o?n?a?l?\]? ?([A-Za-z][A-Za-z0-9]*)\*\s*\n&emsp;Type: \[([A-Za-z]+)\]", text)
    if r: ret = r.group(2)
    static = "An instance of the" not in text.split("## Parameters",1)[-1].split("*")[0:1] and \
             not re.search(r"\*%s\*\s*\n&emsp;Type: \[%s\]" % (re.escape(owner), re.escape(owner)), text)
    return dict(owner=owner, method=method, brief=brief, params=out, ret=ret, static=static,
                doc=md.name, dir=md.parent.name)

def build(typename):
    d = DOC/typename.lower()
    if not d.is_dir(): return None
    methods = []
    for md in sorted(d.glob("*-method.md")):
        m = parse(md)
        if m: methods.append(m)
    return methods

CANON = {}
EXISTING = {}
GENERATED = set()
UNKNOWN = set()
def canonical_names():
    out = {}
    for d in sorted(DOC.iterdir()):
        if not d.is_dir(): continue
        for md in d.glob("*-data-type.md"):
            m = re.search(r"^# ([A-Za-z][A-Za-z0-9]*) Data Type", md.read_text(errors="replace"), re.M|re.I)
            if m: out[d.name] = m.group(1)
        for md in d.glob("*-option.md"):
            m = re.search(r"^# ([A-Za-z][A-Za-z0-9]*) option type", md.read_text(errors="replace"), re.M)
            if m: out[d.name] = m.group(1)
    return out

def emit(name, methods, known):
    seen = set()
    decls, docs, bodies, named = [], [], [], set()
    for m in methods:
        ps = [(n, cpp_param(t, v), t) for n, t, v in m["params"]]
        sig = (m["method"], tuple(p for _, p, _ in ps), m["static"])
        if sig in seen: continue
        seen.add(sig)
        ret = cpp_return(m["ret"])
        def declarable(t):
            t = t.rstrip("[]")
            c = cpp_type(t).replace("::agiru::", "")
            return c in GENERATED and c != name
        for _, _, t in ps:
            if declarable(t): named.add(cpp_type(t).replace("::agiru::", ""))
        if declarable(m["ret"]): named.add(cpp_type(m["ret"]).replace("::agiru::", ""))
        args = ", ".join(f"{p} {n}" for n, p, _ in ps)
        al = f"{name}.{m['method']}(" + ", ".join(t for _, _, t in ps) + ")"
        doc = [f"  /// \\brief AL `{al}`." + (f" {m['brief']}" if m["brief"] else "")]
        for n, _, t in ps:
            doc.append(f"  /// \\param {n} The AL `{t}`.")
        if ret != "void":
            doc.append(f"  /// \\return The AL `{m['ret']}`.")
        doc.append("  /// \\throws Error always -- the surface is declared, the behaviour is not"
                   " (board:0035).")
        decls.append("\n".join(doc) + "\n  " + ("static " if m["static"] else "") +
                     f"{ret} {m['method']}({args});")
        callargs = "".join(f"  static_cast<void>({n});\n" for n, _, _ in ps)
        bodies.append(f"{ret} {name}::{m['method']}({args}) {{\n{callargs}  detail::RefuseDoor(\"{al}\");\n}}")
    used = set()
    for m in methods:
        for _, t, _ in m["params"]: used.add(t)
        used.add(m["ret"])
    needed = {x for t in used for x in re.findall(r"[A-Za-z][A-Za-z0-9]*", cpp_type(t))}
    extra = "".join(f'#include "{EXISTING[t]}"\n' for t in sorted(needed) if t in EXISTING)
    fwd = "".join(f"class {t};\n" for t in sorted(named))
    head = f'''#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"

{extra}
#include <string>
#include <string_view>

/// \\file
/// \\brief AL `{name}` -- the surface the platform documentation declares.

namespace agiru {{

{fwd}
/// \\brief AL `{name}`.
///
/// \\warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/{name.lower()}/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class {name} {{
public:
'''
    return head + "\n\n".join(decls) + "\n};\n\n} // namespace agiru\n", bodies
