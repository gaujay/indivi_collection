/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 *
 */

#ifndef INDIVI_OFFSET_VECTOR_H
#define INDIVI_OFFSET_VECTOR_H

#include <cmath>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <limits>
#include <memory>
#include <utility>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

#define INDIVI_OV_GROWTH_F  2.f   // Capacity growth factor (must be > 1.f)
#define INDIVI_OV_DEFAULT_INIT    // Default-construct values, instead of value-construct (avoid memory value-initialization)
#ifdef INDIVI_OV_DEFAULT_INIT
  #define INDIVI_OV_VALUE_INIT value_type
#else
  #define INDIVI_OV_VALUE_INIT value_type()
#endif
#define INDIVI_OV_SHIFT_EMPTY     // Shift offset according to realloc-mode when vector becomes empty
//#define INDIVI_OV_DEBUG
static_assert(INDIVI_OV_GROWTH_F > 1.f, "INDIVI_OV_GROWTH_F must be > 1.f");

namespace indivi
{
// ShiftMode (only apply to 'push/emplace back/front')
// 'insert' (and 'erase') always use NEAR to minimize shifts (e.g. |_|a|b|c|_| -> |_|a|b|d|c| or |a|d|b|c|_|)
enum class ShiftMode : char {
  NEAR,     // shift data to their closest neighbor   (e.g. push_back: |_|_|_|a| -> |_|_|a|b|)
  CENTER,   // shift data to the center of storage    (e.g. push_back: |_|_|_|a| -> |_|a|b|_|)
  FAR,      // shift data to opposite side of storage (e.g. push_back: |_|_|_|a| -> |a|b|_|_|)
};

enum class ReallocMode : char {
  START,    // reallocate data to the start of storage  (e.g. push_back: |a| -> |a|b|_|_|)
  CENTER,   // reallocate data to the center of storage (e.g. push_back: |a| -> |_|a|b|_|)
  END,      // reallocate data to the end of storage    (e.g. push_back: |a| -> |_|_|a|b|)
};


template <class T,
          ShiftMode SHIFT_MODE = ShiftMode::NEAR,
          ReallocMode REALLOC_MODE = ReallocMode::START>
class offset_vector
{
  // Members
  T*  m_begin;
  T*  m_offset;
  T*  m_end;
  T*  m_end_of_storage;
  
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  
  using storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
  
  //
  // Constructor/Destructor
  //
  offset_vector()
    : m_begin(nullptr)
    , m_offset(nullptr)
    , m_end(nullptr)
    , m_end_of_storage(nullptr)
  {}
  
  offset_vector(size_type count, const T& value)
  {
    auto new_storage = std::unique_ptr<storage_type[]>(new storage_type[count]);
    auto new_begin = reinterpret_cast<T*>(new_storage.get());
    std::uninitialized_fill_n(new_begin, count, value);
    
    m_begin = new_begin;
    m_offset = m_begin;
    m_end = m_begin + count;
    m_end_of_storage = m_end;
    new_storage.release();
  }
  
  explicit offset_vector(size_type count)
  {
    auto new_storage = std::unique_ptr<storage_type[]>(new storage_type[count]);
    auto new_begin = reinterpret_cast<T*>(new_storage.get());
    auto new_end = new_begin + count;
    
    iterator it = new_begin;
    try {
      for (; it != new_end; ++it)
        ::new (static_cast<void*>(it)) INDIVI_OV_VALUE_INIT;
    }
    catch (...) {
      destroy_range(new_begin, it);
      throw;
    }
    
    m_begin = new_begin;
    m_offset = m_begin;
    m_end = new_end;
    m_end_of_storage = m_end;
    new_storage.release();
  }
  
  offset_vector(const offset_vector& other)
  {
    size_type size = other.size();
    
    auto new_storage = std::unique_ptr<storage_type[]>(new storage_type[size]);
    auto new_begin = reinterpret_cast<T*>(new_storage.get());
    std::uninitialized_copy(other.begin(), other.end(), new_begin);
    
    m_begin = new_begin;
    m_offset = m_begin;
    m_end = m_begin + size;
    m_end_of_storage = m_end;
    new_storage.release();
  }
  
  offset_vector(offset_vector&& other)
  {
    m_begin = other.m_begin;
    m_offset = other.m_offset;
    m_end = other.m_end;
    m_end_of_storage = other.m_end_of_storage;
    
    other.m_begin = nullptr;
    other.m_offset = nullptr;
    other.m_end = nullptr;
    other.m_end_of_storage = nullptr;
  }
  
  offset_vector(std::initializer_list<T> init)
  {
    size_type size = init.size();
    
    auto new_storage = std::unique_ptr<storage_type[]>(new storage_type[size]);
    auto new_begin = reinterpret_cast<T*>(new_storage.get());
    std::uninitialized_copy(init.begin(), init.end(), new_begin);
    
    m_begin = new_begin;
    m_offset = m_begin;
    m_end = m_begin + size;
    m_end_of_storage = m_end;
    new_storage.release();
  }

  ~offset_vector()
  {
    destroy_range(m_offset, m_end);
    delete[] reinterpret_cast<storage_type*>(m_begin);
  }
  
  //
  // Assignment
  //
  offset_vector& operator=(const offset_vector& other)
  {
    if (this != &other)
    {
      size_type otherSize = other.size();
      size_type capacity_ = capacity();
      if (otherSize <= capacity_)
      {
        auto new_offset = m_begin + realloc_offset(otherSize, capacity_);
        
        // Before new offset
        m_offset = destroy_range(m_offset, new_offset);
        
        // Before old offset
        size_type copy_n = std::min<size_type>(m_offset - new_offset, otherSize);
        iterator ot = std::uninitialized_copy_n(other.begin(), copy_n, new_offset);
        m_offset = new_offset;
        
        // Common part
        const_iterator it = other.begin() + copy_n;
        size_type assign_n = std::min<size_type>(other.end() - it, m_end - ot);
        ot = std::copy_n(it, assign_n, ot);
        
        // Other remaining
        ot = std::uninitialized_copy(it + assign_n, other.end(), ot);
        
        // This overflow
        destroy_range_backward(ot, m_end);
        m_end = ot;
      }
      else
      {
        clear();
        reserve_without_offset(otherSize);
        std::uninitialized_copy(other.begin(), other.end(), m_begin);
        m_end += otherSize;
      }
    }
    return *this;
  }
  
