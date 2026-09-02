Type: root
State: open
Area: gen, rt
Tags: active

# An object member is a lazy handle, and the containment cycle stops being a cycle

`Background Error Handling Mgt.` declares `ItemJournalErrorsMgt: Codeunit "Item Journal Errors
Mgt."` and `Item Journal Errors Mgt.` declares `BackgroundErrorHandlingMgt: Codeunit "Background
Error Handling Mgt."`. Each holds the other. As C++ members by value that is a class of infinite
size, and the compiler says so: `field has incomplete type`.

## Reference

**THIS ITEM WAS FILED, DELETED ON A FALSE PREMISE, AND IS FILED AGAIN.** Its first form said a
codeunit variable is a handle. It was closed with the reasoning that AL has no containment cycle and
that the cycle was in the INCLUDES -- true of `Language`/`LanguageImpl`, which was the pair
examined, and false of the tree. The pair above is a real one. Grepping one example and
generalising is what went wrong, and it is written down here rather than quietly repaired.

**The platform documentation settles the semantics and not merely the workaround.** A codeunit
variable is an instance the platform creates; two codeunits that name each other are ordinary AL and
the BaseApp is full of them. So AL cannot be constructing eagerly either -- an eager pairing would
not terminate. **By value is therefore WRONG about AL, not merely inconvenient in C++**, and that is
what decides this rather than the compiler's complaint.

**Predecessor**: openerp holds codeunit variables as Python attributes, which are references, and
never meets the question. It pays for it elsewhere -- nothing tells it whether two variables are the
same instance -- which is the thing to keep right here.

**AND THE RULE IS WIDER THAN THE FIRST FORM SAID.** It was filed for CODEUNIT members, on the
argument that two codeunits may name each other and two tables may not. The tables disproved that
the same day: `Currency Exchange Rate` declares a variable of its OWN type, and `Currency` holds a
`Currency Exchange Rate` that holds one back. A Record member recurses exactly as a codeunit member
does. So the rule is one rule -- a member of OBJECT type is a handle -- and the narrow form is
recorded here as refuted rather than quietly widened.

## How

- An object MEMBER becomes `Instance<T>`: a pointer that news `T` on first use and frees it with a
  function pointer captured at that moment, so the destructor never needs `T` complete. The header
  then needs only a forward declaration and the cycle is gone from the include graph as well.
- `Instance<T>` converts implicitly to `T&`. The handle is how agiru DECLARES the member, not
  something AL code mentions, so it disappears at every use but the one C++ cannot hide.
- A procedure LOCAL stays by value. It lives inside a body, where every type is complete and no
  cycle can form, and by value is the shape a reader expects.
- The call site becomes `->`, the same visible deviation an interface variable already carries
  (board:0027). AL writes `.` and C++ cannot; a handle that pretended otherwise would be the clever
  kind of wrong this tree refuses.

## What will be true

- [x] `apps/` holds no object member by value, and the two codeunits above compile.
- [ ] A gate case states the pairing: two codeunits naming each other translate and run.
- [ ] `Instance<T>` frees what it made, proven by a counter rather than by reading the code.
- [ ] **Negative control**: emit the members by value again and require the gate to go red.
