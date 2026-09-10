# `OnQueryClosePage` answers true unless the trigger says otherwise

**Finding (2026-09-10).** `Workflow Templates.OnQueryClosePage` only raises on a bad lookup and
never exits a value; the generated trigger returned `{}` -- false -- and `ClosePage` refused
"the page refused to close (OnQueryClosePage)" for every clean close (5 UT cases across WF Buffer
Table/Page UT and the payment registration codeunits, chain 87).

**Reference.** `devenv-onqueryclosepage-page-trigger.md`: "The default value is **true**."

**Choice.** The generator's implicit exit and the named return's initial value are `true` for
that trigger (`DefaultsToTrue` beside `IsTryFunction`), which is the only trigger the platform
documents with a true default. No gate yet: the page-generation gates have no page fixture; the
first one carries this case.
