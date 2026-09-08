Type:     task
Status:   open
Area:     net, gen
Source:   the UT ranking after board:0616, 2026-09-08
Class:    activation

# The rebuilt .NET types find themselves, and `DateTimeOffset` is one of them

**321 UT FAILURES ARE ONE .NET MEMBER: `DateTimeOffset.Now`** (measured 2026-09-08, the largest
single entry in the ranking by a factor of one and a half).

It reaches them through an AL codeunit rather than directly. `Codeunit DotNet_DateTimeOffset` is
BC's own wrapper -- it is transpiled, it holds a `DotNet DateTimeOffset`, and
`ConvertToUtcDateTime` is what the BaseApp calls:

```AL
DotNetDateTimeOffsetSource := DotNetDateTimeOffsetSource.DateTimeOffset(DateTimeSource);
DotNetDateTimeOffsetNow := DotNetDateTimeOffsetNow.Now;
exit(DotNetDateTimeOffsetSource.LocalDateTime - DotNetDateTimeOffsetNow.Offset);
```

The whole surface, measured over BCApps: `ConvertToUtcDateTime` 34, `GetOffset` 6, `Parse` 6,
`ToString` 2, and one each of `ToUnixTimeSeconds`, `ToUnixTimeMilliseconds`, `ToLocalTime`,
`SetDateTimeOffset`, `FromUnixTimeSeconds`, `DateTimeOffset`, `DateTime`.

## The time zone is the decision, and the documentation makes it

`devenv-about-dates.md`: **"stores ALL `DateTime` fields as UTC and in the UI layer, we convert
these fields to the timezone, specified by the user on the User Settings page"**, and **"`Date`
fields are NEVER converted per time zone"**.

**SO A SESSION'S ZONE IS A USER SETTING, AND THIS RUNTIME HAS NO USER SETTINGS YET.** The honest
value is therefore UTC: `LocalDateTime` is the instant, `Offset` is zero, `ToLocalTime` is the
identity. That is not a placeholder -- it is what a session whose zone is UTC answers, and it is
DETERMINISTIC, which reading the host's zone would not be. When the UI brings the user's setting
(phase 2), this class reads it in one place.

The predecessor never got here: its `TimeZoneInfo` item (openerp WI-979) is still open, and its
comment names the same two decisions -- does the column store UTC, and does `CurrentDateTime`
answer the session's zone or the server's.

## What blocks it is a LIST SOMEBODY HAS TO REMEMBER TO FILL

`src/tc/Main.cpp` carries `Rebuilt()`, five names by hand, and the stub generator skips those. That
is CLAUDE.md's own trap: **it must FIND ITSELF** from the class declarations in `include/dotnet/`,
and an empty result is an ABORT. Until it does, adding a rebuilt .NET type means editing a list in
a second place and the stub generator emits a duplicate of the class.

**AND A MEMBER MAY CARRY THE CLASS'S OWN NAME ONLY WHILE THE CLASS DECLARES NO CONSTRUCTOR.** AL
spells .NET's constructor `X := X.DateTimeOffset(dt)`, so the rebuilt class needs a member named
`DateTimeOffset` -- which C++ forbids as a member FUNCTION outright and allows as a data member
only in a class with no user-declared constructor. `AbsentType` already carries that note for the
same reason.

## What proves it

`DotNet_DateTimeOffset.ConvertToUtcDateTime` returns its argument for a UTC session instead of
refusing, and the ranking loses its largest entry. The negative control is `Rebuilt()`: with
`DateTimeOffset` in `include/dotnet/` and the list not finding it, the stub generator emits a
second `dotnet::DateTimeOffset` and the tree does not compile -- which is the loud failure the
hand-maintained list does NOT give today.
