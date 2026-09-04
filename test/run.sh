#!/bin/sh
# `make test` -- the fast gate. Every case is its own program; a crash takes only its own case down,
# not the run.
#
# AN EMPTY GATE REPORTS RED. A runner that says green at zero cases is the first trap on CLAUDE.md's
# list -- a gate blind to its own path.
set -eu
cd "$(dirname "$0")/.."
B=build

cmake --build "$B" -j "$(nproc)" >/dev/null

cases=$(find "$B" -maxdepth 1 -name 'gate_*' -type f -perm -u+x | sort)
if [ -z "$cases" ]; then
  printf 'test: the gate is EMPTY (0 cases under test/gate/). It therefore reports red.\n' >&2
  exit 1
fi

red=0
n=0
for c in $cases; do
  n=$((n + 1))
  if ! "$c"; then red=$((red + 1)); fi
done

# THE DOOR'S GENERATOR IS A CASE TOO, and it is a script rather than a binary because what it
# asserts is about FILES: that running the generator leaves them as they are.
n=$((n + 1))
if ! sh "$(dirname "$0")/door-reproduces.sh"; then red=$((red + 1)); fi
printf '\ntest: %s case(s), %s red\n' "$n" "$red"
[ "$red" -eq 0 ]
