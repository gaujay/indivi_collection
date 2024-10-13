# Indivi-Collection

A collection of std-like containers written in C++11.

Includes Google Benchmark and Google Test support.

### Categories

- `flat_umap`/`flat_uset` (flat unordered map/set)
    - an associative container that stores unordered unique key-value pairs/keys
    - similar to `std::unordered_map`/`std::unordered_set`
    - open-addressing schema, with a dynamically allocated, consolidated array of values and metadata (capacity grows based on power of 2)
    - each entry uses 2 additional bytes of metadata (to store overflow counters and distances from original buckets)
    - optimized for small sizes (starting at 2, container sizeof is 40 Bytes on 64-bits)
    - avoid the need for tombstone mechanism or rehashing on iterator erase (with a good hash function)
    - group buckets to rely on SIMD operations for speed (SSE2 or NEON are mandatory)
    - come with an optimized 64-bits hash function (based on [wyhash](https://github.com/wangyi-fudan/wyhash))
    - search, insertion, and removal of elements have average constant time 𝓞(1) complexity

- `sparque` (sparse deque)
	- a sequence, non-contiguous and reversible container that allows fast random insertion and deletion (with basic exception safety)
	- dynamically allocated and automatically adjusted storage (allocator-aware, space complexity 𝓞(n))
	- similar to `std::deque`, but based on a counted B+ tree where each memory chunk behave as a double-ended vector.
	- options (see 'sparque.h' for more details):
		- chunk size (default: max(4, 1024 / sizeof(T)))
		- node size (default: 16)
	- complexity:
		- random access - 𝓞(log_b(n)), where b is the number of children per node
		- insertion or removal of elements at start/end - constant 𝓞(1)
		- insertion or removal of elements - amortized 𝓞(m), where m is the number of elements per chunk
		- iteration - contant 𝓞(n)

- `devector` (double-ended vector)
	- a sequence, contiguous and reversible container (with basic exception safety)
	- dynamically allocated and automatically handled storage (supports allocator, space complexity 𝓞(n))
	- similar to `std::vector` but with an additional 'offset', allowing front data manipulation
		- example representation:  |\_|a|b|\_|\_|  (with size=2, capacity=5, offset=1)
	- options (see 'devector.h' for more details):
		- reallocation position mode (start, center, end)
		- data shift mode (near, center, far)
		- growth factor
	- complexity:
		- random access - constant 𝓞(1)
		- remove at start/end - constant 𝓞(1)
		- insert at start/end - amortized constant 𝓞(1), or 𝓞(N) if size < capacity and start == startOfStorage/end == endOfStorage
		- insert/remove - linear in the distance to the closest between start and end 𝓞(N/2)

### Benchmark results

See corresponding 'bench' sub-folders for graphs.

### Dependencies

This project uses git submodule to include Google Benchmark and Google Test repositories:

    $ git clone https://github.com/gaujay/indivi_collection.git
    $ cd indivi_collection
    $ git submodule init && git submodule update
    $ git clone https://github.com/google/googletest lib/benchmark/googletest

### Building

Support GCC/MinGW, Clang and MSVC (see 'CMakeLists.txt').

You can open 'CMakeLists.txt' with a compatible IDE or use command line:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make <target> -j

### License

Apache License 2.0

Benchmarked third-party libraries:
- [seq::tiered_vector](https://github.com/Thermadiag/seq): MIT License
- [segmented_tree](https://github.com/det/segmented_tree): Boost Software License - Version 1.0
