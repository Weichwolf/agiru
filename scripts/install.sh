#!/bin/sh
# Development dependencies for Debian 13. External AL sources and database provisioning are separate.
set -eu
sudo apt-get update
sudo apt-get install -y \
  clang-19 clang-format-19 clang-tidy-19 lld-19 \
  libc++-19-dev libc++abi-19-dev libunwind-19-dev libclang-rt-19-dev \
  cmake ninja-build ccache python3 git curl unzip jq podman \
  libpq-dev postgresql-client-17 libxml2-dev \
  doxygen graphviz
