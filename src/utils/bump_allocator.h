/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#ifndef BUMP_ALLOCATOR_H
#define BUMP_ALLOCATOR_H

#include <type_traits>
#include <utility>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>


template <class T, uint32_t TPerChunk = 256u>
class bump_allocator
{
public:
  using value_type = T;
  
  using propagate_on_container_swap = std::true_type;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  
  template<class U>
  struct rebind
  {
    typedef bump_allocator<U, TPerChunk> other;
  };
  
  bump_allocator() = default;
  
  bump_allocator(const bump_allocator& /*other*/) noexcept
  {}
  
  template<class U>
  bump_allocator(const bump_allocator<U, TPerChunk>& /*other*/) noexcept
  {}
  
  bump_allocator(bump_allocator&& other) noexcept
    : mPos(other.mPos)
    , mChunks(std::move(other.mChunks))
    , mAllocated(other.mAllocated)
  {
    other.mPos = nullptr;
    other.mChunks.clear();
    other.mAllocated = 0u;
  }
  
  ~bump_allocator()
  {
    assert(mAllocated == 0);
    for (T* chunk : mChunks)
      delete[] reinterpret_cast<storage_type*>(chunk);
  }
  
  bump_allocator& operator=(const bump_allocator& /*other*/) noexcept
  {
    return *this;
  }
  
  bump_allocator& operator=(bump_allocator&& other) noexcept
  {
    for (T* chunk : mChunks)
      delete[] reinterpret_cast<storage_type*>(chunk);
    
    mPos = other.mPos;
    mChunks = std::move(other.mChunks);
    mAllocated = other.mAllocated;
    
    other.mPos = nullptr;
    other.mAllocated = 0;
    
    return *this;
  }
  
  T* allocate(std::size_t n)
  {
    assert(n <= (std::size_t)TPerChunk);
    if (mChunks.empty() || needNewChunk((uint32_t)n))
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
  
  bool empty() const noexcept
  {
    return mAllocated == 0u;
  }
  
  friend bool operator!=(const bump_allocator& lhs, const bump_allocator& rhs) noexcept
  {
    return &lhs != &rhs;
  }
  friend bool operator==(const bump_allocator& lhs, const bump_allocator& rhs) noexcept
  {
    return &lhs == &rhs;
  }
  
private:
  using storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
  
  bool needNewChunk(uint32_t n) const
  {
    T* chunk = mChunks.back();
    uint32_t taken = (uint32_t)(mPos - chunk);
    return taken + n > TPerChunk;
  }
  
  T* mPos = nullptr;
  std::vector<T*> mChunks;
  uint64_t mAllocated = 0u; // debug
};


#endif // BUMP_ALLOCATOR_H
