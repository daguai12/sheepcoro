file(GLOB_RECURSE SHEEPCORO_BENCH_SOURCES "${PROJECT_SOURCE_DIR}/benchmark/*/*.cpp")

add_custom_target(build-bench COMMAND echo "Building benchmark case...")

add_custom_target(build-benchtools
  COMMAND cargo run --release --manifest-path=${PROJECT_SOURCE_DIR}/third_party/rust_echo_bench/Cargo.toml -- --help
)

foreach (sheepcoro_bench_source ${SHEEPCORO_BENCH_SOURCES})
  get_filename_component(sheepcoro_bench_filename ${sheepcoro_bench_source} NAME)
  string(REPLACE ".cpp" "" sheepcoro_bench_name ${sheepcoro_bench_filename})
  add_executable(${sheepcoro_bench_name} EXCLUDE_FROM_ALL ${sheepcoro_bench_source})
  add_dependencies(build-bench ${sheepcoro_bench_name})

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${sheepcoro_bench_name} PRIVATE "-g")
  endif()
  if(ENABLE_COMPILE_OPTIMIZE)
    target_compile_options(${sheepcoro_bench_name} PUBLIC -O3)
  endif()
  target_link_libraries(${sheepcoro_bench_name} ${PROJECT_NAME})

  set_target_properties(${sheepcoro_bench_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmark"
  )

  string(REPLACE "_bench" "" sheepcoro_bench_command ${sheepcoro_bench_name})
  add_custom_target(bench_${sheepcoro_bench_command}
    COMMAND $<TARGET_FILE:${sheepcoro_bench_name}>
    DEPENDS ${sheepcoro_bench_name}
    COMMENT "Running ${sheepcoro_bench_command} bench..."
  )
endforeach()
