# agiru

AL→C++-Transpiler und Runtime für Business Central. Die BaseApp wird nicht angebunden, sie wird
ÜBERSETZT: 9 300 AL-Objekte, 2,56 Mio. Zeilen AL aus `~/Git/BCApps/`, nach C++23, mitsamt der
Geschäftslogik. Ergebnis ist ein eigenständiges ERP auf einem Prozess und einer PostgreSQL.

## Warum C++ und nicht die Sprache, in der das schon einmal versucht wurde

`~/Git/openerp/` ist derselbe Versuch in Python: 63 k Zeilen Runtime, 3,3 Mio. Zeilen generiert,
ein Backlog mit gemessenen Reverts. Er ist die dritte Referenz dieses Baums und seine Niederlagen
sind bezahlte Tage. Drei davon sind der Grund für den Sprachwechsel:

- **AL ist statisch getypt und Python ist es nicht.** Ein AL-`Record` ist ein Satz benannter Felder
  fester Typen; in Python wurde er ein Descriptor-Dict, und jeder Typfehler fiel erst zur Laufzeit
  auf — in einem Testlauf von Stunden. In C++ ist eine Tabelle eine generierte Klasse mit
  getypten Feldern; derselbe Fehler ist ein Compilerfehler in Sekunden.
- **Die .NET-Typen sind Klassen, keine Brücken.** AL spricht `System.Text.StringBuilder`,
  `System.IO.MemoryStream`, `System.Xml.XmlDocument`. openerp hat sie als `dotnet_*.py`-Brücken
  auf Python-Bibliotheken abgebildet und an der Semantik-Differenz geblutet. Hier werden sie
  NACHGEBAUT — eine C++-Klasse je .NET-Klasse, mit dem Verhalten, das die .NET-Doku beschreibt.
  Das ist mehr Arbeit einmal und keine Arbeit danach.
- **Ein Prozess kostete ein Gigabyte.** openerps App-Image ist ~1 GB je Prozess, was Fork
  ausschloss, Threads erzwang und die Testparallelität auf zwei Worker deckelte. Ein übersetztes
  Programm trägt seinen Code im Textsegment und teilt ihn zwischen Prozessen ohne Zutun.

Was von openerp ÜBERNOMMEN wird, steht unten unter „Die Doku ist die Spezifikation" und
„Jeder Fehler ist ein generischer Gap". Beides ist dort gemessen worden und gilt hier unverändert.

## Stand

Der Baum ist LEER. Es gibt einen Bauplan, ein Gate, ein Board und keinen Transpiler. Was hier steht,
ist die Verfassung, gegen die der erste Code geschrieben wird — nicht die Beschreibung von etwas
Bestehendem. Wo eine Zahl fehlt, steht kein Platzhalter, sondern ein Board-Item.

## Die Referenzen

Jede Frage nach Soll-Verhalten hat drei Quellen, in dieser Reihenfolge:

| # | Quelle | beantwortet |
|---|---|---|
| 1 | **Plattform-Doku** `~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer/` (4 386 MD) | was die PLATTFORM garantiert — Validate-Reihenfolge, Trigger-Lebenszyklus, Transaktionsverhalten, Systemfelder. Steht im AL-Quelltext NICHT |
| 2 | **AL-Quelltext** `~/Git/BCApps/src/` | was die BaseApp TUT — die Verwendung, nie die Garantie |
| 3 | **Anwender-Doku** `~/Git/dynamics365smb-docs/` (2 802 MD) | was der Anwender erwartet — das fachliche Soll |

Dazu `~/Git/openerp/` als **vierte, gemessene Referenz**: dieselbe Semantik einmal implementiert,
mit Backlog-Kommentaren zu widerlegten Hypothesen. Vor jeder nicht-trivialen Semantik dort greppen.

Wo was steht:

| gesucht | Ort |
|---|---|
| Methode eines Typs | `methods-auto/<typ>/<typ>-<methode>[-<argtypen>]-method.md` (1 876 Dateien, 135 Typen) |
| Überladungen | eigene Datei je Signatur — `record-insert--method.md`, `record-insert-boolean-method.md`, `record-insert-boolean-boolean-method.md` |
| Objekt-/Feld-Eigenschaft | `properties/devenv-<name>-property.md` (349) |
| Trigger | `triggers-auto/` (152) |
| Attribut (`[EventSubscriber]`, `[TryFunction]`) | `attributes/` (41) |
| Compiler-Diagnose | `diagnostics/` (907) |
| Konzepte | `devenv-*.md` im Wurzelverzeichnis |

