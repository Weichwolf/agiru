# DIE SCHICHTUNG IST DER BUILD UND SIE IST EINMAL DEKLARIERT -- in src/<stufe>/reaches. Diese Datei
# LIEST sie und haelt keine zweite Karte: eine zweite veraltet, laesst eine Stufe aus dem Archiv
# fallen und bricht den Build an einer Stelle, die nichts damit zu tun hat.
#
# Der Include-Pfad einer Stufe ist ihr eigenes Verzeichnis plus die Verzeichnisse, die ihre
# `reaches` nennt -- und sonst nichts. Ein Stufenbruch scheitert deshalb am `#include` mit Datei und
# Zeile, nicht spaeter im Linker.
cmake_minimum_required(VERSION 3.28)
project(agiru CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# EINE WARNUNG IST EIN FEHLER, und das gilt fuer den generierten Baum wie fuer den geschriebenen.
# `src/app/` faellt aus der statischen Analyse heraus, weil ein Fund dort keine Adresse hat; aus dem
# Compiler faellt es NICHT, und das ist der Halt, der es ersetzt.
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_program(LLD ld.lld-19 ld.lld)
if(LLD)
  add_link_options(-fuse-ld=lld)
endif()

# Die Stufen in Abhaengigkeitsreihenfolge. Die Reihenfolge hier ist eine BAUREIHENFOLGE; wer wen
# sehen darf, sagt weiterhin allein die `reaches`-Datei.
set(TIERS al net db gen rt app cli)

# DIE SCHICHTUNG WIRD GEPRUEFT, BEVOR EINE QUELLE ANGESEHEN WIRD. Eine Stufe ohne .cpp wird unten
# uebersprungen; ihre `reaches` waere damit nie gelesen worden, und ein Tippfehler darin haette bis
# zu ihrer ersten Datei geschlafen.
foreach(tier IN LISTS TIERS)
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/${tier}/reaches")
    message(FATAL_ERROR "src/${tier}/ traegt keine reaches-Datei.")
  endif()
  file(STRINGS "${CMAKE_SOURCE_DIR}/src/${tier}/reaches" reached REGEX "^[a-z]+$")
  foreach(dep IN LISTS reached)
    if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/${dep}")
      message(FATAL_ERROR "src/${tier}/reaches nennt '${dep}' -- diese Stufe gibt es nicht.")
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

  # Die eigene Stufe, die Tuer, und genau das, was `reaches` nennt.
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
