#!/usr/bin/env bash
set -euo pipefail
profile=${AGIRU_BROWSER_TRUST_PROFILE:?private browser trust profile required}
[[ "$profile" =~ ^/tmp/agiru-browser-https\.[A-Za-z0-9]+/browser-home$ && -d "$profile" ]]
account_home=$(getent passwd "$(id -u)" | cut -d: -f6)
[[ "$account_home" = /home/* ]]
exec bwrap --ro-bind / / --dev-bind /dev /dev --proc /proc --bind /tmp /tmp \
  --bind "$profile" "$account_home" -- /usr/bin/chromium "$@"