**Die Überladungs-Dateinamen sind der Schlüssel.** Ein Verhalten hängt oft am ARGUMENT, nicht am
Methodennamen. openerp hat daran drei Reverts bezahlt: die SystemId-Regel steht in
`record-insert-boolean-boolean-method.md`, nicht in der Datei daneben.

### Die Doku ist die Spezifikation

Die Abdeckung der BC-Testsuite ist unbekannt, die Doku ist vollständig. Was die Doku beschreibt,
muss die Runtime können, unabhängig davon, ob ein Test es abfragt.

1. **Namensgleichheit mit AL ist Architektur-Invariante.** Typ, Methode und Parameter heißen wie in
   AL. Nur so ist der Doku-Abgleich mechanisch: ein Typ, der anders heißt, bricht ihn für ALLE
   seine Methoden. openerp hat sich `Record`→`Table`, `RecordRef`→`_RecordRefProxy`, `List`→`AlList`
   geleistet und den Abgleich damit für jede dieser Klassen verloren. Hier heißt `Record` `Record`.
2. **Ein dokumentiertes Verhalten ohne Gate-Test ist eine Lücke**, auch wenn kein AL-Test darauf
   fällt.
3. **Der Vollständigkeitsmesser ist ein Zähler mit Baseline** — Doku-Syntaxblock gegen C++-Signatur
   über alle 135 AL-Typen. Er misst NICHT, ob eine vorhandene Signatur das Richtige tut.

### Jeder Fehler ist ein generischer Gap

Transpiler und Runtime kennen **keine** konkreten AL-Objekte. Also kann kein Fehler
„reservierungs-" oder „verkaufsspezifisch" sein. Ein scheiternder Test zeigt ein unvollständig
implementiertes generisches AL-Primitiv — Builtin, Trigger-Semantik, Event-Dispatch, FlowField,
TableRelation. Ist AL zu 100 % generisch implementiert, passen alle Tests. Ein AL-Objektname, der
in `src/` außerhalb von `src/app/` auftaucht, ist ein Befund und kein Fix.

## Die Handwerksregeln

C++-Wahrheiten, keine agiru-Entscheidungen. Sie bewegen sich nicht.

- **C++23**, `-Wall -Werror -Wpedantic`; eine Warnung IST ein Fehler. `clang++-19` ist der
  Referenz-Compiler, `g++-14` muss denselben Baum übersetzen — zwei Frontends finden verschiedene
  Fehler, und der zweite kostet nur Rechenzeit
- **Was der Compiler entscheiden kann, ist ein `static_assert`, nie ein Testfall.** Feldzahl,
  Layout, Enum-Vollständigkeit, Vollständigkeit eines Katalogs. Der Transpiler EMITTIERT diese
  Zusicherungen: eine TableRelation, deren Ziel nicht existiert, ist ein Übersetzungsfehler und
  keine Laufzeitmeldung. Das ist der Hauptgewinn gegenüber Python und wird nicht verschenkt
- **Das Typsystem vor Prüfern**: `std::span`/`std::string_view` an Grenzen, `std::expected` wo eine
  Verweigerung ihren Grund trägt, starke Typen statt `int` für alles, was eine Bedeutung hat
  (`TableId`, `FieldNo`, `EntryNo`). AL vertauscht sie sonst still
- **`private` ist Default**, eine breitere Tür begründet sich. Ein öffentliches Datenmember ist eine
  Invariante, die niemand halten kann
- **Kein Kommentar, der WAS sagt.** Code und Name erklären sich selbst; ein Kommentar, der die
  Zeile daneben wiederholt, ist dieselbe Aussage in zwei Sprachen und driftet weg. Nur ein
  nicht-offensichtliches WARUM rechtfertigt einen, dann eine Zeile. `include/` ist die Tür und
  trägt Doxygen; der Rest von `src/` trägt Prosa nur im Beweis
- **Ein Name ist ein Versprechen.** Ein Wort, das in AL etwas anderes heißt, gibt das Wissen des
  Lesers gegen ihn aus. Die AL-Vokabeln sind Gesetz — Record, FieldRef, Codeunit, Trigger, Validate,
  Filter, Key, Flowfield, Dimension
