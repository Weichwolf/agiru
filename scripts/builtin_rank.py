#!/usr/bin/env python3
"""Which AL builtins the UT milestone actually calls, ranked by how many test methods stand on them.

A BUILTIN IS A CALL WITH NO RECEIVER. `Format(X)` and `StrSubstNo(A, B)` are the platform's; every
other call in an AL body goes through a variable, and that variable's type decides what it is. So
the ranking is: bare `Name(` in a body, minus the object's own procedures, minus AL's keywords.

The population is the milestone's -- the 86 codeunits whose name ends in UT under W1/Tests -- and
the unit is the [Test] METHOD, because that is what the milestone counts.
"""
import collections
import pathlib
import re
import sys

ROOT = pathlib.Path.home() / "Git/BCApps/src/Layers/W1/Tests"

# AL's own words, which are not calls even when a parenthesis follows.
KEYWORDS = {
    "if", "then", "else", "begin", "end", "case", "of", "repeat", "until", "while", "do", "for",
    "to", "downto", "foreach", "in", "exit", "with", "var", "procedure", "trigger", "local",
    "internal", "protected", "and", "or", "not", "xor", "div", "mod", "asserterror", "true",
    "false", "array", "record", "codeunit", "page", "report", "query", "xmlport", "enum",
    "interface", "temporary", "label", "text", "code", "integer", "decimal", "boolean", "date",
    "time", "datetime", "duration", "guid", "option", "blob", "bigtext", "recordid", "recordref",
    "fieldref", "variant", "dotnet", "list", "dictionary", "testpage", "testrequestpage", "return",
    "implements", "extends", "namespace", "using", "pragma",
}


def stripped(body):
    """What is left when the STRINGS and the COMMENTS are gone.

    A word inside a message reads like a call to a regular expression and is not one: `Format` in
    'You must Format(X) first' would be counted, and so would every English word before a bracket.
    """
    body = re.sub(r"//[^\n]*", " ", body)
    return re.sub(r"'(?:[^']|'')*'", " ", body)


def bodies(text):
    """Every [Test] method's body: `begin` to the matching `end;`, attributes excluded.

    THE ATTRIBUTES ARE NOT THE BODY. `[Scope('OnPrem')]`, `[HandlerFunctions('...')]` and
    `[TransactionModel(...)]` read as calls to anything counting `Name(` -- 1 667, 576 and 106 of
    them -- and they are declarations about the test rather than anything it calls.
    """
    out = []
    for m in re.finditer(r"\[Test\]", text):
        start = text.find("begin", m.end())
        if start < 0:
            continue
        out.append(stripped(text[start:text.find("\n    end;", start)]))
    return out


def main():
    files = sorted(p for p in ROOT.rglob("*.Codeunit.al")
                   if re.search(r'^\s*codeunit\s+\d+\s+"?[^"\n]*UT"?\s*$',
                                p.read_text(errors="replace"), re.M | re.I))
    methods = 0
    calls = collections.Counter()
    for path in files:
        text = path.read_text(errors="replace")
        # The object's OWN procedures are not builtins, however bare the call looks.
        own = {m.lower() for m in re.findall(r"(?im)^\s*(?:local\s+|internal\s+)?procedure\s+(\w+)",
                                             text)}
        for body in bodies(text):
            methods += 1
            seen = set()
            for m in re.finditer(r"(?<![\w.\"])([A-Za-z]\w*)\s*\(", body):
                name = m.group(1)
                if name.lower() in KEYWORDS or name.lower() in own:
                    continue
                seen.add(name)
            for name in seen:
                calls[name] += 1
    print(f"population {len(files)} UT codeunits, {methods} [Test] methods")
    print()
    print("the builtins they call, by how many [Test] methods stand on each")
    for name, count in calls.most_common(40):
        print(f"  {count:>5}  {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
