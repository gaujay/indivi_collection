/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 *
 */

// Benchmark
#include <benchmark/benchmark.h>

// Std
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <string>
#include <vector>
#include <deque>
#include <random>
#include <algorithm>

// Utils
#include "utils/generators.h"

// Src
#include "indivi/offset_vector.h"
using namespace  indivi;

// Constants
#ifndef INNER_LOOP
  #define INNER_LOOP 4
#endif
#ifndef SRAND_SEED
  #define SRAND_SEED 55187
#endif

#define DATA_LEN  31  // if applicable (e.g string length: avoid small string optimization)


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
void Assign_Fill_Reserved(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec;
      vec.reserve(range);
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
void Reserve_Double(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      auto it = vec.begin();
      state.ResumeTiming();
      
      vec.reserve(range*2);
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.capacity() != (size_t)range*2 || it == vec.begin())
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Shrink_Half(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      vec.reserve(range*2);
      state.ResumeTiming();
      
      vec.shrink_to_fit();
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.capacity() != (size_t)range)
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
void Resize_Double_Reserved(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      vec.reserve(range*2);
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
void ResizeVal_Double_Reserved(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      vec.reserve(range*2);
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
void Squash_Offset(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      vec.pop_front();
      state.ResumeTiming();
      
      vec.shift_data_start();
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range-1 || vec.offset() != 0)
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
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec;
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
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
void PushBack_Reserved(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec;
      vec.reserve(range);
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
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
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(vec.cbegin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
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
void Insert_Random_Reserved(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one_inc<typename V::value_type>(DATA_LEN));
      vec.reserve(range*2);
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(vec.cbegin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
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
void Insert_Random_Empty_Reserved(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec;
      vec.reserve(range);
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
void InsertN_Middle(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*2, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      vec.insert(vec.cbegin() + range, range,
                 get_one_inc<typename V::value_type>(DATA_LEN));
      
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*3)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}
//
template <class V>
void InsertN_Middle_Reserved(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*2, get_one<typename V::value_type>(DATA_LEN));
      vec.reserve(range*3);
      state.ResumeTiming();
      
      vec.insert(vec.cbegin() + range, range,
                 get_one_inc<typename V::value_type>(DATA_LEN));
      
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*3)
        std::cout << "Error" << std::endl;
    }
    state.ResumeTiming();
  }
}

//
template <class V>
void Push_Front(benchmark::State& state)
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
void Push_Front_Reserved(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      vec.reserve(range*2);
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
void Insert_Front(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(vec.cbegin(),
                   get_one_inc<typename V::value_type>(DATA_LEN));
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
void Insert_Front_Reserved(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      vec.reserve(range*2);
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.insert(vec.cbegin(),
                   get_one_inc<typename V::value_type>(DATA_LEN));
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
void EraseFront(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.erase(vec.begin());
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (!vec.empty())
        std::cout << "Error" << std::endl;
    }
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
      
      for (int64_t i=0; i<range; ++i) {
        vec.erase(vec.begin() + get_rand<size_t>(0, vec.size()-1));
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
void Erase_FirstHalf(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    for (size_t i=0; i<INNER_LOOP; ++i)
    {
      V vec(range*2, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();

      vec.erase(vec.begin(), vec.begin() + range);
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
void Erase_MiddleHalf(benchmark::State& state)
{
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*3, get_one<typename V::value_type>(DATA_LEN));
      state.ResumeTiming();

      vec.erase(vec.begin() + range, vec.begin() + range*2);
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
void EraseInsert_Combo(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  for (auto _ : state)
  {
    state.PauseTiming();
    {
      V vec(range*5, get_one<typename V::value_type>(DATA_LEN));
      vec.erase(vec.begin(), vec.begin() + range); // add room to work
      vec.erase(vec.end()-range, vec.end());
      state.ResumeTiming();
      
      for (int64_t i=0; i<range; ++i) {
        vec.erase( vec.begin() + get_rand<size_t>(0, vec.size()-1));
        vec.insert(vec.begin() + get_rand<size_t>(0, vec.size()),
                   get_one_inc<typename V::value_type>(DATA_LEN));
      }
      benchmark::DoNotOptimize(vec);
      
      state.PauseTiming();
      if (vec.size() != (size_t)range*3)
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
      vec.erase(vec.begin(), vec.begin() + range); // add room to work
      vec.erase(vec.end()-range, vec.end());
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
template <class V>
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
      data_helper<V, typename V::value_type>::fill_n(vec);
      state.ResumeTiming();
      
      for (int64_t i=0; i<range/1024; ++i) {
        found &= std::end(vec) != std::find(std::begin(vec), std::end(vec),
                                            data_helper<V, typename V::value_type>::rand_1(vec.size()-1));
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
template <class V>
void Increment_Each(benchmark::State& state)
{
  std::srand(SRAND_SEED);
  
  int64_t range = state.range(0);
  V vec(range);
  data_helper<V, typename V::value_type>::fill_n(vec);
  
  for (auto _ : state)
  {
    std::for_each(vec.begin(), vec.end(), [](typename V::value_type &v){
      data_helper<V, typename V::value_type>::inc(v);
    });
    benchmark::DoNotOptimize(vec);
  }
}

//////////////////////////////////////////////////////////////
//
#define MULT  (2)
#define RMIN  (1<<14)
#define RMAX  (1<<22)
////
//BENCHMARK_TEMPLATE(Construct_NDefault, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////BENCHMARK_TEMPLATE(Construct_NDefault, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NDefault, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NDefault, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NDefault, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NDefault, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Construct_NCopy, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NCopy, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NCopy, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NCopy, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NCopy, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Construct_NCopy, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Assign_Fill_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Assign_Fill_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Assign_Fill,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Assign_Fill_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Assign_Fill_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Assign_Fill,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Reserve_Double, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Reserve_Double, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Reserve_Double, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Reserve_Double, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Shrink_Half, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Shrink_Half, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Shrink_Half, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Shrink_Half, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Resize_Double_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////BENCHMARK_TEMPLATE(Resize_Double_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Resize_Double,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Resize_Double_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Resize_Double_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Resize_Double,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(ResizeVal_Double_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(ResizeVal_Double_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(ResizeVal_Double,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(ResizeVal_Double_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(ResizeVal_Double_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(ResizeVal_Double,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Squash_Offset, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Squash_Offset, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Clear, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Clear, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Clear, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(PushBack,          std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack,          offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack,          std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack,          offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PushBack,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/16)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          std::vector<Bytes<200>>   )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          offset_vector<Bytes<200>> )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty_Reserved, std::vector<Bytes<200>>   )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty_Reserved, offset_vector<Bytes<200>> )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Empty,          std::deque<Bytes<200>>    )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/128)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Insert_Random,          std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/32)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/32)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/32)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/32)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/16, RMAX/32)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          std::vector<Bytes<200>>   )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/256)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          offset_vector<Bytes<200>> )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/256)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Reserved, std::vector<Bytes<200>>   )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/256)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random_Reserved, offset_vector<Bytes<200>> )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/256)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Random,          std::deque<Bytes<200>>    )->RangeMultiplier(MULT)->Range(RMIN/128, RMAX/256)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(InsertN_Middle,          std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle,          offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle_Reserved, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle,          std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle,          std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle,          offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle_Reserved, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(InsertN_Middle,          std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Insert_Front_Reserved, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Push_Front_Reserved,   offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Push_Front,            std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Insert_Front_Reserved, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Push_Front_Reserved,   offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Push_Front,            std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(EraseFront, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PopFront,   offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseFront, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseFront, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(PopFront,   offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseFront, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
////
BENCHMARK_TEMPLATE(Erase_Random, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, std::vector<Bytes<200>>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, offset_vector<Bytes<200>> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(Erase_Random, std::deque<Bytes<200>>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Erase_FirstHalf, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
////BENCHMARK_TEMPLATE(Erase_FirstHalf, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMAX, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_FirstHalf, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_FirstHalf, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_FirstHalf, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_FirstHalf, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Erase_MiddleHalf, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMAX, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_MiddleHalf, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMAX, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_MiddleHalf, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMAX, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_MiddleHalf, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_MiddleHalf, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Erase_MiddleHalf, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(EraseInsert_Combo, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Combo, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Combo, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Combo, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Combo, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Combo, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(EraseInsert_Random, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/64)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, std::vector<Bytes<200>>   )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, offset_vector<Bytes<200>> )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(EraseInsert_Random, std::deque<Bytes<200>>    )->RangeMultiplier(MULT)->Range(RMIN/64, RMAX/128)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Find_Random, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Find_Random, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Find_Random, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Find_Random, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Find_Random, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Find_Random, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN/4, RMAX/4)->Unit(benchmark::kMicrosecond);
////
//BENCHMARK_TEMPLATE(Increment_Each, std::vector<int>          )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Increment_Each, offset_vector<int>        )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Increment_Each, std::deque<int>           )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Increment_Each, std::vector<std::string>  )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Increment_Each, offset_vector<std::string>)->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
//BENCHMARK_TEMPLATE(Increment_Each, std::deque<std::string>   )->RangeMultiplier(MULT)->Range(RMIN, RMAX)->Unit(benchmark::kMicrosecond);
