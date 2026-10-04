# agiru

An AL-to-C++ transpiler and runtime for Microsoft Dynamics 365 Business Central. The BaseApp is not
wrapped, it is translated: 9 300 AL objects, 2.56 million lines, into C++23.

The target it is built against: **`agiru` and PostgreSQL run on a Raspberry Pi 5 with 16 GB -- four
out-of-order cores at 2.4 GHz, aarch64 -- and they run fast.**

agiru-owned code is [MIT licensed](LICENSE), with no license keys, trials, paid tiers or
license-based feature restrictions. O365/Microsoft 365 and integrations with other
Microsoft cloud services are outside the product scope; core ERP, user/company permissions
and generic interfaces remain required. Third-party components retain their licenses.

The tree is new. `AGENTS.md` defines the project contracts; `board/` is the state of the work.

```
make provision   # BC demo database from the Microsoft CDN, SQL Server, PostgreSQL
make             # library, transpiler, client
make lint        # format and static analysis, baseline 0
make test        # the gate
make help        # the list
```

Prerequisites: see `scripts/install.sh`. The BC version is pinned in `BC_VERSION` and must match the
checked-out BCApps source -- `make provision` refuses otherwise.

XML uses system libxml2 (MIT); JSON uses system yyjson (MIT, Debian `libyyjson-dev`).
The libraries remain private implementation dependencies; packaging must retain their notices.