- **Jede Zahl trägt ihre Herkunft** (`abgeleitet` · `gemessen` · `[GESETZT]`) mit Einheit und
  Population. Eine nackte Konstante im Code ist ein Befund
- **Eine Diagnose ist ein deklariertes Label**, nie ein freies Literal — die Fehlermeldungen einer
  Datei stehen oben beisammen und lesen sich als Liste. AL-Fehlertexte sind Teil des
  Soll-Verhaltens: Tests vergleichen sie
- **Ein Fehlschlag ist laut.** Eine Deklaration anzunehmen und nichts damit zu tun ist schlimmer
  als sie abzulehnen. `catch (...) {}` ist ein Befund mit Zähler
- **Artefakte nach `build/` oder ins System-Temp**, nie in den Baum. `compile_commands.json` ist die
  Ausnahme, weil clangd es an der Wurzel sucht; es ist gitignored

## Die Invarianten

Vier Festlegungen. Alles andere darf ein Item revidieren, diese nicht.

- **KEIN BINÄRER FLIESSKOMMATYP TRÄGT EINEN BETRAG.** AL-`Decimal` ist .NET-`decimal`, BC speichert
  `decimal(38,20)`. Ein `double` in einer Buchungszeile ist ein Defekt, kein Rundungsproblem — er
  bricht die Summenprobe, an der jede Buchung hängt. `Decimal` ist ein Projekttyp mit eigenem
  Beweis; welche Darstellung ihn trägt, ist gemessen zu entscheiden und nicht zu raten (board:0002)
- **DER GENERIERTE BAUM WIRD NIE VON HAND ANGEFASST.** `src/app/` ist Transpiler-Ausgabe. Ein Fix
  gehört in `src/gen/` oder `src/rt/`. Eine Hand-Änderung dort überlebt den nächsten Lauf nicht und
  kostet die Zeit zweimal
- **DIE RUNTIME KENNT KEIN AL-OBJEKT.** Weder Transpiler noch Runtime nennen je eine konkrete
  Tabelle, Codeunit oder Library-Methode. Jede AL-App muss durch beide laufen. Ein hartcodierter
  AL-Name ist der Fix, der die nächsten zehn Fälle verhindert hat und den elften bricht
- **DETERMINISMUS IST PFLICHT.** Dieselbe Buchung über denselben Datenbestand erzeugt dieselben
  Posten, byteweise, zweimal. Alles, was aus nebenläufiger Arbeit zusammengesetzt wird, wird in
  DEKLARIERTER Reihenfolge kombiniert, nie in Fertigstellungsreihenfolge. Der Beweis ist ein
  Digest über den Postenbestand nach einem Lauf — dieselbe Mechanik, mit der ein Renderer seine
  Bilder prüft

## Wie der Baum geordnet ist

Grundsätze, keine Karte — eine Karte veraltet an dem Tag, an dem ein Verzeichnis umzieht.

- **Ein Verzeichnis IST eine Abhängigkeitsstufe** und trägt eine `reaches`-Datei, die nennt, was es
  sehen darf. Der Include-Pfad wird DARAUS abgeleitet, also scheitert ein Stufenbruch am `#include`
  mit Datei und Zeile statt hinterher im Linker

```
  src/al     ← Lexer, Parser, AST der AL-Sprache            reaches: —
  src/net    ← die .NET-Klassen, nachgebaut                 reaches: —
  src/db     ← PostgreSQL über libpq, Schema, Cursor        reaches: —
  src/gen    ← der Generator: AST → C++                     reaches: al
  src/rt     ← die Runtime: Record, Codeunit, Page, Events  reaches: net db
  src/app    ← GENERIERT. Nie von Hand.                     reaches: rt
  src/cli    ← die eine Tür nach außen                      reaches: rt app
```

- **Ein Header ist ÖFFENTLICH nur, wenn ein Client die Runtime ohne ihn nicht benutzen kann.**
  `include/agiru/` ist die Tür und nichts sonst steht darin
- **ES GIBT EINEN CLIENT.** Ein zweites Programm wäre eine zweite Tür
- **`make` IST DER EINZIGE EINGANG.** Nichts wird gestartet, indem an ihm vorbeigegriffen wird.
  Dass darunter CMake und Ninja arbeiten, ändert das nicht: eine Tür vor einem Generator ist keine
  zweite Mechanik, sondern die Tür

