# agiru -- the transpiler under src/al and src/gen, the runtime under src/rt, the translated BaseApp
# under apps/. This Makefile is the ONE way in: nothing is started by reaching past it into a
# script. That CMake and Ninja work behind it is not a second mechanism, it is what stands behind
# the door.
SHELL := /bin/bash
.DEFAULT_GOAL := all
SELF := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
B    := $(SELF)/build

.PHONY: all apps builtins comments cronus db gap lint schema tc test transpile tree provision doc clean spotless help

# `make` DELETES THE COMMENTS IN `src/` BEFORE IT BUILDS. CLAUDE.md states the rule -- `include/` is
# documented and `src/` is not -- and a rule that only nags is one somebody is always about to get
# to. Deleting does not destroy: every line removed is in the commit that added it, which is where
# a reason belongs. The door keeps its Doxygen, and `make lint` counts what is undocumented there.
all: comments db   ## strip the comments, then the library, the transpiler and the client
	@cmake --build $(B) -j $(shell nproc)

# THE FORMATTER RUNS AFTER THE STRIP, because removing a line changes what fits on the next one and
# `make lint` would otherwise fail on a tree `make` just wrote. The two together are idempotent.
comments:          ## delete every comment in src/; include/ keeps its Doxygen
	@python3 $(SELF)/test/strip-comments.py $(SELF)/src $(SELF)/include
	@git -C $(SELF) diff --name-only --diff-filter=ACM -- '*.h' '*.cpp' 2>/dev/null | \
	  grep -v '^apps/' | xargs -r clang-format -i 2>/dev/null || true

db: $(B)/CMakeCache.txt   ## compile_commands.json for clangd and clang-tidy
	@ln -sf $(B)/compile_commands.json $(SELF)/compile_commands.json

$(B)/CMakeCache.txt:
	@cmake -S $(SELF) -B $(B) -G Ninja \
	  -DCMAKE_CXX_COMPILER=clang++ \
	  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

lint: all          ## format and analysis over what changed (FULL=1: the whole tree and the baselines)
	@AGIRU_AL_SOURCE=$${AGIRU_AL_SOURCE:-$$HOME/Git/BCApps/src/Layers/W1/BaseApp} AGIRU_BC_SOURCE=$${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src} FULL=$(FULL) sh $(SELF)/test/lint.sh

test: all          ## the fast gate
	@sh $(SELF)/test/run.sh

# TRANSPILING NEEDS THE TRANSPILER AND NOTHING ELSE. `all` now builds the slice out of apps/, so
# hanging transpile off it would make the tree depend on its own output -- and a slice file that
# stopped compiling would take away the one command that repairs it.
tc: db             ## just the transpiler
	@cmake --build $(B) -j $(shell nproc) --target agirutc

transpile: tc      ## every app in apps.json through the transpiler into apps/
	@$(B)/agirutc $${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src} $(SELF)/apps.json $(SELF)/apps

gap: db            ## the first generated header that does not compile (SOURCE=1: the bodies)
	@SOURCE=$(SOURCE) SWEEP=$(SWEEP) sh $(SELF)/scripts/first_gap.sh $(SELF)/apps

# BARE `make` IS `src/`, AND `make apps` IS THE GENERATED TREE. They are separate build directories
# because they are separate questions: the runtime and the transpiler must stand on their own, and
# 5 835 generated translation units must never be in the way of the one-second loop that repairs
# them. Ninja stops at the first failing edge, which is the point -- every error in `apps/` is one
# generic gap in `src/`, so the second error is almost always the first one again.
apps: all          ## the generated tree, stopping at the first error
	@cmake -S $(SELF) -B $(B)/apps -G Ninja \
	  -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
	  -DAGIRU_BUILD_APPS=ON > /dev/null
	@cmake --build $(B)/apps -j $(shell nproc) -- -k 1

tree: db           ## how much of the generated tree the compiler accepts
	@sh $(SELF)/scripts/tree_syntax.sh

builtins:          ## which AL builtins the UT milestone calls, ranked
	@python3 $(SELF)/scripts/builtin_rank.py

schema:            ## how much of the CRONUS dataset the transpiled schema can hold
	@python3 $(SELF)/scripts/schema_gap.py

cronus:            ## the demo database from the CDN into PostgreSQL, one to one
	@sh $(SELF)/scripts/fetch_artifact.sh
	@sh $(SELF)/scripts/mssql_restore.sh
	@python3 $(SELF)/scripts/cronus_to_pg.py

provision:         ## MSSQL container, BC demo .bak from the CDN, PostgreSQL master
	@sh $(SELF)/scripts/provision.sh

doc:               ## the door's documentation -> build/doc
	@doxygen $(SELF)/doc/Doxyfile

clean:             ## remove build artefacts
	@rm -rf $(B) $(SELF)/compile_commands.json

spotless: clean    ## and the downloaded artefact with it
	@rm -rf $(SELF)/work

help:              ## this list
	@grep -hE '^[a-z]+:.*##' $(MAKEFILE_LIST) | sed 's/:.*##/\t/' | expand -t14
