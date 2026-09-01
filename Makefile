# agiru -- der Transpiler unter src/al und src/gen, die Runtime unter src/rt, die uebersetzte
# BaseApp unter src/app. Dieses Makefile ist der EINE Eingang: nichts wird gestartet, indem an ihm
# vorbei in ein Skript gegriffen wird. Dass darunter CMake und Ninja arbeiten, ist kein zweiter
# Mechanismus, sondern das, was hinter der Tuer steht.
SHELL := /bin/bash
.DEFAULT_GOAL := all
SELF := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
B    := $(SELF)/build

.PHONY: all db lint test transpile provision doc clean spotless help

all: db            ## die Bibliothek, den Transpiler und den Client daneben
	@cmake --build $(B) -j $(shell nproc)

db: $(B)/CMakeCache.txt   ## compile_commands.json fuer clangd und clang-tidy
	@ln -sf $(B)/compile_commands.json $(SELF)/compile_commands.json

$(B)/CMakeCache.txt:
	@cmake -S $(SELF) -B $(B) -G Ninja \
	  -DCMAKE_CXX_COMPILER=clang++ \
	  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

lint: db           ## Format, statische Analyse, die Tuer
	@sh $(SELF)/test/lint.sh

test: all          ## das schnelle Gate
	@sh $(SELF)/test/run.sh

transpile: all     ## die BaseApp durch den Transpiler nach src/app/
	@$(B)/agiru transpile

provision:         ## MSSQL-Container, BC-Demo-.bak vom CDN, PostgreSQL-Master
	@sh $(SELF)/scripts/provision.sh

doc:               ## die Dokumentation der Tuer -> build/doc
	@doxygen $(SELF)/doc/Doxyfile

clean:             ## Bauartefakte entfernen
	@rm -rf $(B) $(SELF)/compile_commands.json

spotless: clean    ## und das heruntergeladene Artefakt dazu
	@rm -rf $(SELF)/work

help:              ## diese Liste
	@grep -hE '^[a-z]+:.*##' $(MAKEFILE_LIST) | sed 's/:.*##/\t/' | expand -t14
