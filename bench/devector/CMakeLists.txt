#
set(INCLUDE_FILES
	  ${CMAKE_SOURCE_DIR}/src/indivi/devector.h
	  benchmark_devector.h
)

set(SOURCE_FILES
    benchmark_main.cpp
)

add_executable(devector_benchmark
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

target_include_directories(devector_benchmark
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

#
target_link_libraries(devector_benchmark
    benchmark
    ${CMAKE_THREAD_LIBS_INIT}
)