| | |
|---|---|
| `make` | die Bibliothek, den Transpiler und den Client daneben |
| `make db` | `compile_commands.json` für clangd und clang-tidy |
| `make lint` | Format · statische Analyse · die Tür |
| `make test` | das schnelle Gate |
| `make transpile` | die BaseApp durch den Transpiler nach `src/app/` |
| `make provision` | MSSQL-Container, BC-Demo-`.bak` vom CDN, PostgreSQL-Master |
| `make help` | die Liste |

## Was was beweist

**Jede Baseline darf nur SCHRUMPFEN.** Eine strenge Analyse über einen gewachsenen Baum ist an Tag
eins rot und in Woche eins abgeschaltet; ein aufgezeichneter Zähler, den ein Commit senken und nie
heben darf, hält neuen Code auf null und lässt alten reparieren, wenn er angefasst wird.

**Dieser Baum ist neu, also ist jede Baseline heute 0 und bleibt es.** Es gibt keine Altlast, für
die eine Ausnahme zu machen wäre. Was hier je über null steht, ist an dem Tag hineingeschrieben
worden — nicht geerbt.

| Zähler | misst |
|---|---|
| `test/lint-baseline` | clang-tidy-Funde über `src/`, ohne `src/app/` |
| `test/doc-baseline` | undokumentierte öffentliche Namen in `include/` |
| `test/todo-baseline` | `TODO`/`FIXME`/`catch (...) {}` — stille Schlucker |

**Generierter Code wird nicht analysiert, der Generator wird es.** `src/app/` fällt aus `make lint`
heraus, weil ein Befund dort keine Adresse hat. Er fällt NICHT aus dem Compiler heraus:
`-Wall -Werror -Wpedantic` gilt für ihn wie für alles andere, und das ist der Halt.

**Ein Haken ist verdient, wenn sein Beweis steht UND seine Gegenprobe rot wird.** Eine Gegenprobe,
die durchgeht, beweist nichts. Das ist die Falle, die hier am meisten kostet.

## Der Board

`board/` ist ein flaches Verzeichnis von Arbeits-Items als Markdown. Es enthält nur, was OFFEN ist.

**Drei Konventionen stehen hier, weil sie zu brechen still und unumkehrbar ist.** Alles andere über
den Board ist aus dem Board selbst lesbar.

- **Eine Nummer wird EINMAL vergeben und nie wieder**, und die nächste kommt aus der HISTORY, die
  jede je vergebene ID kennt — nicht aus dem Verzeichnis, das nur das Offene kennt. Aus dem
  Verzeichnis genommen wäre sie die Nummer eines Geschlossenen, und zwei Dinge teilten dauerhaft
  eine Identität
- **Ein Item schließen heißt die DATEI LÖSCHEN.** Was es sagte, steht im Commit, und `git log` ist
  das Logbuch. Ein zurückgelassenes `State: closed` nimmt dem Verzeichnis seine Aussage
- **`active` steht im Commit des Items selbst, VOR der Arbeit** — die einzige Besitzmarkierung.
  Mehrere dürfen auf einer Kette stehen, jedes nennt, worauf es wartet

**Jedes Item nennt seine Referenz und seine Wahl** — was die Plattform-Doku sagt, was der
AL-Quelltext tut, was openerp daraus gemacht hat und was es gekostet hat, welcher Weg genommen wird
und warum. Ein Item, das das nicht sagen kann, ist noch nicht verstanden, und diese Zeile zu
schreiben ist der größere Teil des Denkens.

**Titel sagen, was WAHR SEIN WIRD.** Einer im Präsens ist eine Beschwerde, einer im Futur ist ein
Ziel, auf das jemand zielen kann.

**Vor dem Anlegen die History greppen**: eine Entfernung war eine Entscheidung, und dasselbe erneut
anzulegen hebt sie versehentlich auf.

**Ein Defekt, der bei anderer Arbeit gefunden wird, wird in derselben Runde ein Item**, auch wenn er
in derselben Runde schließt: die Alternative ist ein Defekt, den nur eine Person je kannte.

## Wie hier gearbeitet wird

**Reihenfolge: erst das Fundament auf das Ziel bringen, dann darauf bauen, dann die Lücken
schließen.** Ein Umbau auf ein zu kurzes Ziel kommt irgendwo an, das wieder verlassen werden muss.

