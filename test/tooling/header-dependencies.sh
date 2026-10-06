#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=${B:-build}
mkdir -p "$B"
proof=$(mktemp -d /tmp/agiru-header-dependencies.XXXXXX)
CXX=${CXX:-clang++-19}
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/gen)

compile_header() {
  local header=$1
  local dependencies=$2
  shift 2
  printf '#include "%s"\n' "$header" | "$CXX" "${flags[@]}" "$@" \
    -x c++ - -fsyntax-only -MD -MF "$dependencies" -MT header-probe
}

reject_dependency() {
  local dependencies=$1
  local forbidden=$2
  if rg -q "/$forbidden([[:space:]\\\\]|$)" "$dependencies"; then
    printf 'header-dependencies: %s contains <%s>\n' "$dependencies" "$forbidden" >&2
    return 1
  fi
}

for header in ObjectKind.h RuntimeSurface.h Names.h; do
  compile_header "$header" "$proof/$header.d"
  reject_dependency "$proof/$header.d" filesystem
done
compile_header dotnet/Regex.h "$proof/Regex.h.d"
reject_dependency "$proof/Regex.h.d" regex
compile_header dotnet/HashAlgorithm.h "$proof/HashAlgorithm.h.d"
for forbidden in evp.h Regex.h Variant.h vector; do
  reject_dependency "$proof/HashAlgorithm.h.d" "$forbidden"
done
compile_header dotnet/Convert.h "$proof/Convert.h.d"
for forbidden in Regex.h Variant.h vector evp.h; do
  reject_dependency "$proof/Convert.h.d" "$forbidden"
done
compile_header dotnet/Convert.h "$proof/forced-Array.h.d" -include dotnet/Regex.h
if reject_dependency "$proof/forced-Array.h.d" Regex.h \
  > "$proof/forced-Array.h.log" 2>&1; then
  printf 'header-dependencies: complete Array implementation escaped the control\n' >&2
  exit 1
fi
compile_header dotnet/HashAlgorithm.h "$proof/forced-crypto.h.d" -include openssl/evp.h
if reject_dependency "$proof/forced-crypto.h.d" evp.h \
  > "$proof/forced-crypto.h.log" 2>&1; then
  printf 'header-dependencies: private crypto backend escaped the control\n' >&2
  exit 1
fi
compile_header type/JsonHandle.h "$proof/JsonHandle.h.d"
for forbidden in memory yyjson.h json.hpp; do
  reject_dependency "$proof/JsonHandle.h.d" "$forbidden"
done
compile_header type/JsonHandle.h "$proof/forced-yyjson.h.d" -include yyjson.h
if reject_dependency "$proof/forced-yyjson.h.d" yyjson.h \
  > "$proof/forced-yyjson.h.log" 2>&1; then
  printf 'header-dependencies: JSON backend escaped the control\n' >&2
  exit 1
fi
compile_header meta/ModuleDef.h "$proof/ModuleDef.h.d"
for forbidden in ModuleInfo.h Guid.h List.h Text.h Version.h vector; do
  reject_dependency "$proof/ModuleDef.h.d" "$forbidden"
done
compile_header meta/ModuleDef.h "$proof/forced-ModuleInfo.h.d" -include type/ModuleInfo.h
if reject_dependency "$proof/forced-ModuleInfo.h.d" ModuleInfo.h \
  > "$proof/forced-ModuleInfo.h.log" 2>&1; then
  printf 'header-dependencies: AL module state escaped the declaration control\n' >&2
  exit 1
fi
compile_header runtime/ReportRegistry.h "$proof/ReportRegistry.h.d"
for forbidden in Report.h Page.h RecordRef.h Variant.h vector; do
  reject_dependency "$proof/ReportRegistry.h.d" "$forbidden"
done
compile_header runtime/ReportRegistry.h "$proof/forced-Report.h.d" -include runtime/Report.h
if reject_dependency "$proof/forced-Report.h.d" Report.h \
  > "$proof/forced-Report.h.log" 2>&1; then
  printf 'header-dependencies: full report header escaped the registry control\n' >&2
  exit 1
fi
compile_header runtime/TableDefinition.h "$proof/TableDefinition.h.d"
for forbidden in Record.h RecordRef.h Variant.h PageDef.h vector mutex filesystem regex; do
  reject_dependency "$proof/TableDefinition.h.d" "$forbidden"
done
compile_header runtime/TableDefinition.h "$proof/forced-Record.h.d" -include runtime/Record.h
if reject_dependency "$proof/forced-Record.h.d" Record.h \
  > "$proof/forced-Record.h.log" 2>&1; then
  printf 'header-dependencies: record state escaped the table declaration control\n' >&2
  exit 1
fi

compile_header runtime/PageCore.h "$proof/PageCore.h.d"
for forbidden in PageTraps.h TestPage.h TableDef.h Record.h vector mutex; do
  reject_dependency "$proof/PageCore.h.d" "$forbidden"
done
compile_header runtime/PageCore.h "$proof/forced-PageTraps.h.d" -include runtime/test/PageTraps.h
if reject_dependency "$proof/forced-PageTraps.h.d" PageTraps.h \
  > "$proof/forced-PageTraps.h.log" 2>&1; then
  printf 'header-dependencies: test trapping escaped the production control boundary\n' >&2
  exit 1
