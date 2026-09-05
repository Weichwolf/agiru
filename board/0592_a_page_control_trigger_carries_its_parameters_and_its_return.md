Type: root
Area: gen

# A page control trigger carries its parameters and its return

A control's trigger is emitted as `void X()` -- `src/gen/BodyWriter.cpp`, `ControlBodies` -- while
AL declares several of them WITH a signature:

| trigger | AL declaration (`triggers-auto/`) |
|---|---|
| `OnLookup` | `trigger OnLookup(var Text: Text): Boolean` |
| `OnAssistEdit` | `trigger OnAssistEdit(): Boolean` |
| `OnDrillDown` | `trigger OnDrillDown(): Boolean` |
| `OnValidate` | `trigger OnValidate()` |
| `OnAfterGetRecord` | `trigger OnAfterGetRecord()` |

Measured over `apps/` on 2026-09-05: the body of such a trigger already writes `exit(true)` and
`Text := ...`, and both are dropped on the floor -- the first silently, because a `return` in a
`void` function that AL wrote as `exit(Value)` compiles as `return;` only by accident of the
statement writer.

What it costs today: nothing compiles wrong, because the writer never emits the value. What it
costs the UI phase: a lookup that cannot say it handled the lookup, and a `var Text` the page
cannot write back -- both of which the TestPage tests read.

**The choice:** the trigger's declaration is in the AST already (`al::ProcedureDecl` under
`al::PageControl`), so the signature is `Returns(trigger, objects)` and `Parameters(trigger, ...)`
exactly as a page procedure gets them, and the runtime's caller has to pass and read them. That is
the same shape `Table<Derived>::Validate` already uses for a field trigger.

**Until then** the writer emits no `FallsOff` for a control trigger and its bare `exit` returns
nothing, which is consistent with the void signature and refuses nothing silently.
