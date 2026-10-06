#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-streams.XXXXXX)
mkdir -p "$proof/source" "$proof/files"
export TMPDIR="$proof/files"
cp test/runtime/streams/*.al "$proof/source/"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
"$B/gate_StreamGate" > "$proof/current.log" 2>&1
"$B/gate_FileGate" > "$proof/file-current.log" 2>&1
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
sanitizers=(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
sanitized_environment=(ASAN_OPTIONS=detect_stack_use_after_return=1:symbolize=0
  UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=0)
"$CXX" "${flags[@]}" "${sanitizers[@]}" test/gate/StreamGate.cpp "${links[@]}" \
  -o "$proof/sanitized-gate"
"$CXX" "${flags[@]}" "${sanitizers[@]}" "$runner_source" "${sources[@]}" "${links[@]}" \
  -o "$proof/sanitized-runner"
"$CXX" "${flags[@]}" "${sanitizers[@]}" -fPIC -shared src/net/Stream.cpp src/net/Blob.cpp \
  "${links[@]}" -o "$proof/sanitized.so"
for fixture in sanitized-gate sanitized-runner; do
  env "${sanitized_environment[@]}" LD_PRELOAD="$proof/sanitized.so" \
    "$proof/$fixture" > "$proof/$fixture.log" 2>&1
done
awk '
  /^  return bytes_;$/ {
    print "  return std::shared_ptr<Storage>(bytes_.get(), [](Storage *pointer) { static_cast<void>(pointer); });";
    changed++; next
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/net/Blob.cpp > "$proof/unowned-provider.cpp"
"$CXX" "${flags[@]}" "${sanitizers[@]}" -fPIC -shared src/net/Stream.cpp \
  "$proof/unowned-provider.cpp" "${links[@]}" -o "$proof/unowned-provider.so"
for fixture in sanitized-gate sanitized-runner; do
  if env "${sanitized_environment[@]}" LD_PRELOAD="$proof/unowned-provider.so" \
      "$proof/$fixture" > "$proof/unowned-$fixture.log" 2>&1; then
    printf 'streams: unowned provider escaped %s\n' "$fixture" >&2
    exit 1
  fi
  rg -q 'ERROR: AddressSanitizer: heap-use-after-free' "$proof/unowned-$fixture.log"
done
awk '
  /std::make_shared<Storage>\(\*other.bytes_\)/ {
    sub(/std::make_shared<Storage>\(\*other.bytes_\)/, "other.bytes_"); changed++
  }
  { print }
  END { if (changed != 2) exit 2 }
' src/net/Blob.cpp > "$proof/shared-value.cpp"
"$CXX" "${flags[@]}" -fPIC -shared "$proof/shared-value.cpp" "${links[@]}" \
  -o "$proof/shared-value.so"
for fixture in "$B/gate_StreamGate" "$proof/runner"; do
  name=$(basename "$fixture")
  if LD_PRELOAD="$proof/shared-value.so" "$fixture" > "$proof/shared-value-$name.log" 2>&1; then
    printf 'streams: shared BLOB value escaped %s\n' "$name" >&2
    exit 1
  fi
  rg -q 'BLOB value copies do not share mutations|BLOB by-value writes preserve the caller' \
    "$proof/shared-value-$name.log"
done
for control in file-lines-only file-count-endings file-one-based; do
  awk -v control="$control" '
    control == "file-one-based" && /^  return static_cast<Integer>\(position_\);$/ {
      print "  return static_cast<Integer>(position_) + 1;"; changed++; next
    }
    /^Integer File::ReadText\(/ {
      reading = 1; print
      if (control == "file-count-endings") { print "  const std::size_t initial = position_;"; changed++ }
      next
    }
    reading && control == "file-lines-only" && /^  if \(textMode_\) \{$/ {
      print "  if (true) {"; changed++; next
    }
    reading && control == "file-count-endings" && /return static_cast<Integer>\(text.size\(\)\);/ {
      print "    return static_cast<Integer>(position_ - initial);"; changed++; next
    }
    { print }
    END { if (changed != (control == "file-count-endings" ? 2 : 1)) exit 2 }
  ' src/net/AlFile.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "$proof/$control.cpp" "${links[@]}" -o "$proof/$control.so"
  for fixture in "$B/gate_FileGate" "$proof/runner"; do
    name=$(basename "$fixture")
    if LD_PRELOAD="$proof/$control.so" "$fixture" > "$proof/$control-$name.log" 2>&1; then
      printf 'streams: %s escaped %s\n' "$control" "$name" >&2
      exit 1
    fi
    rg -q 'binary Read retains XML|default File.Read does not split|text mode excludes|text-mode File.Read excludes|file pointer starts at zero|File.Pos begins at zero' \
      "$proof/$control-$name.log"
  done
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
for control in file-default-text file-capacity-erased; do
  mkdir -p "$proof/$control/type"
  awk -v control="$control" '
    control == "file-default-text" && /^  bool textMode_ = false;$/ {
      print "  bool textMode_ = true;"; changed++; next
    }
    control == "file-capacity-erased" && /maximum = Read.Max\(\);/ {
      sub(/maximum = Read.Max\(\);/, "maximum = 0;"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/type/File.h > "$proof/$control/type/File.h"
  "$CXX" "-I$proof/$control" "${flags[@]}" test/gate/FileGate.cpp "${links[@]}" \
    -o "$proof/$control-gate"
  "$CXX" "-I$proof/$control" "${flags[@]}" "$runner_source" "${sources[@]}" "${links[@]}" \
    -o "$proof/$control-runner"
  for fixture in gate runner; do
    if "$proof/$control-$fixture" > "$proof/$control-$fixture.log" 2>&1; then
      printf 'streams: %s escaped %s\n' "$control" "$fixture" >&2
      exit 1
    fi
    rg -q 'defaults to binary|default File.Read does not split|binary capacity allows|declared capacity' \
      "$proof/$control-$fixture.log"
  done
  sha256sum "$proof/$control/type/File.h" "$proof/$control-gate" "$proof/$control-runner" \
    >> "$proof/controls.sha256"
  rm -- "$proof/$control/type/File.h" "$proof/$control-gate" "$proof/$control-runner"
  rmdir -- "$proof/$control/type" "$proof/$control"
done
sha256sum Makefile test/runtime/streams.sh src/net/Stream.cpp src/net/Blob.cpp include/type/Stream.h include/type/Blob.h test/gate/StreamGate.cpp \
  src/net/AlFile.cpp include/type/File.h test/gate/FileGate.cpp "$B/gate_FileGate" \
  test/runtime/streams/Fixture.Codeunit.al "$runner_source" "$B/agirutc" \
  "$B/libagiru_net.so" "$B/libagiru_rt.so" "$B/gate_StreamGate" > "$proof/inputs.sha256"
sha256sum "$proof/unowned-provider.cpp" "$proof/unowned-provider.so" \
  "$proof/shared-value.cpp" "$proof/shared-value.so" >> "$proof/controls.sha256"
rm -- "$proof/runner.o" "$proof/runner" "$proof/sanitized-gate" "$proof/sanitized-runner" \
  "$proof/sanitized.so" "$proof/unowned-provider.cpp" "$proof/unowned-provider.so" \
  "$proof/shared-value.cpp" "$proof/shared-value.so"
mapfile -t temporary_files < <(rg --files --hidden --no-ignore "$proof/files")
if [ "${#temporary_files[@]}" -gt 0 ]; then rm -- "${temporary_files[@]}"; fi
rmdir -- "$proof/files"
printf 'streams: cursor/provider/value contracts preserve ASan/UBSan; file modes/capacities/positions preserve generated AL; nine compiled controls reject both; %s\n' "$proof"
