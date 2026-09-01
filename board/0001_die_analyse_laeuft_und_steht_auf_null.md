Type: bug
State: open
Area: build
Tags: gate, owner

# `make lint` läuft, meldet 0 Funde und geht bei einer eingebauten Verletzung rot

**Der Zustand, der dieses Item nötig macht:** `.clang-tidy`, `test/lint.sh` und die Baseline sind
geschrieben, aber **nie ausgeführt worden** — auf dieser Maschine gab es beim Aufsetzen kein
`clang-tidy`. Eine Analysekonfiguration, die niemand gefahren hat, ist eine Vermutung über das
Verhalten eines Werkzeugs, nicht ein Gate.

## Referenz

**Plattform-Doku / AL-Quelltext**: berühren dieses Item nicht — es ist Werkzeug, nicht Semantik.

**Vorgänger (openerp)**: hatte keine statische Analyse. Was er stattdessen hatte, waren
`scripts/analysis/*.py` — 30 Werkzeuge, die dieselben Fragen NACHTRÄGLICH über den erzeugten
Bestand stellten (`silent_sinks`, `swallowed_errors`, `runtime_method_audit`). Jedes davon ist ein
Check, den ein Compiler oder ein Analysator vorher beantwortet hätte. Das ist der ganze
Erfahrungswert: die Prüfung, die der Baum sich selbst schreiben muss, ist die, die die Sprache
nicht mitbringt.

**outshine** ist die Herkunft der Baseline-Mechanik: „a strict analysis over a grown tree is red on
day one and switched off in the first week". Dort war der Baum gewachsen und die Baseline fiel von
12 979. Hier ist er leer, also ist der Weg nicht Absenken, sondern Halten.

**Die Wahl:** Baseline 0 ab Tag eins, keine Stufen, keine Ausnahme ohne Grund daneben. Möglich nur,
weil nichts zu erben ist.

## Wie

- `clang-tidy-19` installieren (`scripts/install.sh`).
- `make lint` fahren. Was `Main.cpp` an Funden erzeugt, IST das erste Ergebnis — nicht die
  Vermutung, dass es keine gibt.
- Jeder Fund ist eine Entscheidung: reparieren, oder der Check wandert mit **gemessener Fundzahl
  und Grund** in die Ausnahmeliste. Eine Ausnahme ohne Zahl gibt es nicht.
- `misc-include-cleaner` ist der wahrscheinlichste Streitfall. In outshine war er bei der
  Standardbibliothek scharf und bei allem anderen daneben. Hier steht er ohne `IgnoreHeaders` —
  wenn er sich hier ebenso verhält, kommt die Messung in die Zeile darüber und die Ausnahme
  darunter, in dieser Reihenfolge.

## Was wahr sein wird

- [ ] `make lint` meldet `0 Fund(e) über N Einheit(en), die Baseline erlaubt 0`.
- [ ] Jeder abgeschaltete Check trägt seine gemessene Fundzahl und seinen Grund über sich.
- [ ] Beweis: `clang-format --dry-run --Werror` und `clang-tidy` laufen beide über jede Datei
      außerhalb von `src/app/`, und die Unit-Zahl in `test/lint-baseline` ist die des Builds.
- [ ] **Gegenprobe**: eine Zeile `int x = 42;` in `Main.cpp` legen und verlangen, dass
      `make lint` mit `readability-magic-numbers` rot geht. Geht es grün, analysiert das Gate die
      Datei nicht — die erste Falle auf CLAUDE.mds Liste, und sie hat outshine eine Runde gekostet.
