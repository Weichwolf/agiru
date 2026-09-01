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
set(TIERS al net db gen rt app cli)

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
  else()
    add_library(agiru_${tier} STATIC ${sources})
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

# THE GATE. Every case under test/gate/ is its own program with its own `main`, returning red or
# green -- no framework, no registration, no collected binary that takes everything down with one
# crash.
file(GLOB gate_cases CONFIGURE_DEPENDS "test/gate/*.cpp")
foreach(case IN LISTS gate_cases)
  get_filename_component(name "${case}" NAME_WE)
  add_executable(gate_${name} "${case}")
  target_include_directories(gate_${name} PRIVATE
    "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/test/gate")
  foreach(tier IN LISTS TIERS)
    if(TARGET agiru_${tier})
      target_link_libraries(gate_${name} PRIVATE agiru_${tier})
    endif()
  endforeach()
endforeach()
