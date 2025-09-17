file(GLOB_RECURSE SHEEPCORO_BENCHTEST_SOURCES "${PROJECT_SOURCE_DIR}/benchtests/*bench.cpp")

set(benchbuild_list "")
set(benchtest_list "")

add_custom_target(_build-benchtests)

foreach (sheepcoro_benchtest_source ${SHEEPCORO_BENCHTEST_SOURCES})
  get_filename_component(sheepcoro_benchtest_filename ${sheepcoro_benchtest_source} NAME)
  string(REPLACE ".cpp" "" sheepcoro_benchtest_name ${sheepcoro_benchtest_filename})
  add_executable(${sheepcoro_benchtest_name} EXCLUDE_FROM_ALL ${sheepcoro_benchtest_source})
  add_dependencies(_build-benchtests ${sheepcoro_benchtest_name})

  target_link_libraries(${sheepcoro_benchtest_name} ${PROJECT_NAME} benchmark)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${sheepcoro_benchtest_name} PRIVATE "-g")
  endif()
  if(ENABLE_COMPILE_OPTIMIZE)
    target_compile_options(${sheepcoro_benchtest_name} PUBLIC -O3)
  endif()

  set_target_properties(${sheepcoro_benchtest_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchtests"
    COMMAND ${sheepcoro_benchtest_name}
  )

  string(REPLACE "_bench" "" sheepcoro_benchtest_command ${sheepcoro_benchtest_name})
  add_custom_target(benchbuild-${sheepcoro_benchtest_command}
    COMMAND echo "build ${sheepcoro_benchtest_command} benchtest..."
    DEPENDS ${sheepcoro_benchtest_name}
    COMMENT "build ${sheepcoro_benchtest_command} benchtests..."
  )

  add_custom_target(benchtest-${sheepcoro_benchtest_command}
    COMMAND $<TARGET_FILE:${sheepcoro_benchtest_name}>
    DEPENDS ${sheepcoro_benchtest_name}
    COMMENT "Running ${sheepcoro_benchtest_command} benchtests..."
  )

  list(APPEND benchbuild_list benchbuild-${sheepcoro_benchtest_command})
  list(APPEND benchtest_list benchtest-${sheepcoro_benchtest_command})
endforeach()

add_custom_target(build-benchtests 
  COMMAND echo "[benchtests build command]"
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/print_list.sh ${benchbuild_list}
  COMMAND echo "[benchtests run command]"
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/print_list.sh ${benchtest_list}
  DEPENDS _build-benchtests
)