  offset_vector& operator=(offset_vector&& other)
  {
    destroy_range(m_offset, m_end);
    delete[] reinterpret_cast<storage_type*>(m_begin);
    
    m_begin = other.m_begin;
    m_offset = other.m_offset;
    m_end = other.m_end;
    m_end_of_storage = other.m_end_of_storage;
    
    other.m_begin = nullptr;
    other.m_offset = nullptr;
    other.m_end = nullptr;
    other.m_end_of_storage = nullptr;
    
    return *this;
  }
  
  offset_vector& operator=(std::initializer_list<T> ilist)
  {
    size_type listSize = ilist.size();
    size_type capacity_ = capacity();
    if (listSize <= capacity_)
    {
      auto new_offset = m_begin + realloc_offset(listSize, capacity_);
      
      // Before new offset
      m_offset = destroy_range(m_offset, new_offset);
      
      // Before old offset
      size_type copy_n = std::min<size_type>(m_offset - new_offset, listSize);
      std::uninitialized_copy_n(ilist.begin(), copy_n, new_offset);
      m_offset = new_offset;
      
      // Common part
      const_iterator it = ilist.begin() + copy_n;
      iterator ot = new_offset + copy_n;
      size_type assign_n = std::min<size_type>(ilist.end() - it, m_end - ot);
      ot = std::copy_n(it, assign_n, ot);
      
      // List remaining
      ot = std::uninitialized_copy(it + assign_n, ilist.end(), ot);
      
      // This overflow
      destroy_range_backward(ot, m_end);
      m_end = ot;
    }
    else
    {
      clear();
      reserve_without_offset(listSize);
      std::uninitialized_copy(ilist.begin(), ilist.end(), m_begin);
      m_end += listSize;
    }
    
    return *this;
  }
  
  void assign(size_type count, const T& value)
  {
    size_type capacity_ = capacity();
    if (count <= capacity_)
    {
      auto new_offset = m_begin + realloc_offset(count, capacity_);
      
      // Before new offset
      m_offset = destroy_range(m_offset, new_offset);
      
      // Before old offset
      size_type copy_n = std::min<size_type>(m_offset - new_offset, count);
      std::uninitialized_fill_n(new_offset, copy_n, value);
      m_offset = new_offset;
      
      // Common part
      iterator ot = new_offset + copy_n;
      size_type assign_n = std::min<size_type>(count - copy_n, m_end - ot);
      ot = std::fill_n(ot, assign_n, value);
      
      // Remaining
      ot = std::uninitialized_fill_n(ot, count - (assign_n + copy_n), value);
      
      // This overflow
      destroy_range_backward(ot, m_end);
      m_end = ot;
    }
    else
    {
      clear();
      reserve_without_offset(count);
      m_end = std::uninitialized_fill_n(m_begin, count, value);
    }
  }
  
  void assign(const_iterator first, const_iterator last)
  {
    size_type capacity_ = capacity();
    size_type count = last - first;
    if (count <= capacity_)
    {
      auto new_offset = m_begin + realloc_offset(count, capacity_);
      
      // Before new offset
      m_offset = destroy_range(m_offset, new_offset);
      
      // Before old offset
      size_type copy_n = std::min<size_type>(m_offset - new_offset, count);
      std::uninitialized_copy_n(first, copy_n, new_offset);
      m_offset = new_offset;
      
      // Common part
      const_iterator it = first + copy_n;
      iterator ot = new_offset + copy_n;
      size_type assign_n = std::min<size_type>(last - it, m_end - ot);
      ot = std::copy_n(it, assign_n, ot);
      
      // Remaining
      ot = std::uninitialized_copy(it + assign_n, last, ot);
      
      // This overflow
      destroy_range_backward(ot, m_end);
      m_end = ot;
    }
    else
    {
      clear();
      reserve_without_offset(count);
      std::uninitialized_copy(first, last, m_begin);
      m_end += count;
    }
  }
  
  void assign(std::initializer_list<T> ilist)
  {
    operator=(ilist);
  }
  
  //
  // Capacity
  //
  bool empty() const noexcept
  {
    return (m_offset == m_end);
  }
  
  size_type size() const noexcept
  {
    return (m_end - m_offset);
  }
  
  size_type max_size() const noexcept
  {
    return (std::numeric_limits<difference_type>::max() / 2);
  }
  
  size_type capacity() const noexcept
  {
    return (m_end_of_storage - m_begin);
  }
  
  void shrink_to_fit()
  {
    if (m_begin != nullptr) // capacity > 0
    {
      if (empty())
      {
        // Reset
        delete[] reinterpret_cast<storage_type*>(m_begin);
        m_begin = nullptr;
        m_offset = nullptr;
        m_end = nullptr;
        m_end_of_storage = nullptr;
      }
      else if (m_end != m_end_of_storage || m_offset != m_begin) // size != capacity
      {
        m_end_of_storage = m_begin; // force re-allocation
        reserve_without_offset(size());
      }
    }
  }
  
  void reserve(size_type new_cap)
  {
    reserve_shifted(new_cap, 0);
  }
  
  //
  // Iterators
  //
  iterator        begin()       noexcept  { return m_offset; }
  const_iterator  begin() const noexcept  { return m_offset; }
  const_iterator cbegin() const noexcept  { return m_offset; }
  
  iterator        end()       noexcept  { return m_end; }
  const_iterator  end() const noexcept  { return m_end; }
  const_iterator cend() const noexcept  { return m_end; }
  
  reverse_iterator        rbegin()       noexcept { return std::reverse_iterator<      iterator>(m_end); }
  const_reverse_iterator  rbegin() const noexcept { return std::reverse_iterator<const_iterator>(m_end); }
  const_reverse_iterator crbegin() const noexcept { return std::reverse_iterator<const_iterator>(m_end); }
  
  reverse_iterator        rend()       noexcept { return std::reverse_iterator<      iterator>(m_offset); }
  const_reverse_iterator  rend() const noexcept { return std::reverse_iterator<const_iterator>(m_offset); }
  const_reverse_iterator crend() const noexcept { return std::reverse_iterator<const_iterator>(m_offset); }
  
  //
  // Element access
  //
  reference operator[](size_type pos)
  {
    assert(pos < size());
    return *(m_offset + pos);
  }
  
  const_reference operator[](size_type pos) const
  {
    assert(pos < size());
    return *(m_offset + pos);
  }
  
  reference at(size_type pos)
  {
    if (pos >= size())
      throw std::out_of_range("offset_vector::at");
    return *(m_offset + pos);
  }
  
  const_reference at(size_type pos) const
  {
    if (pos >= size())
      throw std::out_of_range("offset_vector::at");
    return *(m_offset + pos);
  }
  
  reference front()
  {
    assert(!empty());
    return *(m_offset);
  }
  
  const_reference front() const
  {
    assert(!empty());
    return *(m_offset);
  }
  
