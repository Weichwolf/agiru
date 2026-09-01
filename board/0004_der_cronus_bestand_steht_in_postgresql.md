Type: arc
State: open
Area: db
Tags: provision, owner

# Der CRONUS-Bestand steht in PostgreSQL und stammt nachweislich aus 28.4

`make provision` holt das Artefakt, spielt es in SQL Server ein und stellt die PostgreSQL bereit.
Der letzte Schritt fehlt: die Daten sind noch nicht drüben, weil das Zielschema erst der Transpiler
erzeugt.

## Referenz

**Gemessen 2026-09-01, ohne Download** — über einen Range-Request auf das zentrale
Verzeichnis des Zip:

| | |
|---|---|
| CDN | `bcartifacts-exdbf9fwegejdqak.b02.azurefd.net` — der Blob-Host antwortet nicht mehr (`AuthorizationFailure`, network security perimeter), `bcartifacts.azureedge.net` löst nicht auf |
| neuestes OnPrem-Artefakt | `28.4.53241.0/w1`, 372 706 292 B — es gibt kein 29.x und kein 30.x |
| darin | `database/Demo Database BC (28-0).bak`, 824 299 520 B |
| BCApps `main` | trägt `30.0.0.0` — **dafür gibt es keine Demo-Datenbank** |
| BCApps `releases/28.4` | trägt `28.4.0.0` — das ist das Paar |

Die Listung des Containers ist über Front Door **nach Pfad und nicht nach Query gecacht**: zwei
Anfragen auf `/onprem?...` mit verschiedenen `prefix` liefern dieselbe Antwort. Wer die
Versionsliste braucht, variiert den PFAD (`//onprem/`), nicht die Query. Das hat hier eine
Viertelstunde gekostet und steht deshalb hier.

**Plattform-Doku**: BC legt je Mandant `<Company>_$<Tabelle>$<AppGuid>` ab, Spalten in BCs
SQL-Kodierung des AL-Feldnamens (`No.` → `No_`). Tabellen mit `DataPerCompany=No` stehen ohne
Mandantenpräfix.

**Vorgänger (openerp)**: `scripts/setup/cronus_bak_loader.py` löst genau diese Abbildung — generisch,
ohne AL-Objektnamen: Tabelle über `al_name_to_snake` mit Collapse-Fallback, Spalte über
Collapse-Vergleich, BC-Nulldatum `1753-01-01` → leer. **Diese Datei ist die Vorlage und ihre
Kommentare sind bezahlte Tage.** Was sie außerdem zeigt: der zweite Lauf ging über einen
committeten `pg_dump`, nicht über SQL Server — der Container ist eine Durchreiche, kein Bestandteil
des Testlaufs.

**Die Wahl:** Übertragung über `bcp` heraus und `COPY` hinein, nicht über einen Datenbanktreiber.
Ein Treiber wäre eine Abhängigkeit für einen Vorgang, der genau einmal je Release läuft, und `bcp`
liegt im Image, das ohnehin da ist.

## Was wahr sein wird

- [ ] Jede Tabelle des transpilierten Schemas, die in CRONUS Zeilen hat, hat sie danach auch in
      PostgreSQL, mit derselben Zeilenzahl.
- [ ] Die Abbildung nennt **keinen** AL-Objektnamen — sie ist Namensregel, nicht Tabelle.
- [ ] Was nicht abgebildet werden konnte, steht als Liste da und wird nicht verschwiegen: eine
      stille Auslassung ist der Fehler, der als Runtime-Defekt zurückkommt.
- [ ] `agiru_master` ist danach schreibgeschützt und Vorlage; ein Lauf klont ihn über
      `CREATE DATABASE ... TEMPLATE`.
- [ ] Beweis: Zeilenzahlen je Tabelle auf beiden Seiten, und die Summe der Beträge einer
      Postentabelle Ziffer für Ziffer gleich (hängt an board:0002).
- [ ] **Gegenprobe**: eine Spalte aus der Abbildung nehmen und verlangen, dass der Vergleich rot
      geht. Ein Vergleich, der nur Zeilen zählt, merkt eine fehlende Spalte nicht.
