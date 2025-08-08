#
set(INCLUDE_FILES
    benchmark_flat_map.h
)

set(SOURCE_FILES
    benchmark_main.cpp
)

add_executable(flat_unordered_benchmark
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

target_include_directories(flat_unordered_benchmark
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        # ${CMAKE_SOURCE_DIR}/lib/absl_20240116.2
        # ${CMAKE_SOURCE_DIR}/lib/boost_1_88_0
)

#
target_link_libraries(flat_unordered_benchmark
    benchmark
    ${CMAKE_THREAD_LIBS_INIT}
)

set_property(TARGET flat_unordered_benchmark PROPERTY CXX_STANDARD 14)
