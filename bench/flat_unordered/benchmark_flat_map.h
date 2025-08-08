/**
 * Copyright 2025 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

// Benchmark
#include <benchmark/benchmark.h>

// Utils
#include "utils/romu_prng.h"

// Src
#include "indivi/flat_umap.h"
#include "indivi/flat_wmap.h"

// 3rd-parties
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wpedantic"
// #pragma GCC diagnostic ignored "-Woverflow"
// #include "absl/container/internal/raw_hash_set.cc"
// #include "absl/base/internal/raw_logging.cc"
// #include "absl/hash/internal/hash.cc"
// #include "absl/hash/internal/city.cc"
// #include "absl/hash/internal/low_level_hash.cc"
// #include "absl/container/flat_hash_map.h"
// #pragma GCC diagnostic pop

// #define BOOST_FORCEINLINE // disabled for fairness?
// #include "boost/unordered/unordered_flat_map.hpp"

// Std
#include <algorithm>
#include <array>
#include <iostream>
#include <random>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>

// Constants
#define APPROX_CPU_CACHE_SIZE 20971520u // for cache flushing (in bytes)

// Options
#ifndef INNER_MAPS
  // some benchmarks use multiple maps to avoid CPU caching bias and stabilize results
  // helps simulate "real world" use cases where cache is usually cold on first access
  #define INNER_MAPS 20
#endif
#ifndef INNER_LOOP
  #define INNER_LOOP 4
#endif
#ifndef SRAND_SEED
  #define SRAND_SEED 55187
#endif

// Hash functions
struct uint32_murmur
{
  using is_avalanching = std::true_type;
  
  std::size_t operator()(const uint32_t& key) const
  {
    // MurmurHash3’s 64-bit finalizer
    uint64_t result = key;
    result ^= result >> 33;
    result *= 0xff51afd7ed558ccdull;
    result ^= result >> 33;
    result *= 0xc4ceb9fe1a85ec53ull;
    result ^= result >> 33;
    return result;
  }
};

struct uint64_murmur
{
  using is_avalanching = std::true_type;
  
  std::size_t operator()(const uint64_t& key) const
  {
    // MurmurHash3’s 64-bit finalizer
    uint64_t result = key;
    result ^= result >> 33;
    result *= 0xff51afd7ed558ccdull;
    result ^= result >> 33;
    result *= 0xc4ceb9fe1a85ec53ull;
    result ^= result >> 33;
    return result;
  }
};


template <typename K>
static void shuffle(std::vector<K>& vec, uint64_t seed = SRAND_SEED)
{
  // std::shuffle is not cross platform
  // let's implement our own
  using std::swap;
  
  std::mt19937_64 mt(seed);
  for (int64_t i = (int64_t)vec.size() - 1; i >= 0; i--)
  {
    int j = (int)(mt() % (i + 1));
    swap(vec[i], vec[j]);
  }
}

static inline void flush_cache()
{
  static auto buffer = std::vector<uint64_t>(APPROX_CPU_CACHE_SIZE / sizeof(uint64_t));
  
  RomuDuoJr gen(std::time(0));
  uint64_t first = gen();
  first = first > 0 ? first : 1u;
  for (auto& val : buffer)
    val += first;
  
  if (buffer.back() + buffer.front() + buffer[buffer.size() / 2] == 0u)
    std::cout << "Error: flush_cache" << std::endl; // do not optimize
}

//
template <class M>
void Emplace_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  key_t key = 0;
  
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i = 0; i < INNER_LOOP; ++i)
    {
      M map;
      state.ResumeTiming();

      for (int64_t j = 0; j < range; ++j) {
        map.emplace(key, (val_t)key + 1);
        ++key;
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(map);
      
      if (map.size() != (size_t)range)
        std::cout << "Error: " << map.size() << " != " << range << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Emplace_Sequence_Reserve(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  key_t key = 0;
  
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i = 0; i < INNER_LOOP; ++i)
    {
      M map;
      map.reserve(range);
      state.ResumeTiming();
      
      for (int64_t j = 0; j < range; ++j) {
        map.emplace(key, (val_t)key + 1);
        ++key;
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(map);
      
      if (map.size() != (size_t)range)
        std::cout << "Error: " << map.size() << " != " << range << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Emplace_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  RomuDuoJr gen(SRAND_SEED);
  
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i = 0; i < INNER_LOOP; ++i)
    {
      M map;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < range; ++j) {
        key_t key = (key_t)gen();
        map.emplace(key, (val_t)key + 1);
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(map);
      
      if (map.size() != (size_t)range) // with good seed no collision
        std::cout << "Error: " << map.size() << " != " << range << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Emplace_Random_Reserve(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  RomuDuoJr gen(SRAND_SEED);
  
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i = 0; i < INNER_LOOP; ++i)
    {
      M map;
      map.reserve(range);
      state.ResumeTiming();
      
      for (int64_t j = 0; j < range; ++j) {
        key_t key = (key_t)gen();
        map.emplace(key, (val_t)key + 1);
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(map);
      
      if (map.size() != (size_t)range) // with good seed no collision
        std::cout << "Error: " << map.size() << " != " << range << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Find_Existing_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  std::vector<key_t> keys;
  keys.reserve(range);
  key_t key = 0;
  
  for (int64_t i = 0; i < range; ++i) {
    map0.emplace(key, (val_t)key + 1);
    keys.emplace_back(key);
    ++key;
  }
  
  std::array<M, INNER_MAPS> maps;
  for (auto& map : maps)
    map = map0;
  
  shuffle(keys);
  
  int64_t k = 0;
  int64_t sz = (int64_t)keys.size();
  for (auto _ : state)
  {
    state.PauseTiming();
    // int64_t k = 0;
    key = 0;
    flush_cache();
    
    for (const auto& map : maps)
    {
      uint64_t accu = 0u;
      state.ResumeTiming();
      
      // for (int64_t j = 0; j < count; ++j) {
      //   accu += map.find(key)->second;
      //   ++key;
      // }
      for (int64_t j = 0; j < count; ++j, ++k) {
        k = k < sz ? k : 0;
        accu += map.find(keys[k])->second;
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu == 0u)
        std::cout << "Error: " << accu << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Find_Existing_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  std::vector<key_t> keys;
  keys.reserve(range);
  RomuDuoJr gen(SRAND_SEED);
  
  while (map0.size() < (size_t)range) {
    key_t key = (key_t)gen();
    map0.emplace(key, (val_t)key + 1);
    keys.emplace_back(key);
  }
  
  std::array<M, INNER_MAPS> maps;
  for (auto& map : maps)
    map = map0;
  
  shuffle(keys);
  
  int64_t k = 0;
  int64_t sz = (int64_t)keys.size();
  for (auto _ : state)
  {
    state.PauseTiming();
    flush_cache();
    
    for (const auto& map : maps)
    {
      uint64_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j, ++k) {
        k = k < sz ? k : 0;
        accu += map.find(keys[k])->second;
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu == 0u)
        std::cout << "Error: " << accu << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Find_NonExisting_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  if (count > range)
    std::cout << "Error: count > map size " << std::endl;
  
  M map0;
  map0.reserve(range);
  key_t key = 0;
  for (int64_t i = 0; i < range; ++i) {
    map0.emplace(key, (val_t)key + 1);
    ++key;
  }
  
  std::array<M, INNER_MAPS> maps;
  for (auto& map : maps)
    map = map0;
  
  for (auto _ : state)
  {
    state.PauseTiming();
    flush_cache();
    
    for (const auto& map : maps)
    {
      uint64_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j) {
        auto it = map.find(key);
        accu += it == map.end() ? 0u : 1u;
        ++key;
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu != 0u)
        std::cout << "Error: " << accu << " in " << range << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Find_NonExisting_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  constexpr uint64_t Mask = 0x0000000001000000ull; // arbitrary (single bit should avoid bias)
  
  int64_t range = state.range(0);
  if (count > range)
    std::cout << "Error: count > map size " << std::endl;
  
  M map0;
  map0.reserve(range);
  RomuDuoJr gen(SRAND_SEED);
  while (map0.size() < (size_t)range) {
    key_t key = (key_t)(gen() & ~Mask); // force unset bit
    map0.emplace(key, (val_t)key + 1);
  }
  
  std::array<M, INNER_MAPS> maps;
  for (auto& map : maps)
    map = map0;
  
  for (auto _ : state)
  {
    state.PauseTiming();
    flush_cache();
    
    for (const auto& map : maps)
    {
      uint64_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j)
      {
        key_t key = (key_t)(gen() | Mask); // force set bit
        auto it = map.find(key);
        accu += it == map.end() ? 0u : 1u;
      }

      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu != 0u)
        std::cout << "Error: " << accu << " in " << range << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Replace_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  std::vector<key_t> keys;
  keys.reserve(range);
  key_t key = 0;
  
  for (int64_t i = 0; i < range; ++i) {
    map0.emplace(key, (val_t)key + 1);
    keys.emplace_back(key);
    ++key;
  }
  
  std::array<M, INNER_MAPS> maps;
  for (auto& map : maps)
    map = map0;
  
  shuffle(keys);
  
  int64_t k = 0;
  int64_t sz = (int64_t)keys.size();
  for (auto _ : state)
  {
    state.PauseTiming();
    flush_cache();
    
    for (auto& map : maps)
    {
      uint64_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j, ++k) {
        k = k < sz ? k : 0;
        accu += ++map[keys[k]];
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu == 0u)
        std::cout << "Error: " << accu << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Replace_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  std::vector<key_t> keys;
  keys.reserve(range);
  RomuDuoJr gen(SRAND_SEED);
  
  while (map0.size() < (size_t)range) {
    key_t key = (key_t)gen();
    map0.emplace(key, (val_t)key + 1);
    keys.emplace_back(key);
  }
  
  std::array<M, INNER_MAPS> maps;
  for (auto& map : maps)
    map = map0;
  
  shuffle(keys);
  
  int64_t k = 0;
  int64_t sz = (int64_t)keys.size();
  for (auto _ : state)
  {
    state.PauseTiming();
    flush_cache();
    
    for (auto& map : maps)
    {
      uint64_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j, ++k) {
        k = k < sz ? k : 0;
        accu += ++map[keys[k]];
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu == 0u)
        std::cout << "Error: " << accu << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Erase_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  std::vector<key_t> keys;
  keys.reserve(range);
  key_t key = 0;
  
  for (int64_t i = 0; i < range; ++i) {
    map0.emplace(key, (val_t)key + 1);
    keys.emplace_back(key);
    ++key;
  }
  
  shuffle(keys);
  
  std::array<M, INNER_MAPS> maps;
  int64_t sz = (int64_t)keys.size();
  for (auto _ : state)
  {
    state.PauseTiming();
    int64_t k = 0;
    for (auto& map : maps)
      map = map0;
    flush_cache();
    
    for (auto& map : maps)
    {
      std::size_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j, ++k) {
        k = k < sz ? k : 0;
        accu += map.erase(keys[k]);
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu != (std::size_t)count)
        std::cout << "Error: " << accu << " Vs " << count << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M, int count = 1000>
void Erase_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  std::vector<key_t> keys;
  keys.reserve(range);
  RomuDuoJr gen(SRAND_SEED);
  
  while (map0.size() < (size_t)range) {
    key_t key = (key_t)gen();
    map0.emplace(key, (val_t)key + 1);
    keys.emplace_back(key);
  }
  
  shuffle(keys);
  
  std::array<M, INNER_MAPS> maps;
  int64_t sz = (int64_t)keys.size();
  for (auto _ : state)
  {
    state.PauseTiming();
    int64_t k = 0;
    for (auto& map : maps)
      map = map0;
    flush_cache();
    
    for (auto& map : maps)
    {
      std::size_t accu = 0u;
      state.ResumeTiming();
      
      for (int64_t j = 0; j < count; ++j, ++k) {
        k = k < sz ? k : 0;
        accu += map.erase(keys[k]);
      }
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu != (std::size_t)count)
        std::cout << "Error: " << accu << " Vs " << count << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Iterate_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map;
  map.reserve(range);
  key_t key = 0;
  
  for (int64_t i = 0; i < range; ++i) {
    map.emplace(key, (val_t)key + 1);
    ++key;
  }
  
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i = 0; i < INNER_LOOP; ++i)
    {
      uint64_t accu = 0u;
      flush_cache();
      state.ResumeTiming();
      
      // for (auto it = map.cbegin(); it != map.cend(); ++it)
      //   accu += it->second;
      for (const auto& val : map)
        accu += val.second;
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu == 0u)
        std::cout << "Error: " << accu << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Iterate_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map;
  map.reserve(range);
  RomuDuoJr gen(SRAND_SEED);
  
  while (map.size() < (size_t)range) {
    key_t key = (key_t)gen();
    map.emplace(key, (val_t)key + 1);
  }
  
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i = 0; i < INNER_LOOP; ++i)
    {
      uint64_t accu = 0u;
      flush_cache();
      state.ResumeTiming();
      
      // for (auto it = map.cbegin(); it != map.cend(); ++it)
      //   accu += it->second;
      for (const auto& val : map)
        accu += val.second;
      
      state.PauseTiming();
      benchmark::DoNotOptimize(accu);
      
      if (accu == 0u)
        std::cout << "Error: " << accu << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Rehash_Sequence(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  key_t key = 0;
  
  for (int64_t i = 0; i < range; ++i) {
    map0.emplace(key, (val_t)key + 1);
    ++key;
  }
  
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      M map = map0;
      state.ResumeTiming();
      
      map.rehash((std::size_t)(map.size() * 3.0));
      
      state.PauseTiming();
      benchmark::DoNotOptimize(map);
      
      if (map.size() != map0.size())
        std::cout << "Error: " << map.size() << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class M>
void Rehash_Random(benchmark::State& state)
{
  using key_t = typename M::key_type;
  using val_t = typename M::mapped_type;
  
  int64_t range = state.range(0);
  
  M map0;
  map0.reserve(range);
  RomuDuoJr gen(SRAND_SEED);
  
  while (map0.size() < (size_t)range) {
    key_t key = (key_t)gen();
    map0.emplace(key, (val_t)key + 1);
  }
  
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      M map = map0;
      state.ResumeTiming();
      
      map.rehash((std::size_t)(map.size() * 3.0));
      
      state.PauseTiming();
      benchmark::DoNotOptimize(map);
      
      if (map.size() != map0.size())
        std::cout << "Error: " << map.size() << std::endl;
    }
    state.ResumeTiming();
  }
}

//
void Warm_Up(benchmark::State& state)
{
  Emplace_Sequence<indivi::flat_wmap<uint32_t, uint32_t, uint32_murmur>>(state); // arbitrary
}

//////////////////////////////////////////////////////////////
//
#define MULT  (3)
#define RMIN  (13100)   // max load factor (pow2 * 0.8)
#define RMAX  (3355000)
//
// warmup
BENCHMARK(Warm_Up)->RangeMultiplier(2)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
//
// BENCHMARK_TEMPLATE(Emplace_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Sequence_Reserve, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Emplace_Random_Reserve, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Find_Existing_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Find_Existing_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(Find_Existing_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Find_Existing_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Find_Existing_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Existing_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(Find_NonExisting_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Find_NonExisting_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Find_NonExisting_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_NonExisting_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Replace_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Replace_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Replace_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Replace_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Replace_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Erase_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Erase_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Erase_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Erase_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Iterate_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Iterate_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Iterate_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Iterate_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Iterate_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Rehash_Sequence, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Sequence, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Sequence, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Sequence, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Rehash_Sequence, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Sequence, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Sequence, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Sequence, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Rehash_Random, indivi::flat_umap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Random, indivi::flat_wmap<uint64_t, uint64_t>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Random, boost::unordered_flat_map<uint64_t, uint64_t> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Random, absl::flat_hash_map<uint64_t, uint64_t>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);

// BENCHMARK_TEMPLATE(Rehash_Random, indivi::flat_umap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Random, indivi::flat_wmap<uint64_t, uint64_t, uint64_murmur>         )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Random, boost::unordered_flat_map<uint64_t, uint64_t, uint64_murmur> )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Rehash_Random, absl::flat_hash_map<uint64_t, uint64_t, uint64_murmur>       )->RangeMultiplier(MULT)->Range(RMIN/1, RMAX/1)->Unit(benchmark::kMicrosecond);
