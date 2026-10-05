#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-streams.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/streams/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
"$B/gate_StreamGate" > "$proof/current.log" 2>&1
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
runner_source=test/runtime/streams/Runner.cpp
command=("$CXX" "${flags[@]}" -c "$runner_source" -o "$proof/runner.o")
"${command[@]}"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/$runner_source" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "${command[@]}" > "$B/fixture-commands/streams.json"
for control in read-local reset-local; do
  awk -v control="$control" '
    { print }
    control == "read-local" &&
        ($0 == "std::string InStream::ReadBytes(Integer count) {" ||
         $0 == "Integer InStream::ReadTerminated(std::string &into, Integer length) {" ||
         $0 == "Integer InStream::ReadText(::agiru::Text<0> &text, Integer length) {") {
      print "  if (state_ != nullptr) { state_ = std::make_shared<State>(*state_); }"; changed++
    }
    control == "reset-local" && $0 == "Boolean InStream::ResetPosition() {" {
      print "  if (state_ != nullptr) { state_ = std::make_shared<State>(*state_); }"; changed++
    }
    END { if (changed != (control == "read-local" ? 3 : 1)) exit 2 }
  ' src/net/Stream.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "$proof/$control.cpp" "${links[@]}" \
    -o "$proof/$control.so"
  for fixture in "$B/gate_StreamGate" "$proof/runner"; do
    name=$(basename "$fixture")
    if LD_PRELOAD="$proof/$control.so" "$fixture" > "$proof/$control-$name.log" 2>&1; then
      printf 'streams: %s escaped %s\n' "$control" "$name" >&2
      exit 1
    fi
    if [ "$control" = read-local ]; then
      rg -q 'by-value [rR]ead(s| advances)' "$proof/$control-$name.log"
    else
      rg -q 'reset changes every alias|Reset affects every alias' "$proof/$control-$name.log"
    fi
  done
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sha256sum src/net/Stream.cpp include/type/Stream.h test/gate/StreamGate.cpp \
  test/runtime/streams/Fixture.Codeunit.al "$runner_source" "$B/agirutc" \
  "$B/libagiru_net.so" "$B/libagiru_rt.so" "$B/gate_StreamGate" > "$proof/inputs.sha256"
rm -- "$proof/runner.o" "$proof/runner"
printf 'streams: C++ and generated AL share cursor/reset and independent bindings; two compiled controls reject both; %s\n' "$proof"
