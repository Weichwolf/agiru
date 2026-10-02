# agiru -- the transpiler under src/al and src/gen, the runtime under src/rt, the translated BaseApp
# under apps/. This Makefile is the ONE way in: nothing is started by reaching past it into a
# script. That CMake and Ninja work behind it is not a second mechanism, it is what stands behind
# the door.
SHELL := /bin/bash
.DEFAULT_GOAL := all
.NOTPARALLEL:
SELF := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
B    := $(SELF)/build
JOBS ?= $(shell nproc)
ifeq ($(origin CXX),default)
CXX := clang++-19
endif
export CXX
CCACHE_SLOPPINESS ?= pch_defines,time_macros
export CCACHE_SLOPPINESS

.PHONY: all apps builtins census comments cronus db gap gate lint lint-one schema tc test transpile tree provision doc clean spotless help demo symbols gates ut verify verify-start verify-status
.PHONY: lint-config include-cost slice-check interface-defaults report-layouts

# `make` DELETES THE COMMENTS IN `src/` BEFORE IT BUILDS. AGENTS.md states the rule -- `include/` is
# documented and `src/` is not -- and a rule that only nags is one somebody is always about to get
# to. Deleting does not destroy: every line removed is in the commit that added it, which is where
# a reason belongs. The door keeps its Doxygen, and `make lint` counts what is undocumented there.
# `KEEP=1` BUILDS PAST THE FIRST ERROR. One error per build is one error per quarter hour when the
# slice is 1 500 sources; with it a round reports every unit that fails and the next edit answers a
# CLASS of them. The default stays the stop, because a green build has to mean the first error too.
all: comments db   ## strip the comments, then the library, the transpiler and the client
	@start=$$(date +%s); cmake --build $(B) -j $(JOBS) $(if $(KEEP),-- -k 0); status=$$?; end=$$(date +%s); \
	  printf '%s all %ss %s slice sources exit %s\n' "$$(date +%FT%T)" "$$((end - start))" \
	    "$$(grep -vc '^#' $(SELF)/test/slice)" "$$status" >> $(B)/times.log; exit $$status

# Formatting is idempotent and failures are not swallowed.
comments:          ## strip source comments and format changed handwritten C++
	@python3 $(SELF)/test/strip-comments.py $(SELF)/src $(SELF)/include
	@python3 $(SELF)/test/format-changed.py

db: $(B)/CMakeCache.txt   ## compile_commands.json for clangd and clang-tidy
	@selected=$$(command -v "$(CXX)") || exit 2; \
	  cached=$$(sed -n 's/^CMAKE_CXX_COMPILER:[^=]*=//p' $(B)/CMakeCache.txt); \
	  cached=$$(command -v "$$cached") || exit 2; \
	  if [ "$$(readlink -f "$$selected")" != "$$(readlink -f "$$cached")" ]; then \
	    printf 'Compiler mismatch: build uses %s, CXX requests %s. Use a separate B directory for the requested Clang compiler.\n' "$$cached" "$$selected" >&2; exit 2; \
	  fi
	@cmake --build "$(B)" --target build.ninja
	@ln -sf $(B)/compile_commands.json $(SELF)/compile_commands.json

$(B)/CMakeCache.txt:
	@cmake -S $(SELF) -B $(B) -G Ninja \
	  -DCMAKE_CXX_COMPILER=$(CXX) \
	  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

lint: lint-config gates tc ## format and analysis over what changed (FULL=1: the whole tree and the baselines)
	@AGIRU_AL_SOURCE=$${AGIRU_AL_SOURCE:-$$HOME/Git/BCApps/src/Layers/W1/BaseApp} AGIRU_BC_SOURCE=$${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src} JOBS=$(JOBS) FULL=$(FULL) sh $(SELF)/test/lint.sh

lint-config:       ## prove the clang-tidy function line limit at its boundary
	@B="$(B)" bash "$(SELF)/test/function-size.sh"

include-cost:      ## measure standalone header frontend cost without PCH
	@B="$(B)" bash "$(SELF)/scripts/include_cost.sh" $(HEADERS)

slice-check:       ## count every slice source and refuse missing inputs without compiling
	@B="$(B)" bash "$(SELF)/scripts/slice_check.sh"

interface-defaults: comments db tc ## execute generated interface defaults and refusal controls
	@cmake --build "$(B)" -j "$(JOBS)" --target gate_GenInterfaceGate
	@"$(B)/gate_GenInterfaceGate"
	@B="$(B)" bash "$(SELF)/test/interface-defaults.sh"

report-layouts: comments db tc ## retain named report layouts, actual assets and extension counts
	@cmake --build "$(B)" -j "$(JOBS)" --target gate_ReportLayoutGate
	@"$(B)/gate_ReportLayoutGate"
	@B="$(B)" bash "$(SELF)/test/report-layouts.sh"

lint-one: export AGIRU_LINT_UNIT = $(UNIT)
lint-one: comments db ## analyse one configured unit without a build (UNIT=src/rt/Transaction.cpp)
	@python3 $(SELF)/test/lint-analysis.py --tidy clang-tidy-19 --jobs 1 --require-unit

test: gates tc     ## the fast gate
	@B="$(B)" sh $(SELF)/test/run.sh

