#!/bin/sh
# Development dependencies for Debian 13. External AL sources and database provisioning are separate.
set -eu
sudo apt-get update
sudo apt-get install -y \
  clang-19 clang-format-19 clang-tidy-19 lld-19 g++-14 \
  cmake ninja-build ccache python3 git curl unzip jq podman \
  libpq-dev postgresql-client-17 libxml2-dev \
  doxygen graphviz
