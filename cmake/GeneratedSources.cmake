set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/scope.json" "${CMAKE_SOURCE_DIR}/apps.json"
  "${CMAKE_SOURCE_DIR}/apps/generation-sources.txt"
  "${CMAKE_SOURCE_DIR}/apps/generation-scope.json"
  "${CMAKE_SOURCE_DIR}/apps/generation-apps.json"
  "${CMAKE_SOURCE_DIR}/apps/source-origins.json"
  "${CMAKE_SOURCE_DIR}/scripts/build_sources.py"
  "${CMAKE_SOURCE_DIR}/scripts/scope_inventory.py"
  "${CMAKE_SOURCE_DIR}/scripts/unity_groups.py"
  "${CMAKE_SOURCE_DIR}/scripts/ut_manifest.py")

function(agiru_generated_sources name output)
  execute_process(
    COMMAND python3 "${CMAKE_SOURCE_DIR}/scripts/build_sources.py" project
      --root "${CMAKE_SOURCE_DIR}" ${ARGN} --configure
      --receipt "${CMAKE_BINARY_DIR}/build-sources/${name}-configure.json"
    RESULT_VARIABLE status OUTPUT_VARIABLE rows ERROR_VARIABLE error
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "${error}")
  endif()
  string(REPLACE "\n" ";" rows "${rows}")
  set(${output} "${rows}" PARENT_SCOPE)
  add_custom_target(agiru_inputs_${name}
    COMMAND python3 "${CMAKE_SOURCE_DIR}/scripts/build_sources.py" project
      --root "${CMAKE_SOURCE_DIR}" ${ARGN} --check
      --receipt "${CMAKE_BINARY_DIR}/build-sources/${name}.json"
    VERBATIM)
endfunction()

function(agiru_generated_app_sources app output)
  agiru_generated_sources("app_${app}" rows --app "${app}")
  set(paths)
  foreach(source IN LISTS rows)
    list(APPEND paths "${CMAKE_SOURCE_DIR}/apps/${source}")
  endforeach()
  if(paths)
    set_source_files_properties(${paths} PROPERTIES GENERATED TRUE)
  endif()
  set(${output} "${paths}" PARENT_SCOPE)
endfunction()
