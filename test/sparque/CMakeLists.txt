#
include(GoogleTest)

#
set(SOURCE_FILES_SPARQUE
    test_sparque_main.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/debug_utils.cpp
)

set(gtest_force_shared_crt
    ON CACHE BOOL "" FORCE
)

add_executable(sparque_tests
    ${SOURCE_FILES_SPARQUE}
)

target_include_directories(sparque_tests
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib/benchmark/googletest/googletest/include
)

target_link_libraries(sparque_tests 
    PUBLIC 
        gtest
        gtest_main
)

#
gtest_discover_tests(sparque_tests)
