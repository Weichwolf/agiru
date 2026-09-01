Type: arc
State: open
Area: test
Tags: gate

# Das Gate fährt Fälle und geht rot, wenn einer fällt

`make test` meldet heute rot, weil `test/gate/` leer ist. Das ist die richtige Farbe für ein leeres
Gate und keine Dauerlösung.

## Referenz

**Plattform-Doku / AL-Quelltext**: nicht betroffen.

**Vorgänger (openerp)**: „KEINE separate Test-Architektur" — Tests sind `Subtype=Test`-Codeunits,
der Runner ist eine `Subtype=TestRunner`-Codeunit, beide laufen über die NORMALE Runtime. Das ist
die Entscheidung, die dort am meisten getragen hat: die BC-Testsuite ist damit kein Fremdkörper,
sondern Last auf demselben Pfad, den ein Anwender fährt.

**Sie gilt hier unverändert — aber sie kommt später.** Sie setzt eine Runtime voraus, die
Codeunits ausführt. Was jetzt gebraucht wird, ist die Ebene darunter: Fälle über C++-Einheiten, die
noch keine AL-Semantik haben (`Decimal`, Lexer, Filter-Parser).

**Die Wahl:** zwei Ebenen, klar getrennt.

| Ebene | was sie fährt | wann |
|---|---|---|
| `test/gate/` | C++-Einheiten, ohne Datenbank, ohne AL | jetzt |
| `test/al/` | transpilierte AL-Test-Codeunits über die Runtime | sobald die Runtime Codeunits fährt |

**Kein Test-Framework von der Stange.** Ein Fall ist eine `main`-Funktion, die rot oder grün
zurückgibt; der Runner ist `test/run.sh`. Eine Abhängigkeit, die nur Makros für `assert` liefert,
ist eine Abhängigkeit zu viel — und CLAUDE.md sagt, was der Compiler entscheiden kann, ist ohnehin
ein `static_assert` und gar kein Fall.

## Was wahr sein wird

- [ ] `make test` fährt jeden Fall unter `test/gate/` und meldet die Zahl.
- [ ] Ein fallender Fall macht `make test` rot, mit Datei, Zeile und dem Wert, der nicht stimmte.
- [ ] Ein Fall, der gar nicht gebaut werden kann, ist rot und nicht übersprungen.
- [ ] Beweis: der erste Fall ist der von board:0002 — er hat ohnehin gebaut zu werden.
- [ ] **Gegenprobe**: einen Fall absichtlich falsch stellen und verlangen, dass `make test` rot
      geht. Ein Runner, der nur zählt und nie prüft, zählt auch bei einem falschen Wert weiter.
