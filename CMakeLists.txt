cmake_minimum_required (VERSION 3.0)

project (indivi_collection LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_options(/W4)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG /GL")
else()
  add_compile_options(-Wall -Wextra -pedantic)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG -march=native -mtune=native")
endif()

set(BENCHMARK_ENABLE_TESTING 
    OFF 
    CACHE BOOL "Disable benchmark testing" FORCE
)

#
add_subdirectory(bench)
add_subdirectory(test)

add_subdirectory(lib/benchmark)
add_subdirectory(lib/benchmark/googletest)
