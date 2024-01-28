/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#ifndef GENERATORS_H
#define GENERATORS_H

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <cmath>
#include <cstdlib>
#include <ctime>

/****************************************************************************************************/

template <const size_t N>
struct Data8 {
  int8_t d[N];
  
  Data8() = default;
  Data8(unsigned int i) : d{(int8_t)i} {}
//Data8(const Data8& other) { for (size_t i=0; i<N; ++i) d[i] -= other.d[i]; }
//Data8(Data8&& other) { for (size_t i=0; i<N; ++i) d[i] += other.d[i]; }
//Data8& operator=(const Data8& other) = default;
  inline Data8& operator=(int val) { for (size_t i=0; i<N; ++i) d[i] = (int8_t)val; return *this; }
  inline Data8& operator++() { for (size_t i=0; i<N; ++i) ++(d[i]); return *this; }
};
template <const size_t N>
using Bytes = Data8<N>;

template <const size_t N>
struct Data16 {
  int16_t d[N];
};

template <const size_t N>
struct Data32 {
  int32_t d[N];
};

template <const size_t N>
struct Data64 {
  int64_t d[N];
};

/****************************************************************************************************/

#define CPLXSTRUCT_SIZE  200  // Bytes
struct CplxStruct {
  int8_t d[CPLXSTRUCT_SIZE];
  
  CplxStruct(unsigned int i)
    : d{(int8_t)i}
  {}
  
  ~CplxStruct()
  {}
  
  CplxStruct(const CplxStruct& other)
  {
    this->operator=(other);
  }
  
  CplxStruct(CplxStruct&& other)
    : CplxStruct((const CplxStruct&)other)
  {}
  
  inline CplxStruct& operator=(const CplxStruct& other)
  {
    assign(other);
    return *this;
  }
  
  inline CplxStruct& operator=(CplxStruct&& other)
  {
    assign(other);
    return *this;
  }
  
  void assign(const CplxStruct& other)
  {
    // Time consuming computation
    for (size_t i = 0; i < CPLXSTRUCT_SIZE; ++i)
    {
      if (i % 2)
        d[i] += other.d[i] * 5;
      else
        d[i] -= other.d[i] * 7;
    }
    for (int i = 0; i < CPLXSTRUCT_SIZE; ++i)
    {
      d[i] *= d[CPLXSTRUCT_SIZE - 1 - i];
      d[i] /= other.d[i] == 0 ? other.d[i] + 1 : other.d[i];
    }
    for (int i = CPLXSTRUCT_SIZE - 1; i >= 0; --i)
    {
      d[i] *= other.d[(i*5) % CPLXSTRUCT_SIZE];
      d[i] -= d[(i*10) % CPLXSTRUCT_SIZE];
    }
  }
};

/****************************************************************************************************/

template <typename T>
inline T* allocate(size_t N)
{
  return static_cast<T*>(malloc(sizeof(T)*N));
}

/****************************************************************************************************/

template <typename T>
inline T get_one()
{
  return T(); // default constructed
}

template <typename T>
inline T get_one(unsigned int)
{
  return T(); // default constructed
}

template <>
inline std::string get_one(unsigned int len)
{
  return std::string(len, (char)len);
}

/****************************************************************************************************/

unsigned int _get_one_inc = 0;

template <typename T>
inline T get_one_inc()
{
  return (T)(++_get_one_inc); // forced cast
}

template <typename T>
inline T get_one_inc(unsigned int)
{
  return (T)(++_get_one_inc); // forced cast
}

template <>
inline std::string get_one_inc(unsigned int len)
{
  return std::string(len, (unsigned char)((++_get_one_inc) % 256));
}

/****************************************************************************************************/

inline char get_rand_printable_char()
{
  return (char)(33 + 93.f*(std::rand()/(float)RAND_MAX));
}

template <typename T>
inline T get_rand_unit()
{
	return (std::rand()/(T)RAND_MAX);
}

template <typename T>
inline T get_rand(T min, T max)
{
	return (T)((std::rand()/(double)RAND_MAX)*(max-min) + min);
}

template <typename T>
inline T get_rand(unsigned int)
{
  return (T)(std::rand());  // forced cast
}

template <>
inline bool get_rand<bool>(unsigned int)
{
  return ((std::rand()/(float)RAND_MAX) >= 0.5f ? true : false);
}

template <>
inline char get_rand<char>(unsigned int)
{
  return (char)(-128 + 255.f*(std::rand()/(float)RAND_MAX));
}

template <>
inline std::string get_rand<std::string>(unsigned int len)
{
  return std::string(len, get_rand_printable_char());
}

/****************************************************************************************************/

#endif // GENERATORS_H
