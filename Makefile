# agiru -- the transpiler under src/al and src/gen, the runtime under src/rt, the translated BaseApp
# under apps/. This Makefile is the ONE way in: nothing is started by reaching past it into a
# script. That CMake and Ninja work behind it is not a second mechanism, it is what stands behind
# the door.
SHELL := /bin/bash
.DEFAULT_GOAL := all
SELF := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
B    := $(SELF)/build

.PHONY: all db lint test transpile provision doc clean spotless help

all: db            ## the library, the transpiler, and the client beside them
	@cmake --build $(B) -j $(shell nproc)

db: $(B)/CMakeCache.txt   ## compile_commands.json for clangd and clang-tidy
	@ln -sf $(B)/compile_commands.json $(SELF)/compile_commands.json

$(B)/CMakeCache.txt:
	@cmake -S $(SELF) -B $(B) -G Ninja \
	  -DCMAKE_CXX_COMPILER=clang++ \
	  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

lint: all          ## format, static analysis, the door, the AL population
	@AGIRU_AL_SOURCE=$${AGIRU_AL_SOURCE:-$$HOME/Git/BCApps/src/Layers/W1/BaseApp} sh $(SELF)/test/lint.sh

test: all          ## the fast gate
	@sh $(SELF)/test/run.sh

transpile: all     ## the BaseApp through the transpiler into apps/
	@$(B)/agirutc

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
