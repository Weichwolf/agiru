#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
root=$PWD
name=${AGIRU_DEV_CONTAINER:-agiru-dev}
image=${AGIRU_DEV_IMAGE:-localhost/agiru-dev:latest}
volume=${AGIRU_DEV_VOLUME:-agiru-dev-postgres}
owned() {
  [[ "$(podman inspect --format '{{index .Config.Labels "io.agiru.development"}}' "$name")" = true \
    && "$(podman inspect --format '{{index .Config.Labels "io.agiru.repository"}}' "$name")" = "$root" ]] || {
    printf 'Refusing a container without matching agiru development ownership\n' >&2; exit 1;
  }
}
case "${1:-}" in
  image)
    exec podman build --format docker --layers --build-arg "DEV_UID=$(id -u)" --build-arg "DEV_GID=$(id -g)" \
      --tag "$image" deploy/dev
    ;;
  start)
    shift
    if podman container exists "$name"; then
      owned
      [[ "$#" = 0 ]] || { printf 'Existing container command cannot be replaced by start\n' >&2; exit 1; }
      exec podman start "$name"
    fi
    tls_flags=()
    if [[ -n "${AGIRU_DEV_HTTPS_PORT:-}" ]]; then
      tls_flags+=(--publish "127.0.0.1:${AGIRU_DEV_HTTPS_PORT}:8443")
    fi
    exec podman run --detach --name "$name" \
      --label io.agiru.development=true --label "io.agiru.repository=$root" \
      --userns="keep-id:uid=$(id -u),gid=$(id -g)" --user 0 \
      --publish "127.0.0.1:${AGIRU_DEV_HTTP_PORT:-8080}:8080" \
      "${tls_flags[@]}" --env "AGIRU_HTTP_ADDRESS=${AGIRU_DEV_SITE:-:8080}" \
      --volume "$root:/workspace" --volume "$volume:/var/lib/agiru" "$image" "$@"
    ;;
  stop) owned; exec podman stop --time 30 "$name" ;;
  configure)
    owned
    exec podman exec --user "$(id -u):$(id -g)" "$name" cmake -S /workspace \
      -B /workspace/build/podman -G Ninja -DCMAKE_CXX_COMPILER=clang++-19 \
      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
      -DAGIRU_TEST_DSN=postgresql://agiru:agiru@127.0.0.1:5432/agiru_gate \
      -DAGIRU_MASTER_DSN=postgresql://agiru:agiru@127.0.0.1:5432/agiru_master
    ;;
  exec)
    owned
    shift
    [[ "$#" -gt 0 ]] || { printf 'dev-exec requires a command\n' >&2; exit 2; }
    flags=()
    case "${AGIRU_DEV_INTERACTIVE:-0}" in
      0) ;;
      1) flags+=(--interactive) ;;
      *) printf 'AGIRU_DEV_INTERACTIVE must be 0 or 1\n' >&2; exit 2 ;;
    esac
    exec podman exec "${flags[@]}" --user "$(id -u):$(id -g)" "$name" "$@"
    ;;
  *) printf 'Usage: %s image|start|stop|configure|exec COMMAND...\n' "$0" >&2; exit 2 ;;
esac
