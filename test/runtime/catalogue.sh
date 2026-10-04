#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-catalogue.XXXXXX)
gate="$B/gate_CatalogueGate"
"$gate" > "$proof/concurrent.log" 2>&1
for reader in table name page codeunit profile tables pages codeunits lookup; do
  "$gate" "$reader" > "$proof/$reader.log" 2>&1
done
for kind in table page codeunit; do
  "$gate" duplicate "$kind" > "$proof/duplicate-$kind.log" 2>&1
done
for composition in source base unrelated conflict; do
  "$gate" native-duplicate "$composition" > "$proof/native-duplicate-$composition.log" 2>&1
done

flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in freeze duplicate order source-binding native-duplicate; do
  awk -v control="$control" '
    control == "freeze" && /if \(frozen_\)/ {
      sub(/if \(frozen_\)/, "if (false)"); changed++
    }
    control == "duplicate" && /if \(Number\(entries\[i - 1\]\) == Number\(entries\[i\]\)\)/ {
      sub(/if \(Number\(entries\[i - 1\]\) == Number\(entries\[i\]\)\)/, "if (false)"); changed++
    }
    control == "order" && /return Number\(a\) < Number\(b\);/ {
      sub(/return Number\(a\) < Number\(b\);/, "return Number(a) > Number(b);"); changed++
    }
    control == "source-binding" && /return \*entry->table;/ {
      sub(/return \*entry->table;/, "return *binding;"); changed++
    }
    control == "native-duplicate" && /const bool third =/ {
      $0 = "        const bool third = false;"; changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/Catalogue.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -o "$proof/$control.so"
  if [ "$control" = duplicate ]; then
    for kind in table page codeunit; do
      if LD_PRELOAD="$proof/$control.so" "$gate" duplicate "$kind" \
          > "$proof/control-$control-$kind.log" 2>&1; then
        printf 'catalogue: duplicate %s escaped its composition gate\n' "$kind" >&2
        exit 1
      fi
      rg -q 'FAIL ' "$proof/control-$control-$kind.log"
    done
  elif [ "$control" = native-duplicate ]; then
    if LD_PRELOAD="$proof/$control.so" "$gate" native-duplicate base \
        > "$proof/control-$control.log" 2>&1; then
      printf 'catalogue: duplicate native ABI escaped its composition gate\n' >&2
      exit 1
    fi
    rg -q 'FAIL ' "$proof/control-$control.log"
  elif LD_PRELOAD="$proof/$control.so" "$gate" \
      > "$proof/control-$control.log" 2>&1; then
    printf 'catalogue: %s escaped the immutable lookup gate\n' "$control" >&2
    exit 1
  else
    rg -q 'FAIL ' "$proof/control-$control.log"
  fi
  rm -- "$proof/$control.so" "$proof/$control.cpp"
done
printf 'catalogue: shared sorted lookup, concurrent reads, nine freeze paths and native source composition pass; five controls reject; %s\n' "$proof"
