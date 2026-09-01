Type: root
State: open
Area: rt
Tags: invariant, measured

# Jeder Betrag steht in einem `Decimal`, das die BC-Arithmetik Ziffer für Ziffer trägt

CLAUDE.md nennt es als Invariante: kein binärer Fließkommatyp trägt einen Betrag. Was diese
Invariante KOSTET, ist noch nicht gemessen, und bis dahin ist sie eine Absicht.

## Referenz

**Plattform-Doku**: `methods-auto/decimal/` und die Typseite beschreiben `Decimal` als den
AL-Typ für Beträge; `ROUND` mit `Precision` und `Direction` ist eigenständig dokumentiert
(`methods-auto/system/system-round-*-method.md`). **Die Rundung ist Teil der Semantik, nicht des
Ausgabeformats** — BC rundet an definierten Stellen im Buchungsfluss, und wo gerundet wird,
entscheidet das Ergebnis.

**AL-Quelltext**: BaseApp rechnet Beträge durchweg in `Decimal` und rundet über
`Currency."Amount Rounding Precision"`. Die Genauigkeit ist ein DATENFELD, keine Konstante.

**SQL**: BC legt Decimal-Spalten als `decimal(38,20)` ab. Zur Laufzeit ist der Typ .NET-`decimal`
— 96-Bit-Mantisse, Skala 0–28, dezimal. Die beiden sind nicht dasselbe, und welcher von beiden das
Soll ist, ist genau die offene Frage.

**Vorgänger (openerp)**: hat Pythons `decimal.Decimal` benutzt und damit die Frage umgangen — die
Sprache brachte den Typ mit. Der Preis stand in der Laufzeit, nicht in der Korrektheit. In C++ gibt
es den Typ nicht, also ist er zu bauen und der Preis ist die Bauzeit.

**Die Kandidaten:**

| Darstellung | trägt `decimal(38,20)` | Division | Kosten |
|---|---|---|---|
| `__int128` als Festkomma, Skala 20 | knapp — 10^38 braucht 127 Bit | eigener Algorithmus | am schnellsten, am engsten |
| 128-Bit-Dezimal (IEEE 754-2008 `_Decimal128`) | ja, 34 Ziffern | vom Compiler | gcc/clang können es, libstdc++ bindet es nicht an `<format>` |
| eigenes 256-Bit-Festkomma | mit Reserve | eigener Algorithmus | am langsamsten, am sichersten |

Keine dieser Zeilen ist gemessen. Das ist der Punkt des Items.

## Wie

- Die Nagelprobe ist **nicht** ein Mikrobenchmark, sondern ein Buchungslauf: Beträge werden
  hunderttausendfach addiert, multipliziert und gerundet, und der Fehler akkumuliert oder nicht.
- Die Vergleichsgröße ist der CRONUS-Bestand selbst (board:0004): dieselben Belege, einmal in
  SQL Server als `decimal(38,20)` gespeichert. Jeder Wert, den unsere Arithmetik erzeugt, muss dem
  gespeicherten gleichen — nicht nahe kommen.
- Reihenfolge: erst der Typ mit seinem Beweis, dann alles, was auf ihm steht. Ein Wertetyp muss
  vollständig sein, bevor darauf gebaut wird; openerps Backlog nennt das als eigenes Fehlermuster.

## Was wahr sein wird

- [ ] `Decimal` trägt jeden Wert, den eine BC-`decimal(38,20)`-Spalte tragen kann, verlustfrei
      hin und zurück.
- [ ] `ROUND` bildet die dokumentierte AL-Semantik ab, einschließlich `Direction` und
      benutzerdefinierter `Precision`.
- [ ] Ein Überlauf ist LAUT — die stille Rundung, die einen Betrag um eine Stelle verschiebt, ist
      genau der Defekt, den diese Invariante ausschließen soll.
- [ ] Beweis: die Beträge des CRONUS-Bestands aus SQL Server, durch unsere Arithmetik und zurück,
      Ziffer für Ziffer gleich; die Population steht neben dem Ergebnis.
- [ ] **Gegenprobe**: `Decimal` einmal auf `double` legen und verlangen, dass der Beweis rot geht.
      Geht er grün, prüft er die Arithmetik nicht, sondern nur, dass sie läuft.
