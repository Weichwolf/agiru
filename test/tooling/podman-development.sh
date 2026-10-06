#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
image=${AGIRU_DEV_IMAGE:-localhost/agiru-dev:latest}
prefix=agiru-dev-check-$$
volume=$prefix-data
bad_volume=$prefix-bad-data
proof=$(mktemp -d /tmp/agiru-dev-check.XXXXXX)
sha256sum deploy/dev/Containerfile deploy/dev/entrypoint.sh scripts/dev_container.sh \
  test/tooling/podman-development.sh > "$proof/inputs.sha256"
podman image inspect "$image" --format '{{.Id}}' > "$proof/image.txt"
cleanup() {
  for suffix in main failure bad unowned; do
    target=$prefix-$suffix
    if podman container exists "$target"; then
      [[ "$(podman inspect --format '{{index .Config.Labels "io.agiru.development.test"}}' "$target")" = "$prefix" ]] || continue
      podman rm --force "$target" >/dev/null
    fi
  done
  for target in "$volume" "$bad_volume"; do
    if podman volume exists "$target"; then
      [[ "$(podman volume inspect --format '{{index .Labels "io.agiru.development.test"}}' "$target")" = "$prefix" ]] || continue
      podman volume rm "$target" >/dev/null
    fi
  done
}
trap cleanup EXIT
ready() {
  for ((attempt=0; attempt<60; attempt++)); do
    if podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 \
      -d agiru_gate -c 'SELECT 1' > "$proof/ready.log" 2>&1; then return; fi
    [[ "$(podman inspect --format '{{.State.Running}}' "$prefix-main")" = true ]] || {
      podman logs "$prefix-main" >&2; return 1;
    }
    sleep 0.5
  done
  printf 'Development SQL readiness timed out\n' >&2
  return 1
}
podman volume create --label "io.agiru.development.test=$prefix" "$volume" >/dev/null
podman volume create --label "io.agiru.development.test=$prefix" "$bad_volume" >/dev/null
podman run --detach --name "$prefix-main" --label io.agiru.development=true \
  --label "io.agiru.repository=$PWD" --label "io.agiru.development.test=$prefix" --volume "$volume:/var/lib/agiru" \
  "$image" /bin/sleep 300 > "$proof/main-id.txt"
ready
podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 -d agiru_gate \
  -c "CREATE TABLE development_receipt (amount numeric); INSERT INTO development_receipt VALUES (1234.56)" \
  > "$proof/sql-write.log"
AGIRU_DEV_CONTAINER=$prefix-main bash scripts/dev_container.sh stop > "$proof/stop.log"
AGIRU_DEV_CONTAINER=$prefix-main bash scripts/dev_container.sh start > "$proof/restart.log"
ready
[[ "$(podman exec --user postgres "$prefix-main" psql -XAt -v ON_ERROR_STOP=1 \
  -d agiru_gate -c 'SELECT amount::text FROM development_receipt')" = 1234.56 ]]
podman exec "$prefix-main" bash -c 'kill -TERM "$(head -n 1 "$PGDATA/postmaster.pid")"'
[[ "$(timeout 30 podman wait "$prefix-main")" = 1 ]]
podman logs "$prefix-main" > "$proof/database-exit.log" 2>&1
podman run --detach --name "$prefix-failure" --label "io.agiru.development.test=$prefix" \
  --volume "$volume:/var/lib/agiru" "$image" /bin/sh -c 'exit 42' > "$proof/failure-id.txt"
[[ "$(timeout 45 podman wait "$prefix-failure")" = 42 ]]
podman logs "$prefix-failure" > "$proof/application-exit.log" 2>&1
podman create --name "$prefix-unowned" --label "io.agiru.development.test=$prefix" \
  --entrypoint /bin/true "$image" > "$proof/unowned-id.txt"
status=0
AGIRU_DEV_CONTAINER=$prefix-unowned bash scripts/dev_container.sh start \
  > "$proof/unowned.log" 2>&1 || status=$?
[[ "$status" = 1 ]]
[[ "$(podman inspect --format '{{.State.Status}}' "$prefix-unowned")" = created ]]
status=0
AGIRU_DEV_CONTAINER=$prefix-unowned bash scripts/dev_container.sh stop \
  > "$proof/unowned-stop.log" 2>&1 || status=$?
[[ "$status" = 1 ]]
[[ "$(podman inspect --format '{{.State.Status}}' "$prefix-unowned")" = created ]]
podman run --rm --entrypoint /bin/bash --volume "$bad_volume:/var/lib/agiru" "$image" \
  -c 'mkdir -p /var/lib/agiru/postgres; printf "18\n" > /var/lib/agiru/postgres/PG_VERSION; printf "preserve\n" > /var/lib/agiru/postgres/sentinel'
podman run --detach --name "$prefix-bad" --label "io.agiru.development.test=$prefix" \
  --volume "$bad_volume:/var/lib/agiru" "$image" > "$proof/bad-id.txt"
[[ "$(timeout 30 podman wait "$prefix-bad")" = 1 ]]
podman logs "$prefix-bad" > "$proof/version-refusal.log" 2>&1
rg -q 'PostgreSQL storage version mismatch' "$proof/version-refusal.log"
[[ "$(podman run --rm --entrypoint /bin/bash --volume "$bad_volume:/var/lib/agiru" "$image" \
  -c 'read -r marker < /var/lib/agiru/postgres/sentinel; printf "%s" "$marker"')" = preserve ]]
sha256sum --check "$proof/inputs.sha256" > "$proof/input-integrity.log"
printf 'podman-development: SQL persistence, supervisor exits, ownership and incompatible storage verified; %s\n' "$proof"
