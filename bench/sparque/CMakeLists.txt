#
set(INCLUDE_FILES
    benchmark_sparque.h
)

set(SOURCE_FILES
    benchmark_main.cpp
)

add_executable(sparque_benchmark
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

target_include_directories(sparque_benchmark
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib/seq_1.1.0
        ${CMAKE_SOURCE_DIR}/lib/segmented_tree
)

#
target_link_libraries(sparque_benchmark
    benchmark
    ${CMAKE_THREAD_LIBS_INIT}
)
