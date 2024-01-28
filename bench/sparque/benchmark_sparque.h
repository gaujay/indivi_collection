/**
 * Copyright 2023 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

// Benchmark
#include <benchmark/benchmark.h>

// Std
#include <algorithm>
#include <deque>
#include <string>

#include <cassert>
#include <cstdint>
#include <cstdlib>

// Utils
#include "utils/generators.h"

// Src
#include "indivi/sparque.h"
using namespace  indivi;

// 3rd-parties
#include <seq.hpp>
template <class T>
using seg_tree = boost::segmented_tree::seq<T>;

#include <tiered_vector.hpp>
template <class T>
using tiered_vec = seq::tiered_vector<T>;

// Constants
#ifndef INNER_LOOP
  #define INNER_LOOP 4
#endif
#ifndef SRAND_SEED
  #define SRAND_SEED 55187
#endif

#define DATA_LEN  31  // if applicable (e.g string length: avoid small string optimization)
#define TIME_ITER_CREATION  1 // mesure iterator creation time (for random insert/erase)

//
template <class V>
void Construct_NDefault(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      state.ResumeTiming();
      
      V vec(range);
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming(); // Don't time destruction
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Construct_NCopy(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      state.ResumeTiming();
      
      V vec(range, get_rand<typename V::value_type>(DATA_LEN));
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Assign_Fill(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec;
      state.ResumeTiming();
      
      vec.assign(range, get_rand<typename V::value_type>(DATA_LEN));
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Resize_Half(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      vec.resize(range/2);
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range/2)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Resize_Double(benchmark::State& state)
{
 int64_t range = state.range(0);
 for (auto _ : state)
 {
   state.PauseTiming();
   for (size_t i=0; i<INNER_LOOP; ++i)
   {
     V vec(range, get_one<typename V::value_type>(DATA_LEN));
     state.ResumeTiming();
      
     vec.resize(range*2);
     benchmark::DoNotOptimize(vec);
      
     state.PauseTiming();
     if (vec.size() != (size_t)range*2)
       std::cout << "Error" << std::endl;
   }
   state.ResumeTiming();
 }
}

//
template <class V>
void ResizeVal_Double(benchmark::State& state)
{
 int64_t range = state.range(0);
 for (auto _ : state)
 {
   state.PauseTiming();
   for (size_t i=0; i<INNER_LOOP; ++i)
   {
     V vec(range, get_one<typename V::value_type>(DATA_LEN));
     state.ResumeTiming();
      
     vec.resize(range*2, get_rand<typename V::value_type>(DATA_LEN));
     benchmark::DoNotOptimize(vec);
      
     state.PauseTiming();
     if (vec.size() != (size_t)range*2)
       std::cout << "Error" << std::endl;
   }
   state.ResumeTiming();
 }
}

//
template <class V>
void Clear(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      vec.clear();
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (!vec.empty())
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void PushBack(benchmark::State& state)
{
//  bool done = false;
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    // for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec;
      state.ResumeTiming();
      
      for (int64_t j=0; j<range; ++j) {
        vec.push_back(get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void Insert(benchmark::State& state)
{
//  bool done = false;
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec;
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void PopBack(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.pop_back();
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (!vec.empty())
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Insert_Random_Empty(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec;
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(vec.cbegin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void Insert_Random_Empty2(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec;
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(vec.nth(get_rand<size_t>(0, vec.size())),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Insert_Random(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one_inc<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
      #if !TIME_ITER_CREATION
        state.PauseTiming();
        auto it = vec.begin() + get_rand<size_t>(0, vec.size());
        state.ResumeTiming();
        vec.insert(it, get_one_inc<typename V::value_type>(DATA_LEN));
      #else
        vec.insert(vec.begin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      #endif
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*2)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void Insert_Random2(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one_inc<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
      #if !TIME_ITER_CREATION
        state.PauseTiming();
        auto it = vec.nth( get_rand<size_t>(0, vec.size() ));
        state.ResumeTiming();
        vec.insert(it, get_one_inc<typename V::value_type>(DATA_LEN));
      #else
        vec.insert(vec.nth( get_rand<size_t>(0, vec.size() )),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      #endif
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*2)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void PushFront(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.push_front(get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*2)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void PopFront(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.pop_front();
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (!vec.empty())
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Erase_Random(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
      #if !TIME_ITER_CREATION
        state.PauseTiming();
        auto it = vec.begin() + get_rand<size_t>(0, vec.size()-1);
        state.ResumeTiming();
        vec.erase(it);
      #else
        vec.erase(vec.begin() + get_rand<size_t>(0, vec.size()-1));
      #endif
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*4)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void Erase_Random2(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
      #if !TIME_ITER_CREATION
        state.PauseTiming();
        auto it = vec.nth( get_rand<size_t>(0, vec.size()-1) );
        state.ResumeTiming();
        vec.erase(it);
      #else
        vec.erase(vec.nth( get_rand<size_t>(0, vec.size()-1) ));
      #endif
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*4)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Erase_Center(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
      auto it = vec.begin() + (vec.size() / 2 - range / 2);
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
        it = vec.erase(it);
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*4)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void EraseInsert_Combo(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*5, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.erase( vec.begin() + get_rand<size_t>(0, vec.size()-1));
        vec.insert(vec.begin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*5)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void EraseInsert_Combo2(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*5, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.erase( vec.nth(get_rand<size_t>(0, vec.size()-1)));
        vec.insert(vec.nth(get_rand<size_t>(0, vec.size())),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*5)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}


//
template <class V>
void EraseInsert_Random(benchmark::State& state)
{
  int64_t delta = 0;
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      std::srand(SRAND_SEED);
      V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
        float rd = get_rand_unit<float>();
        float limit = delta == 0 ? 0.5f : (delta > 0 ? 0.6f : 0.4f); // 1.5x the chances
        bool erase = rd < limit ? true : false;
        
        if (erase) {
          vec.erase( vec.begin() + get_rand<size_t>(0, vec.size()-1));
          --delta;
        }
        else {
          vec.insert(vec.begin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
          ++delta;
        }
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void EraseInsert_Random2(benchmark::State& state)
{
  int64_t delta = 0;
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      std::srand(SRAND_SEED);
      V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i)
      {
        float rd = get_rand_unit<float>();
        float limit = delta == 0 ? 0.5f : (delta > 0 ? 0.6f : 0.4f); // 1.5x the chances
        bool erase = rd < limit ? true : false;
        
        if (erase) {
          vec.erase( vec.nth(get_rand<size_t>(0, vec.size()-1)));
          --delta;
        }
        else {
          vec.insert(vec.nth(get_rand<size_t>(0, vec.size())),
                     get_one_inc<typename V::value_type>(DATA_LEN));
          ++delta;
        }
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}

//
template <class V, typename T>
struct data_helper {
  static void fill_n(V& vec) {
    std::iota(vec.begin(), vec.end(), 0);
  }
  static T rand_1(size_t max) {
    return get_rand<T>((T)0, (T)max);
  }
  static void inc(T& val) {
    ++val;
  }
};
template <class V>
struct data_helper<V, std::string> {
  static void fill_n(V& vec) {
    size_t n = 0;
    std::generate(vec.begin(), vec.end(), [&n] () { return std::to_string(n++); });
  }
  static std::string rand_1(size_t max) {
    return std::to_string(get_rand<size_t>(0, max));
  }
  static void inc(std::string& val) {
    ++val[0];
  }
};

//
template <typename T, class V>
struct container_helper {
  static size_t sparse(V& vec, size_t size, uint32_t percent)
  {
    if (percent == 0)
      return size;
    uint16_t N = 16 * sizeof(T) >= 4096u ? 16u : 4096u / sizeof(T);
    size_t full_chunks = size / N;
    size_t erased = (full_chunks * percent) / 100;
    for (size_t i = 0; i < erased; ++i)
      vec.pop_back();
    return size - erased;
  }
};
template <typename T>
struct container_helper<T, sparque<T>> {
  static size_t sparse(sparque<T>& /*vec*/, size_t size, uint32_t percent)
  {
    assert(percent == 0);
    (void)percent;
    return size;
  }
};

