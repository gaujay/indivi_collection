#
include(GoogleTest)

#
set(SOURCE_FILES_DEVECTOR
    test_devector_main.cpp
)

set(gtest_force_shared_crt
    ON CACHE BOOL "" FORCE
)

add_executable(devector_tests
    ${SOURCE_FILES_DEVECTOR}
)

target_include_directories(devector_tests 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib/benchmark/googletest/googletest/include
)

target_link_libraries(devector_tests 
    PUBLIC 
        gtest
        gtest_main
)

#
gtest_discover_tests(devector_tests)