**Autonom durcharbeiten.** Bei Unklarheit die naheliegendste generische Option wählen, messen, bei
net-negativ zurücknehmen und den Grund in den Board schreiben. Eine widerlegte Hypothese wird
kommentiert, nicht gelöscht — der teuerste Fehler ist, eine bereits widerlegte Ursache erneut zu
verfolgen.

**Fix-Klassifikation vor jedem Fix:**

- **silent-wrong-data** — läuft durch, liefert einen falschen Wert, wirft nicht. Net-positiv und
  regressionsarm
- **activation** — ein bisher toter Pfad läuft jetzt. Oft net-negativ, weil Tests über den No-op
  grün waren. Immer volles A/B. Bei net-negativ nicht verwerfen: die Verlustliste nennt die
  tieferen Roots, diese zuerst

## Was schiefgeht

Gemessene Fehlerarten. Die ersten fünf sind aus openerp geerbt und dort bezahlt worden.

| Falle | wie sie aussieht | die Wache |
|---|---|---|
| **nie geschriebener Ausgabeparameter** | eine Builtin mit `var`-Parameter, die den Wert nur lokal setzt | `var` ist eine Referenz und der Compiler prüft sie — in C++ ist diese Falle geschlossen, sofern der Generator nie kopiert |
| **Wertkontext** | AL entscheidet an Verbrauch-gegen-Verwurf, ob ein Misserfolg wirft oder `false` liefert | die Kontexte sind benannt: Zuweisung, `if`/`while`, `exit`, Argument, `case`-Selektor |
| **Bezeichner-Casing** | AL ist case-insensitiv, divergierendes Casing erzeugt zwei Symbole | Collapse-Match, einmal, im Generator |
| **lokale Option-Enums** | derselbe nackte Feldname in zwei Objekten löst falsche Ordinale auf | synthetische, eindeutige Namen |
| **Plattform-Ereignisse** | feuern unabhängig davon, ob das Objekt den Trigger deklariert | die Runtime feuert, nicht das Objekt |
| **ein blindes Gate** | die Analyse findet nichts und meldet Erfolg, weil sie gar nicht lief | ein Zähler von 0 über N Units ist ein ABBRUCH, kein Bestehen |
| **eine grüne Gegenprobe** | die Gegenprobe besteht, also beweist der Beweis nichts | die Behauptung neu fassen oder löschen; nie einen falschen Beweis behalten |
| **eine Baseline, die aus Versehen fällt** | weniger Units übersetzt, also weniger Funde, also ein falscher Boden | die Baseline trägt die Unit-Zahl neben dem Zähler; schrumpft die, ist es ein Abbruch |

## Die Umgebung

Debian 13 (trixie), x86_64, 2 Kerne, 16 GB. Zwei Kerne sind das knappe Gut: ein Volllauf über 2,56
Mio. Zeilen AL ist hier eine Sache von Minuten bis Stunden, nicht von Sekunden. Deshalb `ccache`,
deshalb `lld`, deshalb ist die Übersetzungszeit eine gemessene Größe mit Board-Item und keine
Nebensache.

- **`libstdc++`-14 hat kein `mdspan` und kein `flat_map`** (gemessen, `__cpp_lib_*` beide
  undefiniert unter g++-14 UND clang++-19). Vorhanden und benutzt: `expected`, `print`, `format`,
  `ranges::to`. Wer eines der beiden fehlenden braucht, schreibt es hin oder begründet libc++
- **PostgreSQL und SQL Server laufen als Podman-Container**, nie als Systemdienst
- **`max_locks_per_transaction = 1024`** auf der PG-Instanz. Das BC-Schema hat rund 1 600 Tabellen;
  eine All-in-one-Transaktion nimmt ein Lock je Objekt und sprengt den Default von 64. Bei
  Container-Neuanlage erneut setzen — `postgresql.auto.conf` überlebt `podman rm` nicht
- **Die Demo-Datenbank MUSS zur BCApps-Version passen.** Ein Schema aus der einen Version und Daten
  aus der anderen ist ein Fehlerbild, das wie ein Runtime-Defekt aussieht und keiner ist. Die
  gepinnte Version steht in `BC_VERSION`, und `make provision` verweigert bei Abweichung
