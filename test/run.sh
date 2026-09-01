#!/bin/sh
# `make test` -- das schnelle Gate.
#
# Es gibt heute keinen Fall zu fahren. Diese Datei steht trotzdem, weil ein Gate, das erst
# entsteht, wenn der erste Test da ist, in genau dem Moment geschrieben wird, in dem niemand Zeit
# dafuer hat. Sie geht ROT, solange sie leer ist -- ein leeres Gate, das gruen meldet, ist die
# erste Falle auf CLAUDE.mds Liste.
set -eu
cd "$(dirname "$0")/.."
cases=$(find test/gate -name '*.cpp' 2>/dev/null | wc -l | tr -d ' ')
if [ "$cases" -eq 0 ]; then
  printf 'test: das Gate ist LEER (0 Faelle unter test/gate/). Es meldet deshalb rot und nicht gruen.\n' >&2
  printf 'test: board:0003 ist der Fall, der das aufhebt.\n' >&2
  exit 1
fi
printf 'test: %s Fall/Faelle\n' "$cases"
