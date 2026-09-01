# agiru

An AL-to-C++ transpiler and runtime for Microsoft Dynamics 365 Business Central. The BaseApp is not
wrapped, it is translated: 9 300 AL objects, 2.56 million lines, into C++23.

The target it is built against: **`agiru` and PostgreSQL run on a Raspberry Pi 5 with 16 GB -- four
out-of-order cores at 2.4 GHz, aarch64 -- and they run fast.**

The tree is new. `CLAUDE.md` is the constitution, `board/` is the state of the work.

```
make provision   # BC demo database from the Microsoft CDN, SQL Server, PostgreSQL
make             # library, transpiler, client
make lint        # format and static analysis, baseline 0
make test        # the gate
make help        # the list
```

Prerequisites: see `scripts/install.sh`. The BC version is pinned in `BC_VERSION` and must match the
checked-out BCApps source -- `make provision` refuses otherwise.