  reference back()
  {
    assert(!empty());
    return *(m_end - 1);
  }
  
  const_reference back() const
  {
    assert(!empty());
    return *(m_end - 1);
  }
  
  T* data() noexcept
  {
    return m_offset;
  }
  
  const T* data() const noexcept
  {
    return m_offset;
  }
  
  // Non-standard
#ifdef INDIVI_OV_DEBUG
  T* storage() noexcept
  {
    return m_begin;
  }
  
  const T* storage() const noexcept
  {
    return m_begin;
  }
#endif
  size_type offset() const noexcept
  {
    return (m_offset - m_begin);
  }
  
  //
  // Modifiers
  //
  void clear() noexcept
  {
    destroy_range(m_offset, m_end);
    
  #ifdef INDIVI_OV_SHIFT_EMPTY
    m_offset = m_begin + realloc_offset(0, capacity());
  #endif
    m_end = m_offset;
  }
  
  void push_back(const T& value)
  {
    if (m_end == m_end_of_storage)
    {
      if (!shift_data_left())
        reserve_shifted(std::max<size_type>(1, (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F))), 1);
    }
    assert(m_end < m_end_of_storage);
    ::new (static_cast<void*>(m_end)) T(value);
    ++m_end;
  }
  
  void push_back(T&& value)
  {
    if (m_end == m_end_of_storage)
    {
      if (!shift_data_left())
        reserve_shifted(std::max<size_type>(1, (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F))), 1);
    }
    assert(m_end < m_end_of_storage);
    ::new (static_cast<void*>(m_end)) T(std::forward<T>(value));
    ++m_end;
  }
  
  template<class... Args>
  void emplace_back(Args&&... args)
  {
    if (m_end == m_end_of_storage)
    {
      if (!shift_data_left())
        reserve_shifted(std::max<size_type>(1, (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F))), 1);
    }
    assert(m_end < m_end_of_storage);
    ::new (static_cast<void*>(m_end)) T(std::forward<Args>(args)...);
    ++m_end;
  }
  
  // Non-standard
  void push_front(const T& value)
  {
    if (m_offset == m_begin)
    {
      if (!shift_data_right())
        reserve_shifted(std::max<size_type>(1, (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F))), 0, 1);
    }
    assert(m_offset > m_begin);
    ::new (static_cast<void*>(m_offset-1)) T(value);
    --m_offset;
  }
  
  void push_front(T&& value)
  {
    if (m_offset == m_begin)
    {
      if (!shift_data_right())
        reserve_shifted(std::max<size_type>(1, (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F))), 0, 1);
    }
    assert(m_offset > m_begin);
    ::new (static_cast<void*>(m_offset-1)) T(std::forward<T>(value));
    --m_offset;
  }
  
  template<class... Args>
  void emplace_front(Args&&... args)
  {
    if (m_offset == m_begin)
    {
      if (!shift_data_right())
        reserve_shifted(std::max<size_type>(1, (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F))), 0, 1);
    }
    assert(m_offset > m_begin);
    ::new (static_cast<void*>(m_offset-1)) T(std::forward<Args>(args)...);
    --m_offset;
  }
  
  
  iterator insert(const_iterator pos, const T& value)
  {
    assert(pos >= m_offset);
    assert(pos <= m_end);
    
    // Shift left (decrease offset)
    if (m_offset != m_begin
        && (((size_type)(pos - m_offset) <= size() / 2) || m_end == m_end_of_storage))
    {
      // First
      if (pos == m_offset)
      {
        ::new (static_cast<void*>(m_offset-1)) T(value);
        return --m_offset;
      }
      // Insert
      iterator it = m_offset;
      ::new (static_cast<void*>(it - 1)) T(std::move(*it));
      --m_offset;
      
      it = std::move(it + 1, (iterator)pos, it);
      
      *it = value;
      return it;
    }
    else
    {
      // Re-alloc
      if (m_end == m_end_of_storage)
      {
        difference_type diff = pos - m_offset;
        realloc_insert(pos, value);
        return m_offset + diff;
      }
      // Shift right (increase end)
      // Last
      if (pos == m_end)
      {
        ::new (static_cast<void*>(m_end)) T(value);
        return m_end++;
      }
      // Insert
      iterator it = m_end - 1;
      ::new (static_cast<void*>(it + 1)) T(std::move(*it));
      ++m_end;
      
      it = std::move_backward((iterator)pos, it, it + 1) - 1;
      
      *it = value;
      return it;
    }
  }
  
  iterator insert(const_iterator pos, T&& value)
  {
    assert(pos >= m_offset);
    assert(pos <= m_end);
    
    // Shift left (decrease offset)
    if (m_offset != m_begin
        && (((size_type)(pos - m_offset) <= size() / 2) || m_end == m_end_of_storage))
    {
      // First
      if (pos == m_offset)
      {
        ::new (static_cast<void*>(m_offset-1)) T(std::forward<T>(value));
        return --m_offset;
      }
      // Insert
      iterator it = m_offset;
      ::new (static_cast<void*>(it - 1)) T(std::move(*it));
      --m_offset;
      
      it = std::move(it + 1, (iterator)pos, it);
      
      *it = std::forward<T>(value);
      return it;
    }
    else
    {
      // Re-alloc
      if (m_end == m_end_of_storage)
      {
        difference_type diff = pos - m_offset;
        realloc_insert(pos, std::forward<T>(value));
        return m_offset + diff;
      }
      // Shift right (increase end)
      // Last
      if (pos == m_end)
      {
        ::new (static_cast<void*>(m_end)) T(std::forward<T>(value));
        return m_end++;
      }
      // Insert
      iterator it = m_end - 1;
      ::new (static_cast<void*>(it + 1)) T(std::move(*it));
      ++m_end;
      
      it = std::move_backward((iterator)pos, it, it + 1) - 1;
      
      *it = std::forward<T>(value);
      return it;
    }
  }
  
  iterator insert(const_iterator pos, size_type count, const T& value)
  {
    assert(pos >= m_offset);
    assert(pos <= m_end);
    
    size_type size_ = size();
    size_type capacity_ = capacity();
    if (size_ + count > capacity_)
    {
      difference_type diff = pos - m_offset;
      realloc_insert(pos, count, value);
      return m_offset + diff;
    }
    if (count == 0)
      return (iterator)pos;
    
    bool can_shift_left  = m_offset - count >= m_begin          || pos == m_end;
    bool can_shift_right = m_end    + count <= m_end_of_storage || pos == m_offset;
    size_type to_shift_left  = pos != m_end ? (size_type)(pos - m_offset)
                                            : (size_type)(m_end_of_storage - m_end) >= count ? 0 : size_;
    size_type to_shift_right = pos != m_offset ? (size_type)(m_end - pos)
                                               : (size_type)(m_offset - m_begin) >= count ? 0 : size_;
    
    // Shift left (decrease offset)
    if (can_shift_left && (to_shift_left <= to_shift_right || !can_shift_right))
    {
      // Insert before
      if (pos == m_offset)
      {
        iterator new_offset = std::max<iterator>(m_offset - count, m_begin);
        std::uninitialized_fill_n(new_offset, count, value);
        m_end = new_offset + size_ + count; // handle (size == 0)
        return m_offset = new_offset;
      }
      
      iterator old_offset = m_offset;
      size_type shift_left_dist = std::min<size_type>(m_offset - m_begin, count);
      size_type copy_n = std::min<size_type>(pos - m_offset, shift_left_dist);
      
      // Before offset fill
      size_type fill_n = shift_left_dist - copy_n;
      std::uninitialized_fill_n(m_offset - fill_n, fill_n, value);
      m_offset -= fill_n;
      
      // Before offset copy
      iterator new_offset = m_offset - copy_n;
      std::uninitialized_copy_n(std::make_move_iterator(old_offset), copy_n, new_offset);
      m_offset = new_offset;
      
      // After offset move
      iterator it = old_offset + copy_n;
      iterator ot = std::move(it, (iterator)pos, old_offset);
      
      // After offset assign
      size_type assign_n = std::min<size_type>(count - fill_n, m_end - ot);
      std::fill_n(ot, assign_n, value);
      
      // After offset fill
      size_type fill_n2 = count - (assign_n + fill_n);
      m_end = std::uninitialized_fill_n(m_end, fill_n2, value);
      
      return fill_n > 0 ? old_offset - fill_n
                        : (assign_n > 0 ? ot : m_end - fill_n2);
    }
    else
    {
      // Shift right (increase offset)
      if (can_shift_right)
      {
        assert(size_ > 0);
        assert(pos != m_end);
        
        iterator old_end = m_end;
        size_type shift_right_dist = std::min<size_type>(m_end_of_storage - m_end, count);
        size_type copy_n = std::min<size_type>(m_end - pos, shift_right_dist);
        
        // After end fill
        size_type fill_n = shift_right_dist - copy_n;
        m_end = std::uninitialized_fill_n(m_end, fill_n, value);
        
        // After end copy
        iterator it = old_end - copy_n;
        m_end = std::uninitialized_copy_n(std::make_move_iterator(it), copy_n, m_end);
        
        // Before end move
        iterator ot = std::move_backward((iterator)pos, it, old_end);
        
        // Before end assign
        size_type assign_n = std::min<size_type>(count - fill_n, ot - m_offset);
        std::fill_n(ot - assign_n, assign_n, value);
        
        // Before end fill
        size_type fill_n2 = count - (assign_n + fill_n);
        std::uninitialized_fill_n(m_offset - fill_n2, fill_n2, value);
        m_offset -= fill_n2;
        
        return fill_n2 > 0 ? m_offset : (assign_n > 0 ? ot - assign_n : old_end);
      }
      
      // Shift both: left first
      //
      iterator old_offset = m_offset;
      iterator old_end = m_end;
      size_type shift_left_dist = std::min<size_type>(m_offset - m_begin, (count + 1) / 2);
      size_type copy_n = std::min<size_type>(pos - m_offset, shift_left_dist);
      
      // Before offset fill
      size_type fill_n = shift_left_dist - copy_n;
      std::uninitialized_fill_n(m_offset - fill_n, fill_n, value);
      m_offset -= fill_n;
      
      // Before offset copy
      iterator new_offset = m_offset - copy_n;
      std::uninitialized_copy_n(std::make_move_iterator(old_offset), copy_n, new_offset);
      m_offset = new_offset;
      
      // After offset move
      iterator it = old_offset + copy_n;
      iterator ot = std::move(it, (iterator)pos, old_offset);
      
      // Then shift right
      //
      size_type shift_right_dist = (m_offset + size_ + count) - m_end;
      size_type copy_n2 = std::min<size_type>(m_end - pos, shift_right_dist);
    
      // After end fill
      size_type fill_n2 = shift_right_dist - copy_n2;
      m_end = std::uninitialized_fill_n(m_end, fill_n2, value);
      
      // After end copy
      iterator it2 = old_end - copy_n2;
      m_end = std::uninitialized_copy_n(std::make_move_iterator(it2), copy_n2, m_end);
      
      // Before end move
      std::move_backward((iterator)pos, it2, old_end);
      
      // Center
      // After offset assign
      size_type assign_n = count - fill_n - fill_n2;
      std::fill_n(ot, assign_n, value);
      
      return fill_n > 0 ? old_offset - fill_n
                        : (assign_n > 0 ? ot : old_end);
    }
  }
  
  iterator insert(const_iterator pos, iterator first, iterator last)
  {
    assert(pos >= m_offset);
    assert(pos <= m_end);
    assert(first <= last);
    
    size_type count = last - first;
    size_type size_ = size();
    size_type capacity_ = capacity();
    if (size_ + count > capacity_)
    {
      difference_type diff = pos - m_offset;
      realloc_insert(pos, first, last);
      return m_offset + diff;
    }
    if (count == 0)
      return (iterator)pos;
    
    bool can_shift_left  = m_offset - count >= m_begin          || pos == m_end;
    bool can_shift_right = m_end    + count <= m_end_of_storage || pos == m_offset;
    size_type to_shift_left  = pos != m_end ? (size_type)(pos - m_offset)
                                            : (size_type)(m_end_of_storage - m_end) >= count ? 0 : size_;
    size_type to_shift_right = pos != m_offset ? (size_type)(m_end - pos)
                                               : (size_type)(m_offset - m_begin) >= count ? 0 : size_;
    
    // Shift left (decrease offset)
    if (can_shift_left && (to_shift_left <= to_shift_right || !can_shift_right))
    {
      // Insert before
      if (pos == m_offset)
      {
        iterator new_offset = std::max<iterator>(m_offset - count, m_begin);
        std::uninitialized_copy(first, last, new_offset);
        m_end = new_offset + size_ + count; // handle (size == 0)
        return m_offset = new_offset;
      }
      
      iterator old_offset = m_offset;
      size_type shift_left_dist = std::min<size_type>(m_offset - m_begin, count);
      size_type copy_n = std::min<size_type>(pos - m_offset, shift_left_dist);
      
      // Before offset fill
      size_type fill_n = shift_left_dist - copy_n;
      std::uninitialized_copy_n(first, fill_n, m_offset - fill_n);
      first += fill_n;
      m_offset -= fill_n;
      
      // Before offset copy
      iterator new_offset = m_offset - copy_n;
      std::uninitialized_copy_n(std::make_move_iterator(old_offset), copy_n, new_offset);
      m_offset = new_offset;
      
      // After offset move
      iterator it = old_offset + copy_n;
      iterator ot = std::move(it, (iterator)pos, old_offset);
      
      // After offset assign
      size_type assign_n = std::min<size_type>(count - fill_n, m_end - ot);
      std::copy_n(first, assign_n, ot);
      first += assign_n;
      
      // After offset fill
      size_type fill_n2 = count - (assign_n + fill_n);
      m_end = std::uninitialized_copy_n(first, fill_n2, m_end);
      
      return fill_n > 0 ? old_offset - fill_n
                        : (assign_n > 0 ? ot : m_end - fill_n2);
    }
    else
    {
      // Shift right (increase offset)
      if (can_shift_right)
      {
        assert(size_ > 0);
        assert(pos != m_end);
        
        iterator old_end = m_end;
        size_type shift_right_dist = std::min<size_type>(m_end_of_storage - m_end, count);
        size_type copy_n = std::min<size_type>(m_end - pos, shift_right_dist);
        
        // After end fill
        size_type fill_n = shift_right_dist - copy_n;
        m_end = std::uninitialized_copy_n(first, fill_n, m_end);
        first += fill_n;
        
        // After end copy
        iterator it = old_end - copy_n;
        m_end = std::uninitialized_copy_n(std::make_move_iterator(it), copy_n, m_end);
        
        // Before end move
        iterator ot = std::move_backward((iterator)pos, it, old_end);
        
        // Before end assign
        size_type assign_n = std::min<size_type>(count - fill_n, ot - m_offset);
        std::copy_n(first, assign_n, ot - assign_n);
        first += assign_n;
        
        // Before end fill
        size_type fill_n2 = count - (assign_n + fill_n);
        std::uninitialized_copy_n(first, fill_n2, m_offset - fill_n2);
        first += fill_n2;
        m_offset -= fill_n2;
        
        return fill_n2 > 0 ? m_offset : (assign_n > 0 ? ot - assign_n : old_end);
      }
      
      // Shift both: left first
      //
      iterator old_offset = m_offset;
      iterator old_end = m_end;
      size_type shift_left_dist = std::min<size_type>(m_offset - m_begin, (count + 1) / 2);
      size_type copy_n = std::min<size_type>(pos - m_offset, shift_left_dist);
      
      // Before offset fill
      size_type fill_n = shift_left_dist - copy_n;
      std::uninitialized_copy_n(first, fill_n, m_offset - fill_n);
      first += fill_n;
      m_offset -= fill_n;
      
      // Before offset copy
      iterator new_offset = m_offset - copy_n;
      std::uninitialized_copy_n(std::make_move_iterator(old_offset), copy_n, new_offset);
      m_offset = new_offset;
      
      // After offset move
      iterator it = old_offset + copy_n;
      iterator ot = std::move(it, (iterator)pos, old_offset);
      
      // Then shift right
      //
      size_type shift_right_dist = (m_offset + size_ + count) - m_end;
      size_type copy_n2 = std::min<size_type>(m_end - pos, shift_right_dist);
    
      // After end fill
      size_type fill_n2 = shift_right_dist - copy_n2;
      m_end = std::uninitialized_copy_n(last - fill_n2, fill_n2, m_end);
      
      // After end copy
      iterator it2 = old_end - copy_n2;
      m_end = std::uninitialized_copy_n(std::make_move_iterator(it2), copy_n2, m_end);
      
      // Before end move
      std::move_backward((iterator)pos, it2, old_end);
      
      // Center
      // After offset assign
      size_type assign_n = count - fill_n - fill_n2;
      std::copy_n(first, assign_n, ot);
      
      return fill_n > 0 ? old_offset - fill_n
                        : (assign_n > 0 ? ot : old_end);
    }
  }
  
  iterator insert(const_iterator pos, std::initializer_list<T> ilist)
  {
    return insert(pos, (iterator)ilist.begin(), (iterator)ilist.end());
  }
  
  template< class... Args >
  iterator emplace(const_iterator pos, Args&&... args)
  {
    assert(pos >= m_offset);
    assert(pos <= m_end);
    
    // Shift left (decrease offset)
    if (m_offset != m_begin
        && (((size_type)(pos - m_offset) <= size() / 2) || m_end == m_end_of_storage))
    {
      // First
      if (pos == m_offset)
      {
        ::new (static_cast<void*>(m_offset-1)) T(std::forward<Args>(args)...);
        return --m_offset;
      }
      // Insert
      T value(std::forward<Args>(args)...);
      
      iterator it = m_offset;
      ::new (static_cast<void*>(it - 1)) T(std::move(*it));
      --m_offset;
      
      it = std::move(it + 1, (iterator)pos, it);
      
      *it = std::move(value);
      return it;
    }
    else
    {
      // Re-alloc
      if (m_end == m_end_of_storage)
      {
        difference_type diff = pos - m_offset;
        realloc_insert(pos, std::forward<Args>(args)...);
        return m_offset + diff;
      }
      // Shift right (increase end)
      // Last
      if (pos == m_end)
      {
        ::new (static_cast<void*>(m_end)) T(std::forward<Args>(args)...);
        return m_end++;
      }
      // Insert
      T value(std::forward<Args>(args)...);
      
      iterator it = m_end - 1;
      ::new (static_cast<void*>(it + 1)) T(std::move(*it));
      ++m_end;
      
      it = std::move_backward((iterator)pos, it, it + 1) - 1;

      *it = std::move(value);
      return it;
    }
  }
  
  void pop_back()
  {
    assert(m_end != m_offset);
    
    (--m_end)->~T();
    
  #ifdef INDIVI_OV_SHIFT_EMPTY
    if (empty())
    {
      m_offset = m_begin + realloc_offset(0, capacity());
      m_end = m_offset;
    }
  #endif
  }
  
  // Non-standard
  void pop_front()
  {
    assert(m_end != m_offset);
    
    (m_offset++)->~T();
    
  #ifdef INDIVI_OV_SHIFT_EMPTY
    if (empty())
    {
      m_offset = m_begin + realloc_offset(0, capacity());
      m_end = m_offset;
    }
  #endif
  }
  
  iterator erase(const_iterator pos)
  {
    assert(pos >= m_offset);
    assert(pos < m_end);
    
    if ((size_type)(pos - m_offset) < size() / 2)  // Shift right (increase offset)
    {
      if (pos == m_offset)
      {
        (m_offset++)->~T();
        return m_offset;
      }
      else
      {
        iterator it = (iterator)pos;
        std::move_backward(m_offset, it, it + 1);
        
        (m_offset++)->~T();
        return it + 1;
      }
    }
    else // Shift left (decrease end)
    {
      if (pos == m_end - 1)
      {
        (--m_end)->~T();
        
      #ifdef INDIVI_OV_SHIFT_EMPTY
        if (empty())
        {
          m_offset = m_begin + realloc_offset(0, capacity());
          m_end = m_offset;
        }
      #endif
        return m_end;
      }
      else
      {
        iterator it = (iterator)pos;
        std::move(it + 1, m_end, it);
        
        (--m_end)->~T();
        return it;
      }
    }
  }
  
  iterator erase(const_iterator first, const_iterator last)
  {
    if (first == last)
      return (iterator)last;
    
    assert(first >= m_offset);
    assert(first < last);
    assert(last <= m_end);
    
    if (first - m_offset < m_end - last) // Shift right (increase offset)
    {
      iterator ot = std::move_backward(m_offset, (iterator)first, (iterator)last);
      
      m_offset = destroy_range(m_offset, ot);
      
      return (iterator)last;
    }
    else  // Shift left (decrease end)
    {
      iterator ot = std::move((iterator)last, m_end, (iterator)first);
      
      m_end = destroy_range_backward(ot, m_end);
      
    #ifdef INDIVI_OV_SHIFT_EMPTY
      if (empty())
      {
        m_offset = m_begin + realloc_offset(0, capacity());
        m_end = m_offset;
        
        return m_offset;
      }
    #endif
      return (iterator)first;
    }
  }
  
  void resize(size_type count)
  {
    if (count <= size())
    {
      // Remove overflow
      iterator new_end = m_offset + count;
      m_end = destroy_range_backward(new_end, m_end);
      
    #ifdef INDIVI_OV_SHIFT_EMPTY
      if (count == 0)
      {
        m_offset = m_begin + realloc_offset(0, capacity());
        m_end = m_offset;
      }
    #endif
    }
    else
    {
      if (count > capacity())
      {
        // Re-alloc
        reserve_without_offset(count);
      }
      else
      {
        // Not enough space at end
        if (m_offset + count > m_end_of_storage)
          shift_data_left(count);
      }
      // Push back
      iterator new_end = m_offset + count;
      iterator it = m_end;
      try {
        for (; it != new_end; ++it)
          ::new (static_cast<void*>(it)) INDIVI_OV_VALUE_INIT;
      }
      catch (...) {
        destroy_range_backward(m_end, it);
        throw;
      }
      m_end = new_end;
    }
  }
  
  void resize(size_type count, const value_type& value)
  {
    if (count <= size())
    {
      // Remove overflow
      auto new_end = m_offset + count;
      m_end = destroy_range_backward(new_end, m_end);
      
    #ifdef INDIVI_OV_SHIFT_EMPTY
      if (count == 0)
      {
        m_offset = m_begin + realloc_offset(0, capacity());
        m_end = m_offset;
      }
    #endif
    }
    else
    {
      if (count > capacity())
      {
        // Re-alloc
        reserve_without_offset(count);
      }
      else
      {
        // Not enough space at end
        if (m_offset + count > m_end_of_storage)
          shift_data_left(count);
      }
      // Push back
      auto copy_n = count - size();
      m_end = std::uninitialized_fill_n(m_end, copy_n, value);
    }
  }
  
  void swap(offset_vector& other) noexcept
  {
    T* begin = m_begin;
    T* offset = m_offset;
    T* end = m_end;
    T* end_of_storage = m_end_of_storage;
    
    m_begin = other.m_begin;
    m_offset = other.m_offset;
    m_end = other.m_end;
    m_end_of_storage = other.m_end_of_storage;
    
    other.m_begin = begin;
    other.m_offset = offset;
    other.m_end = end;
    other.m_end_of_storage = end_of_storage;
  }
  
  // Non-standard
  void shift_data_start()
  {
    if (m_offset != m_begin)
      shift_data_left(m_begin, size());
  }
  
  void shift_data_end()
  {
    if (m_end != m_end_of_storage)
      shift_data_right(m_end_of_storage, size());
  }
  
  void shift_data_center()
  {
    size_type size_ = size();
    size_type capacity_ = std::max<size_type>(capacity(), 1);
    if (size_ == 0)
    {
      m_offset = m_begin + (capacity_ - 1) / 2;
      m_end = m_offset;
      return;
    }
    iterator new_offset = m_begin + (capacity_ - size_) / 2;

    if (new_offset < m_offset)
      shift_data_left(new_offset, size_);
    else if (new_offset > m_offset)
      shift_data_right(new_offset + size_, size_);
  }
  
