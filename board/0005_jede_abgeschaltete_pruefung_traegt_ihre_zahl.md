Type: bug
State: open
Area: build
Tags: gate, measured

# Jede abgeschaltete Prüfung trägt ihre gemessene Fundzahl, nicht nur ihren Grund

`.clang-tidy` schaltet vier Prüfungen ab und sagt bei jeder, WARUM. Was keine von ihnen trägt, ist
eine ZAHL — und die Datei sagt das selbst: „Es stehen keine Fundzahlen neben den Ausnahmen, weil
hier noch nichts zu zählen war."

Das ist heute richtig und wird falsch, sobald `src/` Volumen hat. Ein Grund ohne Zahl ist eine
Behauptung über die Größe eines Problems, das niemand gemessen hat.

## Referenz

**outshine** misst jede Ausnahme über zwei repräsentative Übersetzungseinheiten, bevor sie gewählt
wird, und schreibt die Zahl in die Zeile darüber: `modernize-use-trailing-return-type` (897),
`readability-uppercase-literal-suffix` (194). Der Wert liegt nicht in der Zahl selbst, sondern
darin, dass sie eine Ausnahme UNBEQUEM macht: wer 3 667 Funde abschaltet, sieht, dass er es tut.

**Der Gegenbeleg im selben Baum:** dort standen `modernize-avoid-c-style-cast` (91) und
`modernize-avoid-c-arrays` (281) als richtig anerkannt und trotzdem aus, mit „Stufe zwei" als
Grund. Eine Stufe ohne Datum ist ein Versprechen; erst die Zahl war der Termin.

**Die Wahl:** dieselbe Mechanik, aber die Messung kommt NACH dem Volumen. Zwei repräsentative
Einheiten aus verschiedenen Stufen — eine aus `src/gen` (Emitter, viel Zeichenkettenarbeit) und
eine aus `src/rt` (Semantik, viel Verzweigung).

**Was schon gemessen ist** (2026-09-01, eine Einheit, `src/cli/Main.cpp`): mit allen sieben
Kategorien an und den vier Ausnahmen aus fielen **acht** Funde an, alle echt, alle repariert —
`modernize-avoid-c-arrays`, `modernize-use-designated-initializers` (4×), `concurrency-mt-unsafe`,
`readability-implicit-bool-conversion`, `bugprone-exception-escape`. Keiner davon war Geschmack.
Das ist die Stichprobe, die die Kategorienauswahl stützt — und sie ist zu klein, um eine Ausnahme
zu stützen.

## Was wahr sein wird

- [ ] Jede in `.clang-tidy` abgeschaltete Prüfung trägt ihre gemessene Fundzahl über sich, mit der
      Population, an der gemessen wurde.
- [ ] Eine Ausnahme, deren Zahl klein ist, ist keine Ausnahme mehr, sondern eine Reparatur.
- [ ] Die vier heutigen Ausnahmen sind einzeln geprüft: bleibt jede Geschmack, wenn sie an echtem
      Volumen gemessen wird?
- [ ] Beweis: die Messung wird mit dem Commit zitiert, nicht neu hergeleitet — eine Zahl, die in
      einer Datei steht, muss stimmen, bevor sie hineingeschrieben wird.