//
template <class V, uint32_t sparsePercent = 0u>
void Find_Random(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      bool found = true;
      V vec(range);
      auto size = container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
      data_helper<V, typename V::value_type>::fill_n(vec);
      state.ResumeTiming();
      
      for (int64_t i=0; i<range/1024; ++i) {
        found &= std::end(vec) != std::find(std::begin(vec), std::end(vec),
                                            data_helper<V, typename V::value_type>::rand_1(size-1));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (!found)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V, uint32_t sparsePercent = 0u>
void Accumulate_Each(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  data_helper<V, typename V::value_type>::fill_n(vec);
  container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
  
  for (auto _ : state)
  {
    int64_t sum = 0;
    for (const auto& v : vec)
      sum += v;
    
    benchmark::DoNotOptimize(sum);
    if (sum == 0)
      std::cout << "error: 0";
  }
}
//
template <class V, uint32_t sparsePercent = 0u>
void Accumulate_Each_Reverse(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  data_helper<V, typename V::value_type>::fill_n(vec);
  container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
  
  for (auto _ : state)
  {
    int64_t sum = 0;
    auto cbegin = vec.cbegin();
    for (auto it = vec.cend(); it != cbegin;)
      sum += *(--it);
    
    benchmark::DoNotOptimize(sum);
    if (sum == 0)
      std::cout << "error: 0";
  }
}
//
template <class V, uint32_t sparsePercent = 0u>
void Accumulate_Each_Subscript(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  data_helper<V, typename V::value_type>::fill_n(vec);
  container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
  
  for (auto _ : state)
  {
    int64_t sum = 0;
    for (size_t i = 0; i < vec.size(); ++i)
      sum += vec[i];

    benchmark::DoNotOptimize(sum);
    if (sum == 0)
      std::cout << "error: 0";
  }
}

//
template <class V, uint32_t sparsePercent = 0u>
void Increment_Each(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  data_helper<V, typename V::value_type>::fill_n(vec);
  container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
  
  for (auto _ : state)
  {
    std::for_each(vec.begin(), vec.end(), [](typename V::value_type &v){
      data_helper<V, typename V::value_type>::inc(v);
    });
    benchmark::DoNotOptimize(vec);
  }
}
//
template <class V, uint32_t sparsePercent = 0u>
void Increment_Each_Subscript(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  data_helper<V, typename V::value_type>::fill_n(vec);
  container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
  
  for (auto _ : state)
  {
    const size_t sz = vec.size();
    for (std::size_t i = 0 ; i < sz; ++i)
      data_helper<V, typename V::value_type>::inc(vec[i]);
    benchmark::DoNotOptimize(vec);
  }
}

//
template <class V, uint32_t sparsePercent = 0u>
void Random_Accumulate(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
  container_helper<typename V::value_type, V>::sparse(vec, range*5, sparsePercent);
  
  for (auto _ : state)
  {
    int64_t sum = 0;
    for (int64_t i=0; i<range; ++i) {
      sum += vec[get_rand<size_t>(0, vec.size()-1)];
      
      benchmark::DoNotOptimize(sum);
      if (sum == 0)
        std::cout << "error: 0";
    }
    benchmark::DoNotOptimize(vec);
  }
}

//
template <class V, uint32_t sparsePercent = 0u>
void Random_Increment(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range*5, get_one_inc<typename V::value_type>(DATA_LEN));
  container_helper<typename V::value_type, V>::sparse(vec, range*5, sparsePercent);
  
  for (auto _ : state)
  {
    for (int64_t i=0; i<range; ++i) {
      data_helper<V, typename V::value_type>::inc( vec[get_rand<size_t>(0, vec.size()-1)] );
    }
    benchmark::DoNotOptimize(vec);
  }
}

//
template <class V, uint32_t sparsePercent = 0u>
void Sort_All(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  container_helper<typename V::value_type, V>::sparse(vec, range, sparsePercent);
  
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      for (auto& val : vec)
        val = get_rand<typename V::value_type>(DATA_LEN);
      state.ResumeTiming();
      
      std::sort(vec.begin(), vec.end());
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec[0] > vec[1])
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//////////////////////////////////////////////////////////////
//
#define MULT  (2)
#define RMIN  (1<<14)
#define RMAX  (1<<22)
// //
// BENCHMARK_TEMPLATE(Construct_NDefault, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NDefault, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Construct_NCopy, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Construct_NCopy, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Assign_Fill, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Assign_Fill, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Resize_Half, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Half, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Resize_Double, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Resize_Double, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(ResizeVal_Double, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(ResizeVal_Double, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Clear, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN*8, RMAX*8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN*8, RMAX*8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN*8, RMAX*8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN*8, RMAX*8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Clear, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(PushBack, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushBack, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(PopBack, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopBack, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(PushFront, tiered_vec<int>           )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, seg_tree<int>             )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, sparque<int>              )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, tiered_vec<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, seg_tree<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, sparque<std::string>      )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PushFront, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(PopFront, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(PopFront, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Insert_Random_Empty,  tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty2, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty2, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty,  std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty,  tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty2, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty2, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random_Empty,  std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Insert_Random,  tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random2, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random2, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random,  std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random,  tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random2, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random2, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Insert_Random,  std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// //
BENCHMARK_TEMPLATE(Erase_Random,  tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random2, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random2, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random,  std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random,  tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random2, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random2, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random,  std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Erase_Center, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Erase_Center, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/32, RMAX/64)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(EraseInsert_Combo,  tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo2, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo2, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo,  std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo,  tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo2, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo2, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Combo,  std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// // //
// BENCHMARK_TEMPLATE(EraseInsert_Random,  tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random2, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random2, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random,  std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random,  tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random2, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random2, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(EraseInsert_Random,  std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Find_Random, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Find_Random, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/2, RMAX/2)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Accumulate_Each, tiered_vec<int>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each, seg_tree<int>    )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each, sparque<int>     )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each, std::deque<int>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Accumulate_Each_Reverse, tiered_vec<int>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each_Reverse, seg_tree<int>    )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each_Reverse, sparque<int>     )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each_Reverse, std::deque<int>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Accumulate_Each_Subscript, tiered_vec<int>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each_Subscript, seg_tree<int>    )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each_Subscript, sparque<int>     )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Accumulate_Each_Subscript, std::deque<int>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Increment_Each, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Increment_Each_Subscript, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Random_Accumulate, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Accumulate, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Accumulate, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Accumulate, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Random_Increment, tiered_vec<int>          )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, seg_tree<int>            )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, sparque<int>             )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, std::deque<int>          )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, tiered_vec<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, seg_tree<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, sparque<std::string>     )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Random_Increment, std::deque<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/8, RMAX/8)->Unit(benchmark::kMicrosecond);
// //
// BENCHMARK_TEMPLATE(Sort_All, tiered_vec<int>         )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, seg_tree<int>           )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, sparque<int>            )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, std::deque<int>         )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, tiered_vec<std::string> )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, seg_tree<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, sparque<std::string>    )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(Sort_All, std::deque<std::string> )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