private:
  iterator destroy_range(iterator first, iterator last)
  {
    for (; first < last; ++first)
      first->~T();
    
    return first;
  }
  
  iterator destroy_range_backward(iterator first, iterator last)
  {
    while (last > first)
      (--last)->~T();
    
    return last;
  }
  
  void shift_data_left(iterator new_offset, size_type size)
  {
    assert(new_offset >= m_begin);
    assert(new_offset < m_offset);
    
    // Before offset
    iterator old_offset = m_offset;
    size_type copy_n = std::min<size_type>(m_offset - new_offset, size);
    iterator ot = std::uninitialized_copy_n(std::make_move_iterator(m_offset),
                                            copy_n,
                                            new_offset);
    m_offset = new_offset;
    
    // After offset
    iterator it = old_offset + copy_n;
    ot = std::move(it, m_end, ot);
    
    // Destruct remaining
    it = std::max<iterator>(ot, old_offset);
    destroy_range_backward(it, m_end);
    m_end = ot;
  }
  
  void shift_data_right(iterator new_end, size_type size)
  {
    assert(new_end <= m_end_of_storage);
    assert(new_end > m_end);
    
    // After end
    iterator old_end = m_end;
    size_type copy_n = std::min<size_type>(new_end - m_end, size);
    std::uninitialized_copy_n(std::make_move_iterator(m_end - copy_n),
                              copy_n,
                              new_end - copy_n);
    m_end = new_end;
    
    // Before end
    iterator it = old_end - copy_n;
    iterator ot = std::move_backward(m_offset, it, new_end - copy_n);
    
    // Destruct remaining
    it = std::min<iterator>(ot, old_end);
    destroy_range(m_offset, it);
    m_offset = ot;
  }
  
  void shift_data_left(size_type new_size)
  {
    size_type size_ = size();
    size_type capacity_ = capacity();
    assert(new_size > size_);
    assert(new_size <= capacity_);
    assert(m_offset != m_begin);
    
    switch(SHIFT_MODE)
    {
      case ShiftMode::NEAR:
      {
        iterator new_offset = m_end - new_size;
        if (size_ == 0)
        {
        #ifdef INDIVI_OV_SHIFT_EMPTY
          m_offset = m_begin + realloc_offset(new_size, capacity_);
        #else
          m_offset = std::max(m_begin, new_offset);
        #endif
          m_end = m_offset;
          return;
        }
        
        shift_data_left(new_offset, size_);
        break;
      }
      case ShiftMode::CENTER:
      {
        size_type off = (capacity_ - new_size) / 2;
        iterator new_offset = m_begin + off;
        if (size_ == 0)
        {
          m_offset = new_offset;
          m_end = m_offset;
          return;
        }
        
        shift_data_left(new_offset, size_);
        break;
      }
      case ShiftMode::FAR:
      {
        if (size_ == 0)
        {
          m_offset = m_begin;
          m_end = m_begin;
          return;
        }
        
        shift_data_left(m_begin, size_);
        break;
      }
      default:
        assert(false && "offset_vector::shift_data_left");
    }
  }
  
  bool shift_data_left()
  {
    size_type size_ = size();
    size_type capacity_ = capacity();
    if (size_ + 1 > capacity_)
      return false;

    assert(m_offset != m_begin);
    assert(m_end == m_end_of_storage);
    
    switch(SHIFT_MODE)
    {
      case ShiftMode::NEAR:
      {
        if (size_ == 0)
        {
          --m_offset;
          --m_end;
          return true;
        }
        
        iterator it = m_offset;
        ::new (static_cast<void*>(it - 1)) T(std::move(*it));
        --m_offset;
        
        std::move(it + 1, m_end, it);
        (--m_end)->~T();
        break;
      }
      case ShiftMode::CENTER:
      {
        size_type off = (capacity_ - (size_ + 1)) / 2;
        iterator new_offset = m_begin + off;
        if (size_ == 0)
        {
          m_offset = new_offset;
          m_end = new_offset;
          return true;
        }
        
        shift_data_left(new_offset, size_);
        break;
      }
      case ShiftMode::FAR:
      {
        if (size_ == 0)
        {
          m_offset = m_begin;
          m_end = m_begin;
          return true;
        }
        
        shift_data_left(m_begin, size_);
        break;
      }
      default:
        assert(false && "offset_vector::shift_data_left");
    }
    return true;
  }
  
  bool shift_data_right()
  {
    size_type size_ = size();
    size_type capacity_ = capacity();
    if (size_ + 1 > capacity_)
      return false;
    
    assert(m_offset == m_begin);
    assert(m_end != m_end_of_storage);
    
    switch(SHIFT_MODE)
    {
      case ShiftMode::NEAR:
      {
        if (size_ == 0)
        {
          ++m_offset;
          ++m_end;
          return true;
        }
        
        iterator it = m_end - 1;
        ::new (static_cast<void*>(m_end)) T(std::move(*it));
        ++m_end;
        
        std::move_backward(m_offset, it, it + 1);
        (m_offset++)->~T();
        break;
      }
      case ShiftMode::CENTER:
      {
        size_type off = (capacity_ - (size_ + 1)) / 2;
        iterator new_end = m_end_of_storage - off;
        if (size_ == 0)
        {
          m_offset = new_end;
          m_end = new_end;
          return true;
        }

        shift_data_right(new_end, size_);
        break;
      }
      case ShiftMode::FAR:
      {
        if (size_ == 0)
        {
          m_offset = m_end_of_storage;
          m_end = m_end_of_storage;
          return true;
        }
        
        shift_data_right(m_end_of_storage, size_);
        break;
      }
      default:
        assert(false && "offset_vector::shift_data_right");
    }
    return true;
  }
  
  static size_type realloc_offset(size_type new_size, size_type new_cap)
  {
    switch(REALLOC_MODE)
    {
      case ReallocMode::START:
        return 0;
      case ReallocMode::CENTER:
        return (new_cap - std::min<size_type>(std::max<size_type>(new_size, 1), new_cap)) / 2;
      case ReallocMode::END:
        return new_cap - new_size;
      default:
        assert(false && "offset_vector::realloc_offset");
    }
    return 0;
  }
  
  void reserve_shifted(size_type new_cap, size_type rightOffset, size_type leftOffset = 0)
  {
    if (new_cap > max_size())
      throw std::length_error("offset_vector::reserve");
    
    if (new_cap > capacity())
    {
      if (empty())
      {
        assert(new_cap >= leftOffset + rightOffset);
        // Delete and re-allocate
        delete[] reinterpret_cast<storage_type*>(m_begin);
        m_begin = nullptr;
        m_begin = reinterpret_cast<T*>(new storage_type[new_cap]);
        m_offset = m_begin + realloc_offset(leftOffset + rightOffset, new_cap) + leftOffset;
        m_end = m_offset;
        m_end_of_storage = m_begin + new_cap;
      }
      else
      {
        // Allocate new, copy then delete old
        size_type size_ = size();
        assert(new_cap >= size_ + leftOffset + rightOffset);
        
        auto new_storage = std::unique_ptr<storage_type[]>(new storage_type[new_cap]);
        auto new_begin = reinterpret_cast<T*>(new_storage.get());
        auto new_offset = new_begin
            + realloc_offset(size_ + leftOffset + rightOffset, new_cap) + leftOffset;
        
        std::uninitialized_copy(std::make_move_iterator(m_offset),
                                std::make_move_iterator(m_end),
                                new_offset);
        
        destroy_range(m_offset, m_end);
        delete[] reinterpret_cast<storage_type*>(m_begin);
        
        m_begin = new_begin;
        m_offset = new_offset;
        m_end = m_offset + size_;
        m_end_of_storage = m_begin + new_cap;
        new_storage.release();
      }
    }
  }
  
  void reserve_without_offset(size_type new_cap)
  {
    if (new_cap > max_size())
      throw std::length_error("offset_vector::reserve_without_offset");
    
    if (new_cap > capacity())
    {
      if (empty())
      {
        // Delete and re-allocate
        delete[] reinterpret_cast<storage_type*>(m_begin);
        m_begin = nullptr;
        m_begin = reinterpret_cast<T*>(new storage_type[new_cap]);
        m_offset = m_begin;
        m_end = m_offset;
        m_end_of_storage = m_begin + new_cap;
      }
      else
      {
        // Allocate new, copy then delete old
        size_type size_ = size();
        auto new_storage = std::unique_ptr<storage_type[]>(new storage_type[new_cap]);
        auto new_begin = reinterpret_cast<T*>(new_storage.get());
        
        std::uninitialized_copy(std::make_move_iterator(m_offset),
                                std::make_move_iterator(m_end),
                                new_begin);
        
        destroy_range(m_offset, m_end);
        delete[] reinterpret_cast<storage_type*>(m_begin);
        
        m_begin = new_begin;
        m_offset = m_begin;
        m_end = m_offset + size_;
        m_end_of_storage = m_begin + new_cap;
        new_storage.release();
      }
    }
  }
  
  offset_vector(T* begin, size_type offset, size_type capacity)
    : m_begin(begin)
    , m_offset(begin + offset)
    , m_end(begin + offset)
    , m_end_of_storage(begin + capacity)
  {
    assert(offset <= capacity);
  }
  
  void realloc_insert(const_iterator pos, const T& value)
  {
    size_type new_size = size() + 1;
    
    auto new_vec = move_until(pos, new_size);
    ::new (static_cast<void*>(new_vec->m_end)) T(value);
    ++new_vec->m_end;
    move_from(new_vec, (iterator)pos);
    
    swap(*new_vec);
  }
  
  void realloc_insert(const_iterator pos, T&& value)
  {
    size_type new_size = size() + 1;
    
    auto new_vec = move_until(pos, new_size);
    ::new (static_cast<void*>(new_vec->m_end)) T(std::forward<T>(value));
    ++new_vec->m_end;
    move_from(new_vec, (iterator)pos);
    
    swap(*new_vec);
  }
  
  template< class... Args >
  void realloc_insert(const_iterator pos, Args&&... args)
  {
    size_type new_size = size() + 1;
    
    auto new_vec = move_until(pos, new_size);
    ::new (static_cast<void*>(new_vec->m_end)) T(std::forward<Args>(args)...);
    ++new_vec->m_end;
    move_from(new_vec, (iterator)pos);
    
    swap(*new_vec);
  }
  
  void realloc_insert(const_iterator pos, size_type count, const T& value)
  {
    size_type new_size = size() + count;
    
    auto new_vec = move_until(pos, new_size);
    new_vec->m_end = std::uninitialized_fill_n(new_vec->m_end, count, value);
    move_from(new_vec, (iterator)pos);
    
    swap(*new_vec);
  }
  
  template <class InputIt>
  void realloc_insert(const_iterator pos, InputIt first, InputIt last)
  {
    size_type new_size = size() + (last - first);
    
    auto new_vec = move_until(pos, new_size);
    new_vec->m_end = std::uninitialized_copy(first, last, new_vec->m_end);
    move_from(new_vec, (iterator)pos);
    
    swap(*new_vec);
  }
  
  std::unique_ptr<offset_vector> move_until(const_iterator pos, size_type new_size) const
  {
    if (empty())
    {
      return std::unique_ptr<offset_vector>(new offset_vector(
            reinterpret_cast<T*>(new storage_type[new_size]), 0, new_size));
    }
    else
    {
      // Allocate new and move until pos
      size_type new_cap = (size_type)(::ceilf(capacity() * INDIVI_OV_GROWTH_F));
      while (new_cap < new_size)
        new_cap = (size_type)(::ceilf(new_cap * INDIVI_OV_GROWTH_F));
      
      auto new_vec = std::unique_ptr<offset_vector>(new offset_vector(
            reinterpret_cast<T*>(new storage_type[new_cap]), realloc_offset(new_size, new_cap), new_cap));
      
      auto copy_n = pos - m_offset;
      std::uninitialized_copy_n(std::make_move_iterator(m_offset), copy_n, new_vec->m_offset);
      new_vec->m_end += copy_n;
      
      return new_vec;
    }
  }
  
  void move_from(std::unique_ptr<offset_vector>& new_vec, iterator from) const
  {
    assert(new_vec->m_end_of_storage - new_vec->m_end >= m_end - from);
    
    auto copy_n = m_end - from;
    std::uninitialized_copy_n(std::make_move_iterator(from), copy_n, new_vec->m_end);
    new_vec->m_end += copy_n;
  }

}; // end of offset_vector


