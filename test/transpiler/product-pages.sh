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
sha256sum scope.json Makefile src/gen/{Apps,PageSelection,BodyWriter}.{h,cpp} src/gen/CodeunitWriter.h \
  scripts/{scope_inventory,build_sources}.py src/tc/Main.cpp \
  test/gate/{GenPageSelectionGate,GenScopeGate}.cpp "$fixture"/Runner.cpp \
  "$fixture"/{apps,scope}.json "$fixture"/al/fixture/{app.json,*.al,cloud/*.al,dependency/*.al} \
  test/transpiler/product-pages.sh "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/inputs.sha256"
cp -a "$fixture/al" "$proof/al"
cp "$fixture/apps.json" "$fixture/scope.json" "$proof/"
python3 scripts/scope_inventory.py "$proof/al" --apps "$proof/apps.json" \
  --scope "$proof/scope.json" --output "$proof/inventory.json" > "$proof/inventory.log"
jq -e '.summary | .objects == 7 and .product_excluded_objects == 1 and .selected_objects == 4' \
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
if rg --files --no-ignore "$proof/generated" | rg '/(RemotePanel|UnselectedERP|UnselectedSibling)\.(h|cpp)$'; then
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
for control in part-removal source-domain namespace-policy consumed-value argument-order source-activation source-boundary include-precedence; do
  source=PageSelection
  gate_source=GenPageSelectionGate
  case "$control" in
    consumed-value|argument-order) source=BodyWriter;;
    source-activation|source-boundary|include-precedence) source=Apps; gate_source=GenScopeGate;;
  esac
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
    control == "source-activation" && /return std::ranges::find\(scope.sourceInclude, selected\)/ {
      $0 = "  return false;"; changed++
    }
    control == "source-boundary" && /return std::ranges::find\(scope.sourceInclude, selected\)/ {
      $0 = "  return std::ranges::any_of(scope.sourceInclude, [&](const auto &entry) { return selected.starts_with(entry); });"; changed++
    }
    control == "include-precedence" && /if \(ProductExclusion\(scope, relativeSource, domain\)\)/ {
      $0 = "  static_cast<void>(ProductExclusion(scope, relativeSource, domain));"; changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "src/gen/$source.cpp" > "$proof/$control.cpp"
  "$CXX" "${gate_flags[@]}" "test/gate/$gate_source.cpp" "$proof/$control.cpp" \
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
    source-activation) claim='an exact local dependency is selected despite its namespace omission';;
    source-boundary) claim='source activation never widens to siblings, descendants or case aliases';;
    include-precedence) claim='an explicit product exclusion wins over source activation';;
  esac
  rg -q "$claim" "$proof/$control.log"
done
for control in traversal directory duplicate missing; do
  case "$control" in
    traversal) entries='["fixture/dependency/../LocalDependency.Codeunit.al"]';;
    directory) entries='["fixture/dependency/"]';;
    duplicate) entries='["fixture/dependency/LocalDependency.Codeunit.al","fixture/dependency/LocalDependency.Codeunit.al"]';;
    missing) entries='["fixture/dependency/Missing.Codeunit.al"]';;
  esac
  jq --argjson entries "$entries" '.source_include=$entries' "$fixture/scope.json" > "$proof/scope.json"
  if "$B/agirutc" "$proof/al" "$proof/apps.json" "$proof/refused/$control" > "$proof/$control.generator.log" 2>&1; then
    printf 'product-pages: %s source activation escaped generator validation\n' "$control" >&2; exit 1
  fi
  if python3 scripts/scope_inventory.py "$proof/al" --apps "$proof/apps.json" \
    --scope "$proof/scope.json" --output "$proof/$control.inventory.json" > "$proof/$control.inventory.log" 2>&1; then
    printf 'product-pages: %s source activation escaped inventory validation\n' "$control" >&2; exit 1
  fi
  case "$control" in
    traversal) generator_claim='source rules cannot traverse source roots'; inventory_claim='source includes need bounded relative file paths';;
    directory) generator_claim='source includes require exact AL file identities'; inventory_claim="$generator_claim";;
    duplicate) generator_claim='duplicate source include'; inventory_claim="$generator_claim";;
    missing) generator_claim='source include target is missing'; inventory_claim="$generator_claim";;
  esac
  rg -q "$generator_claim" "$proof/$control.generator.log"
  rg -q "$inventory_claim" "$proof/$control.inventory.log"
done
cp "$fixture/scope.json" "$proof/scope.json"
sha256sum --check --status "$proof/inputs.sha256"
printf 'product-pages: seven raw objects; one explicit exclusion, four selected, two omitted; exact dependency body executes without cloud/sibling activation; eight compiled defects and four invalid source policies reject; %s\n' "$proof"
