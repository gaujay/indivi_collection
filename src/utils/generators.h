/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 *
 */

#ifndef GENERATORS_H
#define GENERATORS_H

#include <cstdlib>
#include <ctime>
#include <cmath>

#include <vector>
#include <string>
#include <numeric>
#include <iostream>
#include <algorithm>

/****************************************************************************************************/

template <const size_t N>
struct Data8 {
  int8_t d[N];
  
  Data8(unsigned int i) : d{(int8_t)i} {}
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
	return (T)((std::rand()/(double)RAND_MAX)*(max-min) - min);
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
