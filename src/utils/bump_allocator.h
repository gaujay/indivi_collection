/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 *
 */

#ifndef BUMP_ALLOCATOR_H
#define BUMP_ALLOCATOR_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>
#include <type_traits>


template <class T, uint16_t TPerChunk = 512>
class bump_allocator
{
public:
  using value_type = T;
  
  bump_allocator()
  {}
  
  bump_allocator(const bump_allocator& other) = delete;
  
  bump_allocator(bump_allocator&& other)
    : mPos(other.mPos)
    , mChunks(std::move(other.mChunks))
    , mAllocated(other.mAllocated)
  {
    other.mAllocated = 0;
  }
  
  ~bump_allocator()
  {
    assert(mAllocated == 0);
    for (T* chunk : mChunks)
      delete[] reinterpret_cast<storage_type*>(chunk);
  }
  
  bump_allocator& operator=(bump_allocator&& other) noexcept
  {
    for (T* chunk : mChunks)
      delete[] reinterpret_cast<storage_type*>(chunk);
    
    mPos = other.mPos;
    mChunks = std::move(other.mChunks);
    mAllocated = other.mAllocated;
    other.mAllocated = 0;
    
    return *this;
  }
  
  T* allocate(std::size_t n)
  {
    assert(n <= (std::size_t)TPerChunk);
    if (mChunks.empty() || needNewChunk((uint16_t)n))
    {
      mChunks.push_back(reinterpret_cast<T*>(new storage_type[TPerChunk]));
      mPos = mChunks.back();
    }
    
    T* ptr = mPos;
    mPos += n;
    mAllocated += n;
    return ptr;
  }
  
  void deallocate(T*, std::size_t n)
  {
    // Do nothing
    assert(mAllocated >= n);
    mAllocated -= n;
  }
  
private:
  using storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
  
  bool needNewChunk(uint16_t n) const
  {
    T* chunk = mChunks.back();
    uint16_t taken = (uint16_t)(mPos - chunk);
    return taken + n > TPerChunk;
  }
  
  T* mPos = nullptr;
  std::vector<T*> mChunks;
  uint64_t mAllocated = 0;   // debug
};


#endif // BUMP_ALLOCATOR_H