//
// Non-member functions
//
template<class T>
bool operator==(const offset_vector<T>& lhs, const offset_vector<T>& rhs)
{
  if (lhs.size() != rhs.size())
    return false;
  
  for (auto lit = lhs.cbegin(), rit = rhs.cbegin(); lit != lhs.cend(); ++lit, ++rit)
  {
    if (!(*lit == *rit))
      return false;
  }
  return true;
}

template<class T>
inline bool operator!=(const offset_vector<T>& lhs, const offset_vector<T>& rhs)
{
  if (lhs.size() != rhs.size())
    return true;
  
  for (auto lit = lhs.cbegin(), rit = rhs.cbegin(); lit != lhs.cend(); ++lit, ++rit)
  {
    if (*lit != *rit)
      return true;
  }
  return false;
}

template<class T>
inline bool operator<(const offset_vector<T>& lhs, const offset_vector<T>& rhs)
{
  for (auto lit = lhs.cbegin(), rit = rhs.cbegin(); lit != lhs.cend() && rit != rhs.cend(); ++lit, ++rit)
  {
    if (!(*lit < *rit))
      return false;
  }
  return true;
}

template<class T>
inline bool operator<=(const offset_vector<T>& lhs, const offset_vector<T>& rhs)
{
  for (auto lit = lhs.cbegin(), rit = rhs.cbegin(); lit != lhs.cend() && rit != rhs.cend(); ++lit, ++rit)
  {
    if (!(*lit <= *rit))
      return false;
  }
  return true;
}

template<class T>
inline bool operator>(const offset_vector<T>& lhs, const offset_vector<T>& rhs)
{
  for (auto lit = lhs.cbegin(), rit = rhs.cbegin(); lit != lhs.cend() && rit != rhs.cend(); ++lit, ++rit)
  {
    if (!(*lit > *rit))
      return false;
  }
  return true;
}

template<class T>
inline bool operator>=(const offset_vector<T>& lhs, const offset_vector<T>& rhs)
{
  for (auto lit = lhs.cbegin(), rit = rhs.cbegin(); lit != lhs.cend() && rit != rhs.cend(); ++lit, ++rit)
  {
    if (!(*lit >= *rit))
      return false;
  }
  return true;
}

} // namespace indivi


namespace std
{
  template<class T>
  inline void swap(indivi::offset_vector<T>& lhs, indivi::offset_vector<T>& rhs) noexcept
  {
    lhs.swap(rhs);
  }
}


#endif // INDIVI_OFFSET_VECTOR_H
