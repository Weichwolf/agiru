#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
root=$PWD
src/client/node_modules/.bin/tsc --project src/client/tsconfig.web.json
mkdir -p build/web
src/client/node_modules/.bin/esbuild src/client/web.mts --bundle --format=esm \
  --platform=browser --target=es2022 --legal-comments=external --outfile=build/web/web.js
cp src/client/web/index.html src/client/web/web.css build/web/
cp src/client/node_modules/htmx.org/LICENSE build/web/htmx-LICENSE.txt
cp src/client/node_modules/parse5/LICENSE build/web/parse5-LICENSE.txt
cp src/client/node_modules/entities/LICENSE build/web/entities-LICENSE.txt
printf 'web: browser assets in %s/build/web; no Node.js server dependency\n' "$root"
