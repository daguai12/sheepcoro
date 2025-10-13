include(GoogleTest)

file(GLOB_RECURSE SHEEPCORO_TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.cpp")

add_custom_target(build-tests COMMAND ${CMAKE_CTEST_COMMAND} --show-only)
add_custom_target(build-check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

foreach(sheepcoro_test_source ${SHEEPCORO_TEST_SOURCES})
    get_filename_component(sheepcoro_test_filename ${sheepcoro_test_source} NAME)
    string(REPLACE ".cpp" "" sheepcoro_test_name ${sheepcoro_test_filename})
    add_executable(${sheepcoro_test_name} EXCLUDE_FROM_ALL ${sheepcoro_test_source})
    add_dependencies(build-tests ${sheepcoro_test_name})
    add_dependencies(build-check ${sheepcoro_test_name})

    gtest_discover_tests(${sheepcoro_test_name}
        EXTRA_ARGS
        --gtest_color=auto
        --gtest_output=xml:${CMAKE_BINARY_DIR}/tests/${sheepcoro_test_name}.xml
        --gtest_catch_exceptions=0
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
        DISCOVERY_TIMEOUT 120
        PROPERTIES
        TIMEOUT 120
    )

    target_link_libraries(${sheepcoro_test_name}  ${PROJECT_NAME} gtest)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${sheepcoro_test_name} PRIVATE "-g")
    endif()
    if(ENABLE_COMPLIE_OPTIMIZE)
        target_compile_definitions(${sheepcoro_test_name} PRIVATE -O3)
    endif()

    set_target_properties(${sheepcoro_test_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    string(REPLACE "_test" "" sheepcoro_test_command ${sheepcoro_test_name})
    add_custom_target(build-${sheepcoro_test_command}
        COMMAND echo "build ${sheepcoro_test_name} test..."
        DEPENDS ${sheepcoro_test_name}
        COMMENT "build ${sheepcoro_test_command} tests..."
    )

    add_custom_target(check-${sheepcoro_test_command}
        COMMAND $<TARGET_FILE:${sheepcoro_test_name}>
        DEPENDS ${sheepcoro_test_name}
        COMMENT "Running ${sheepcoro_test_command} test..."
    )

    add_custom_target(memtest-${sheepcoro_test_command}
        COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/memcheck.sh
                ${PROJECT_SOURCE_DIR}/temp/valgrind_output.xml
                $<TARGET_FILE:${sheepcoro_test_name}>
                ${PROJECT_SOURCE_DIR}/temp
        COMMAND ${python_command} ${PROJECT_SOURCE_DIR}/scripts/analysis_valgrind.py
                ${PROJECT_SOURCE_DIR}/temp/valgrind_output.xml
        $<TARGET_FILE:${sheepcoro_test_name}>
        DEPENDS ${sheepcoro_test_name}
        COMMENT "Running ${sheepcoro_test_command} memtests..."
    )
endforeach()

add_executable(echo1 EXCLUDE_FROM_ALL net_tcp_test.cpp)
add_dependencies(build-tests echo1)
add_dependencies(build-check echo1)
target_link_libraries(echo1 ${PROJECT_NAME})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(echo1 PRIVATE "-g")
endif()
if(ENABLE_COMPILE_OPTIMIZE)
  target_compile_options(echo1 PUBLIC -O3)
endif()
set_target_properties(${tinycoro_test_name}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

add_custom_target(build-echo1
  COMMAND echo "build echo test..."
  DEPENDS echo1
  COMMENT "build echo tests..."
)

add_custom_target(test-echo1
  COMMAND ${python_command} ${PROJECT_SOURCE_DIR}/tests/echo.py $<TARGET_FILE:echo1> ${PROJECT_SOURCE_DIR}/third_party/rust_echo_bench/target/release/echo_bench
  DEPENDS echo1
)

add_dependencies(test-echo1 build-benchtools)
