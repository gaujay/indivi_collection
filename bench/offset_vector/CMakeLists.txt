#
set(INCLUDE_FILES
	  ${CMAKE_SOURCE_DIR}/src/indivi/offset_vector.h
	  benchmark_offset_vector.h
)

set(SOURCE_FILES
    benchmark_main.cpp
)

add_executable(offset_vector_benchmark
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

target_include_directories(offset_vector_benchmark
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

#
target_link_libraries(offset_vector_benchmark
    benchmark
    ${CMAKE_THREAD_LIBS_INIT}
)