fi
compile_header runtime/PageDispatcher.h "$proof/PageDispatcher.h.d"
for forbidden in Page.h PageDef.h TestPage.h PageTraps.h Record.h vector mutex; do
  reject_dependency "$proof/PageDispatcher.h.d" "$forbidden"
done
compile_header runtime/PageDispatcher.h "$proof/forced-Page.h.d" -include runtime/Page.h
if reject_dependency "$proof/forced-Page.h.d" Page.h \
  > "$proof/forced-Page.h.log" 2>&1; then
  printf 'header-dependencies: typed page machinery escaped the dispatcher boundary\n' >&2
  exit 1
fi
compile_header runtime/PageInstance.h "$proof/PageInstance.h.d"
for forbidden in PageCore.h Page.h PageDef.h PageSession.h TestPage.h Record.h RecordId.h vector mutex; do
  reject_dependency "$proof/PageInstance.h.d" "$forbidden"
done
compile_header runtime/PageInstance.h "$proof/forced-PageSession.h.d" -include runtime/PageSession.h
if reject_dependency "$proof/forced-PageSession.h.d" PageSession.h \
  > "$proof/forced-PageSession.h.log" 2>&1; then
  printf 'header-dependencies: typed execution escaped the instance interface boundary\n' >&2
  exit 1
fi

compile_header type/Utf8.h "$proof/Utf8.h.d"
for forbidden in Encoding.h Regex.h Record.h vector memory; do
  reject_dependency "$proof/Utf8.h.d" "$forbidden"
done
compile_header type/Utf8.h "$proof/forced-Encoding.h.d" -include dotnet/Encoding.h
if reject_dependency "$proof/forced-Encoding.h.d" Encoding.h \
  > "$proof/forced-Encoding.h.log" 2>&1; then
  printf 'header-dependencies: full codec escaped the UTF-8 validator boundary\n' >&2
  exit 1
fi
for header in PageValue.h PageHtml.h; do
  compile_header "runtime/$header" "$proof/$header.d"
  for forbidden in Page.h PageSession.h PageCore.h PageDef.h TableDef.h Record.h Variant.h vector mutex; do
    reject_dependency "$proof/$header.d" "$forbidden"
  done
done
compile_header runtime/PageHtml.h "$proof/forced-PageCore.h.d" -include runtime/PageCore.h
if reject_dependency "$proof/forced-PageCore.h.d" PageCore.h \
  > "$proof/forced-PageCore.h.log" 2>&1; then
  printf 'header-dependencies: page execution escaped the HTML transport boundary\n' >&2
  exit 1
fi

compile_header runtime/SessionCommand.h "$proof/SessionCommand.h.d"
for forbidden in Session.h Database.h Transaction.h vector memory mutex thread; do
  reject_dependency "$proof/SessionCommand.h.d" "$forbidden"
done
compile_header runtime/SessionCommand.h "$proof/forced-Session.h.d" -include runtime/Session.h
if reject_dependency "$proof/forced-Session.h.d" Session.h \
  > "$proof/forced-Session.h.log" 2>&1; then
  printf 'header-dependencies: ambient session state escaped the command boundary\n' >&2
  exit 1
fi
compile_header runtime/SingleInstance.h "$proof/SingleInstance.h.d"
for forbidden in Codeunit.h Table.h Record.h vector memory mutex; do
  reject_dependency "$proof/SingleInstance.h.d" "$forbidden"
done
compile_header runtime/SingleInstance.h "$proof/forced-Codeunit.h.d" -include runtime/Codeunit.h
if reject_dependency "$proof/forced-Codeunit.h.d" Codeunit.h \
  > "$proof/forced-Codeunit.h.log" 2>&1; then
  printf 'header-dependencies: generated codeunit machinery escaped session storage\n' >&2
  exit 1
fi

compile_header runtime/HttpServer.h "$proof/HttpServer.h.d"
for forbidden in microhttpd.h Database.h Session.h Page.h thread condition_variable mutex deque; do
  reject_dependency "$proof/HttpServer.h.d" "$forbidden"
done
compile_header runtime/HttpServer.h "$proof/forced-microhttpd.h.d" -include microhttpd.h
if reject_dependency "$proof/forced-microhttpd.h.d" microhttpd.h > "$proof/forced-microhttpd.h.log" 2>&1; then
  printf 'header-dependencies: native HTTP backend escaped the public transport boundary\n' >&2
  exit 1
fi

for forbidden in filesystem regex; do
  if [ "$forbidden" = filesystem ]; then header=RuntimeSurface.h; else header=dotnet/Regex.h; fi
  compile_header "$header" "$proof/forced-$forbidden.d" -include "$forbidden"
  if reject_dependency "$proof/forced-$forbidden.d" "$forbidden" \
    > "$proof/forced-$forbidden.log" 2>&1; then
    printf 'header-dependencies: forced <%s> escaped the control\n' "$forbidden" >&2
    exit 1
  fi
done
printf 'header-dependencies: nineteen standalone headers; filesystem/regex/crypto/Array/JSON/report/module/table/page/Unicode/HTTP/session controls refused\n'
