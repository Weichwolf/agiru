# agiru

AL→C++-Transpiler und Runtime für Microsoft Dynamics 365 Business Central. Die BaseApp wird nicht
angebunden, sondern übersetzt: 9 300 AL-Objekte, 2,56 Mio. Zeilen, nach C++23.

Der Baum ist neu. `CLAUDE.md` ist die Verfassung, `board/` der Arbeitsstand.

```
make provision   # BC-Demo-Datenbank vom Microsoft-CDN, SQL Server, PostgreSQL
make             # Bibliothek, Transpiler, Client
make lint        # Format und statische Analyse, Baseline 0
make test        # das Gate
make help        # die Liste
```

Voraussetzungen: siehe `scripts/install.sh`. Die BC-Version ist in `BC_VERSION` gepinnt und muss
zur ausgecheckten BCApps-Quelle passen — `make provision` verweigert sonst.