census:           ## raw AL inventory; namespace selection is diagnostic, never a filter
	@mkdir -p "$(B)"
	@python3 "$(SELF)/scripts/scope_inventory.py" "$${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src}" \
	  --output "$(B)/scope-inventory.json"

# TRANSPILING NEEDS THE TRANSPILER AND NOTHING ELSE. `all` now builds the slice out of apps/, so
# hanging transpile off it would make the tree depend on its own output -- and a slice file that
# stopped compiling would take away the one command that repairs it.
tc: db             ## just the transpiler
	@cmake --build $(B) -j $(JOBS) --target agirutc

transpile: tc      ## every app in apps.json through the transpiler into apps/
	@$(B)/agirutc $${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src} $(SELF)/apps.json $(SELF)/apps

gap: db            ## a ranked header gap (SOURCE=1: bodies; SWEEP=1: complete header sweep)
	@if [ -z "$(SOURCE)" ] && [ "$(SWEEP)" != 1 ] && [ ! -s $(B)/tree-syntax/roots ]; then \
	  JOBS=$(JOBS) sh $(SELF)/scripts/tree_syntax.sh $(SELF)/apps || exit $$?; \
	fi
	@SOURCE=$(SOURCE) SWEEP=$(SWEEP) sh $(SELF)/scripts/first_gap.sh $(SELF)/apps

# BARE `make` IS `src/`, AND `make apps` IS THE GENERATED TREE. They are separate build directories
# because they are separate questions: the runtime and the transpiler must stand on their own, and
# 5 835 generated translation units must never be in the way of the one-second loop that repairs
# them. Ninja stops at the first failing edge, which is the point -- every error in `apps/` is one
# generic gap in `src/`, so the second error is almost always the first one again.
apps: all          ## the generated tree, stopping at the first error
	@cmake -S $(SELF) -B $(B)/apps -G Ninja \
	  -DCMAKE_CXX_COMPILER=$(CXX) -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
	  -DAGIRU_BUILD_APPS=ON > /dev/null
	@cmake --build $(B)/apps -j $(JOBS) -- -k 1

tree: db           ## how much of the generated tree the compiler accepts
	@JOBS=$(JOBS) sh $(SELF)/scripts/tree_syntax.sh

builtins:          ## which AL builtins the UT milestone calls, ranked
	@python3 $(SELF)/scripts/builtin_rank.py

schema:            ## how much of the CRONUS dataset the transpiled schema can hold
	@python3 $(SELF)/scripts/schema_gap.py

demo:              ## the CRONUS rows into the runner's database, so a test starts where BC does
	@python3 $(SELF)/scripts/seed_demo.py

symbols:           ## the AL source of every platform object, out of the system symbols
	@python3 $(SELF)/scripts/fetch_symbols.py

cronus:            ## the demo database from the CDN into PostgreSQL, one to one
	@sh $(SELF)/scripts/fetch_artifact.sh
	@sh $(SELF)/scripts/mssql_restore.sh
	@python3 $(SELF)/scripts/cronus_to_pg.py

provision:         ## MSSQL container, BC demo .bak from the CDN, PostgreSQL master
	@sh $(SELF)/scripts/provision.sh

doc:               ## the door's documentation -> build/doc
	@doxygen $(SELF)/doc/Doxyfile

clean:             ## remove build artefacts
	@python3 $(SELF)/scripts/verify_snapshot.py clean
	@rm -rf $(B) $(SELF)/compile_commands.json

spotless: clean    ## and the downloaded artefact with it
	@rm -rf $(SELF)/work

help:              ## this list
	@grep -hE '^[a-z][a-z0-9-]*:.*##' $(MAKEFILE_LIST) | sed 's/:.*##/\t/' | expand -t14

gates: comments db  ## build only the handwritten C++ gates for the repair loop
	@cmake --build $(B) -j $(JOBS) --target gates

gate: export AGIRU_GATE = $(GATE)
gate: comments db   ## build and run one C++ gate (GATE=RecordRefGate)
	@case "$$AGIRU_GATE" in ''|*[!a-zA-Z0-9_]*) \
	  printf 'GATE must name a C++ gate, for example RecordRefGate\n' >&2; exit 2;; esac; \
	  if [ ! -f "$(SELF)/test/gate/$$AGIRU_GATE.cpp" ]; then \
	    printf 'Unknown gate: %s\n' "$$AGIRU_GATE" >&2; exit 2; \
	  fi; \
	  cmake --build "$(B)" -j $(JOBS) --target "gate_$$AGIRU_GATE" && \
	    "$(B)/gate_$$AGIRU_GATE"

UT_LOG ?= $(B)/ut.log
ut:                ## count source tests, build, then run the UT milestone on disposable clones
	@bash "$(SELF)/scripts/ut-milestone.sh" "$(UT_LOG)" "$(JOBS)" --build

VERIFY_TARGETS ?= all test
verify:             ## freeze this worktree and verify the copy (VERIFY_TARGETS overrides jobs)
	@python3 $(SELF)/scripts/verify_snapshot.py start --reuse --jobs $(JOBS) $(VERIFY_TARGETS)

verify-start:       ## start snapshot verification in the background; source edits may continue
	@python3 $(SELF)/scripts/verify_snapshot.py start --reuse --detach --jobs $(JOBS) $(VERIFY_TARGETS)

verify-status:      ## show the latest snapshot verification result and log location
	@python3 $(SELF)/scripts/verify_snapshot.py status
