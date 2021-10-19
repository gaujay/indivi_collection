#
include(GoogleTest)

#
set(SOURCE_FILES_OFFSETVECTOR
    test_offset_vector_main.cpp
)

set(gtest_force_shared_crt
    ON CACHE BOOL "" FORCE
)

add_executable(offset_vector_tests
    ${SOURCE_FILES_OFFSETVECTOR}
)

target_include_directories(offset_vector_tests 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib/benchmark/googletest/googletest/include
)

target_link_libraries(offset_vector_tests 
    PUBLIC 
        gtest
        gtest_main
)

#
gtest_discover_tests(offset_vector_tests)
