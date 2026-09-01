#!/bin/sh
# Was auf dieser Debian-13-Box fehlt (gemessen 2026-09-01). Vorhanden waren bereits: clang-19,
# clang-format-19, lld-19, gcc-14, cmake 3.31, ninja, ccache, podman, git, curl, unzip, jq, python3.
set -eu
sudo apt-get update
sudo apt-get install -y \
  clang-tidy-19 \
  libpq-dev postgresql-client-17 \
  doxygen graphviz
# clang-tidy-19 landet als /usr/bin/clang-tidy-19; test/lint.sh sucht beide Namen.
