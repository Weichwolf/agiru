# THE LAYERING IS THE BUILD AND IT IS DECLARED ONCE -- in src/<tier>/reaches. This file READS those
# and keeps no second map: a second one goes stale, drops a tier out of the archive and breaks the
# build somewhere that has nothing to do with it.
#
# A tier's include path is its own directory plus the directories its `reaches` names, and nothing
# else. A tier break therefore fails at the `#include` with a file and a line rather than later in
# the linker.
cmake_minimum_required(VERSION 3.28)
project(agiru CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# A WARNING IS AN ERROR, and that holds for the generated tree as much as the written one.
# `src/app/` falls out of the static analysis because a finding there has no address; it does NOT
# fall out of the compiler, and that is the hold which replaces it.
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_program(LLD ld.lld-19 ld.lld)
if(LLD)
  add_link_options(-fuse-ld=lld)
endif()

# The tiers in dependency order. The order here is a BUILD order; who may see whom is still said by
# the `reaches` file alone.
set(TIERS al net db gen rt cli tc)

# THE LAYERING IS CHECKED BEFORE ANY SOURCE IS LOOKED AT. A tier without a .cpp is skipped below;
# its `reaches` would then never have been read, and a typo in it would have slept until its first
# file arrived.
foreach(tier IN LISTS TIERS)
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/${tier}/reaches")
    message(FATAL_ERROR "src/${tier}/ carries no reaches file.")
  endif()
  file(STRINGS "${CMAKE_SOURCE_DIR}/src/${tier}/reaches" reached REGEX "^[a-z]+$")
  foreach(dep IN LISTS reached)
    if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/${dep}")
      message(FATAL_ERROR "src/${tier}/reaches names '${dep}' -- no such tier.")
    endif()
  endforeach()
endforeach()

foreach(tier IN LISTS TIERS)
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/${tier}/*.cpp")
  if(NOT sources)
    continue()
  endif()
  if(tier STREQUAL cli)
    add_executable(agiru ${sources})
    set(target agiru)
  elseif(tier STREQUAL tc)
    add_executable(agirutc ${sources})
    set(target agirutc)
  else()
    # SHARED, so that an app can be built without the runtime being rebuilt into it and so that one
    # copy serves every process. That is what "an app compiles independently of the runtime" means
    # in practice: an app is bound to the DOOR, and a change inside the runtime relinks the runtime.
    add_library(agiru_${tier} SHARED ${sources})
    set(target agiru_${tier})
  endif()

  # Its own tier, the door, and exactly what `reaches` names.
  target_include_directories(${target} PRIVATE
    "${CMAKE_SOURCE_DIR}/src/${tier}" "${CMAKE_SOURCE_DIR}/include")
  file(STRINGS "${CMAKE_SOURCE_DIR}/src/${tier}/reaches" reached REGEX "^[a-z]+$")
  foreach(dep IN LISTS reached)
    target_include_directories(${target} PRIVATE "${CMAKE_SOURCE_DIR}/src/${dep}")
    if(TARGET agiru_${dep})
      target_link_libraries(${target} PRIVATE agiru_${dep})
    endif()
  endforeach()
endforeach()

if(TARGET agiru_db)
  find_package(PostgreSQL REQUIRED)
  target_link_libraries(agiru_db PUBLIC PostgreSQL::PostgreSQL)
endif()


# NO ENVIRONMENT VARIABLE DECIDES A RESULT. The gate's database is a declared build setting, not
# something the shell happens to be carrying: a case that reads its target from the environment
# passes or fails for reasons the tree cannot see.
set(AGIRU_TEST_DSN "postgresql://agiru:agiru@localhost:5433/agiru_master"
    CACHE STRING "Connection string the gate uses")
set(AGIRU_AL_SOURCE "$ENV{HOME}/Git/BCApps/src/Layers/W1/BaseApp"
    CACHE STRING "The AL source the gate parses")
# The root the TRANSPILER reads, which is one level up: apps.json names each app's tree beneath it.
set(AGIRU_BC_SOURCE "$ENV{HOME}/Git/BCApps/src"
    CACHE STRING "The BCApps src root the transpiler reads apps.json against")

# THE APPS. Each is one BC app -- the unit of deployment, version and dependency -- and becomes one
# library, so that the LINKER enforces the direction AL declares: the Base Application may not know
# an extension, only the other way round and only through events.
#
# An app sees the door and the apps it depends on, never the runtime's internals. An app with no
# sources yet is skipped rather than declared empty: the generator has not written it.
# THE GENERATED TREE IS NOT BUILT UNTIL IT COMPILES. Turning it on while the runtime is short of
# what it uses would make `make` red for a reason that is already measured elsewhere -- the AL
# surface baseline says how much of AL is reachable, and that is the number that has to rise. The
# day it is enough, this defaults to ON and the option goes.
option(AGIRU_BUILD_APPS "Build the transpiled apps under apps/" OFF)

file(GLOB app_dirs RELATIVE "${CMAKE_SOURCE_DIR}/apps" CONFIGURE_DEPENDS "apps/*")
foreach(app IN LISTS app_dirs)
  if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/apps/${app}")
    continue()
  endif()
  file(GLOB_RECURSE app_sources CONFIGURE_DEPENDS "apps/${app}/*.cpp")
  if(NOT app_sources OR NOT AGIRU_BUILD_APPS)
    continue()
  endif()
  add_library(agiru_app_${app} SHARED ${app_sources})
  target_include_directories(agiru_app_${app} PRIVATE
    "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/apps/${app}")
  target_link_libraries(agiru_app_${app} PRIVATE agiru_rt)
  target_link_libraries(agiru PRIVATE agiru_app_${app})
endforeach()

# THE GATE. Every case under test/gate/ is its own program with its own `main`, returning red or
# green -- no framework, no registration, no collected binary that takes everything down with one
# crash.
# The target image is compiled INTO every case, because it is the thing under test: it is what
# `src/app/` shall look like, and a specification that does not compile is not one.
file(GLOB target_image CONFIGURE_DEPENDS "test/target/*.cpp")

file(GLOB gate_cases CONFIGURE_DEPENDS "test/gate/*.cpp")
foreach(case IN LISTS gate_cases)
  get_filename_component(name "${case}" NAME_WE)
  add_executable(gate_${name} "${case}" ${target_image})
  # A CASE MAY SEE EVERY TIER. It is not a client: it exists to test the tiers themselves, and a
  # parser or a generator has no public door to be tested through.
  target_include_directories(gate_${name} PRIVATE
    "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/test/gate"
    "${CMAKE_SOURCE_DIR}/test/target")
  foreach(tier IN LISTS TIERS)
    target_include_directories(gate_${name} PRIVATE "${CMAKE_SOURCE_DIR}/src/${tier}")
  endforeach()
  target_compile_definitions(gate_${name} PRIVATE AGIRU_TEST_DSN="${AGIRU_TEST_DSN}"
                                                AGIRU_AL_SOURCE="${AGIRU_AL_SOURCE}"
                                                AGIRU_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  foreach(tier IN LISTS TIERS)
    if(TARGET agiru_${tier})
      target_link_libraries(gate_${name} PRIVATE agiru_${tier})
    endif()
  endforeach()
endforeach()
