#
include(GoogleTest)

#
set(SOURCE_FILES_FLAT_UNORDERED
    test_flat_umap_main.cpp
    test_flat_uset_main.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/debug_utils.cpp
)

set(gtest_force_shared_crt
    ON CACHE BOOL "" FORCE
)

add_executable(flat_unordered_tests
    ${SOURCE_FILES_FLAT_UNORDERED}
)

target_include_directories(flat_unordered_tests 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib/benchmark/googletest/googletest/include
)

target_link_libraries(flat_unordered_tests 
    PUBLIC 
        gtest
        gtest_main
)

#
gtest_discover_tests(flat_unordered_tests)
