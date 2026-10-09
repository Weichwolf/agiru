#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-product-pages.XXXXXX)
fixture=test/transpiler/product-pages
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.bin" -o -name "*.o" -o -name "*.cpp" \) -delete' EXIT
printf 'product-pages: receipts %s\n' "$proof"
git rev-parse HEAD > "$proof/head"
sha256sum scope.json Makefile src/gen/{PageSelection,BodyWriter}.{h,cpp} src/gen/CodeunitWriter.h \
  src/tc/Main.cpp test/gate/GenPageSelectionGate.cpp "$fixture"/Runner.cpp \
  "$fixture"/{apps,scope}.json "$fixture"/al/fixture/{app.json,*.al,cloud/*.al} \
  test/transpiler/product-pages.sh "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
cp -a "$fixture/al" "$proof/al"
cp "$fixture/apps.json" "$fixture/scope.json" "$proof/"
python3 scripts/scope_inventory.py "$proof/al" --apps "$proof/apps.json" \
  --scope "$proof/scope.json" --output "$proof/inventory.json" > "$proof/inventory.log"
jq -e '.summary | .objects == 5 and .product_excluded_objects == 1 and .selected_objects == 3' \
  "$proof/inventory.json" > /dev/null
"$B/agirutc" "$proof/al" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
awk -F '\t' '
  NR == 1 { if (NF != 10) exit 2; next }
  $1 == "part" { parts++ }
  $1 == "call" { calls++ }
  { if ($3 != 50300 || $4 != "Feature Host" || $6 != "Page 50301 Remote Panel" ||
        $7 != "fixture/cloud/Remote.Page.al" || $8 != "microsoft-cloud") exit 2 }
  END { if (parts != 3 || calls != 3) exit 2 }
' "$proof/generated/generation-product-parts.tsv"
if rg --files --no-ignore "$proof/generated" | rg '/(RemotePanel|UnselectedERP)\.(h|cpp)$'; then
  printf 'product-pages: excluded/omitted pages were emitted\n' >&2; exit 1
fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate \
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[[ ${#sources[@]} -gt 0 ]]
"$CXX" "${flags[@]}" -c "$fixture/Runner.cpp" -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/current.bin"
"$proof/current.bin" | tee "$proof/current.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/$fixture/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c "$fixture/Runner.cpp" -o "$proof/runner.o" \
  > "$B/fixture-commands/product-pages.json"
gate_flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
  -Iinclude -Itest/gate -Isrc/gen -Isrc/al "-DAGIRU_SOURCE_DIR=\"$PWD\"" \
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 "-L$B" "-Wl,-rpath,$B")
for control in part-removal source-domain namespace-policy consumed-value argument-order; do
  source=PageSelection
  case "$control" in consumed-value|argument-order) source=BodyWriter;; esac
  awk -v control="$control" '
    BEGIN { if (control == "argument-order") print "#include <ranges>" }
    control == "namespace-policy" && /const auto reason = ProductExclusion\(scope, source, domain\);/ {
      sub(/ProductExclusion\(scope, source, domain\)/,
          "ProductExclusion(scope, source, domain).or_else([] { return std::optional<std::string_view>{\"microsoft-cloud\"}; })"); changed++
    }
    control == "part-removal" && /rows.push_back\(std::move\(row\)\);/ { removal=1 }
    control == "part-removal" && removal && /return true;/ {
      sub(/return true;/, "return false;"); changed++; removal=0
    }
    control == "source-domain" && /const auto reason = ProductExclusion\(scope, source, domain\);/ {
      sub(/scope, source, domain/, "scope, source"); changed++
    }
    control == "consumed-value" && /use == ValueUse::Discarded && refused->productExcluded/ {
      sub(/use == ValueUse::Discarded/, "true"); changed++
    }
    control == "argument-order" && /for \(const al::Expr \*argument : refused->arguments\)/ {
      sub(/refused->arguments/, "std::views::reverse(refused->arguments)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "src/gen/$source.cpp" > "$proof/$control.cpp"
  "$CXX" "${gate_flags[@]}" test/gate/GenPageSelectionGate.cpp "$proof/$control.cpp" \
    -lagiru_gen -lagiru_al -o "$proof/$control.bin" > "$proof/$control.compile.log" 2>&1
  status=0
  "$proof/$control.bin" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]] || { printf 'product-pages: %s expected gate exit 1, got %s\n' "$control" "$status" >&2; exit 1; }
  case "$control" in
    part-removal) claim='the excluded part is removed but charts and unavailable ERP remain';;
    source-domain) claim='a BCApps product rule does not authorize a System-domain exclusion';;
    namespace-policy) claim='an omitted cloud namespace alone cannot authorize product-part removal';;
    consumed-value) claim='missing ERP and consumed cloud calls still refuse';;
    argument-order) claim='excluded direct calls preserve argument evaluation in source order';;
  esac
  rg -q "$claim" "$proof/$control.log"
done
sha256sum --check --status "$proof/inputs.sha256"
printf 'product-pages: five raw objects; one explicit exclusion, three selected, one omitted; native effects/refusals execute; five compiled defects reject; %s\n' "$proof"
