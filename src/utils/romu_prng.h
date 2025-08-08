// Romu Pseudorandom Number Generators
//
// Copyright 2020 Mark A. Overton
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ------------------------------------------------------------------------------------------------
//
// Website: romu-random.org
// Paper:   http://arxiv.org/abs/2002.11331
//
// Copy and paste the generator you want from those below.
// To compile, you will need to #include <stdint.h> and use the ROTL definition below.

#include <random>
#include <cstdint>

#define ROTL(d,lrot) ((d<<(lrot)) | (d>>(8*sizeof(d)-(lrot))))

//===== RomuDuoJr ================================================================================
//
// The fastest generator using 64-bit arith., but not suited for huge jobs.
// Est. capacity = 2^51 bytes. Register pressure = 4. State size = 128 bits.

class RomuDuoJr
{
private:
  uint64_t xState, yState;  // set to nonzero seed

  static uint64_t splitMix64(uint64_t& state) noexcept
  {
    uint64_t z = (state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30U)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27U)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31U);
  }

public:
  RomuDuoJr() noexcept
    : xState(0)
    , yState(0)
  {
    std::random_device rnd;
    std::uniform_int_distribution<uint64_t> dist;
    do {
        xState = dist(rnd);
        yState = dist(rnd);
    } while (xState == 0u || yState == 0u);
  }

  RomuDuoJr(uint64_t seed) noexcept
    : xState(splitMix64(seed))
    , yState(splitMix64(seed))
  {}
  
  void reset(uint64_t seed) noexcept
  {
    xState = splitMix64(seed);
    yState = splitMix64(seed);
  }
  
  uint64_t operator()() noexcept
  {
     uint64_t xp = xState;
     xState = 15241094284759029579u * yState;
     yState = yState - xp;  yState = ROTL(yState,27);
     return xp;
  }
};
