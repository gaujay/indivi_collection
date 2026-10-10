/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <iostream>
#include <stdexcept>
#include <string>

#include <cassert>

// arbitrary
#define INIT_CODE (9876)  // _init after ctr
#define MOVE_CODE (7689)  // _init after move
#define DTR_ID_CODE   (-1234) // id after dtr
#define DTR_VAL_CODE  (-4321) // val after dtr
//#define MVE_ID_CODE   (2143)  // id after move
#define MVE_VAL_CODE  (3412)  // val after move


// Debug Class 1
struct DbgClass
{
  static bool Dbg; // need init in translation unit
  static int idx;
  static int count;
  int id; // 0 = destroyed
  
  DbgClass() : id(++idx) { ++count; if (Dbg) std::cout << "Ctr: " << id << "\n"; }
  DbgClass(int id_) : id(id_) { assert(id > 0); ++count; if (Dbg) std::cout << "Ctr: " << id << "\n"; }
  DbgClass(const DbgClass& other) : id(other.id) { assert(id > 0); ++count; if (Dbg) std::cout << "Ctr(const&): " << id << "\n"; }
  DbgClass(DbgClass&& other) noexcept : id(other.id) { assert(id > 0); ++count; other.id = -other.id; if (Dbg) std::cout << "Ctr(&&): " << id << "\n"; }
  ~DbgClass() { assert(id != 0); --count; if (Dbg) std::cout << "Dtr: " << id << "\n"; id = 0; }
  
  DbgClass& operator=(const DbgClass& other) { id = other.id; if (Dbg) std::cout << "Op=&: " << id << "\n"; return *this; }
  DbgClass& operator=(DbgClass&& other) noexcept { id = other.id; other.id = -other.id; if (Dbg) std::cout << "Op=&&: " << id << "\n"; return *this; }
  
  bool operator==(const DbgClass& other) const { return id == other.id; }
  bool operator!=(const DbgClass& other) const { return id != other.id; }
};

// MurmurHash3’s 64-bit finalizer
// by Austin Appleby, public domain
static uint64_t hash_key(uint64_t key)
{
  uint64_t result = key;
  result ^= result >> 33;
  result *= 0xff51afd7ed558ccdull;
  result ^= result >> 33;
  result *= 0xc4ceb9fe1a85ec53ull;
  result ^= result >> 33;
  return result;
}

namespace std {
template <> struct hash<DbgClass> {
  size_t operator()(const DbgClass& x) const { return hash_key(x.id); }
};
}

// Debug Class 2
class dClass
{
public:
  dClass() : val(-1), id(++count), _init(INIT_CODE) {
    if (!quiet) std::cout << "Ctr0: "   << id << " (val: " << val << ")" << std::endl;
  }
  dClass(int v) : val(v), id(++count), _init(INIT_CODE) {
    if (!quiet) std::cout << "Ctr1: "   << id << " (val: " << val << ")" << std::endl;
  }
  dClass(const dClass& d) : val(d.val), id(++count), _init(INIT_CODE) {
    assert(id != d.id);
    assert(d._init == INIT_CODE);
    ++copies;
    if (!quiet) std::cout << "CtrCpy: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  dClass(dClass&& d) noexcept : val(d.val), id(++count), _init(INIT_CODE) {
    assert(id != d.id);
    assert(d._init == INIT_CODE);
    ++moves;
    if (!quiet) std::cout << "CtrMve: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
    d._init = MOVE_CODE;
    d.val = MVE_VAL_CODE;
  }
  
  ~dClass() {
    assert((id != DTR_ID_CODE || val != DTR_VAL_CODE || _init != -INIT_CODE) && "Error: double destruct");
    assert(id > 0 && id <= count && (_init == INIT_CODE || _init == MOVE_CODE) && "Error: garbage destruct");
    if (!quiet) std::cout << "Dtr: " << id << " (val: " << val << ")" << std::endl;
    id  = DTR_ID_CODE;
    val = DTR_VAL_CODE;
    _init = -INIT_CODE;
    ++decount;
  }
  
  dClass& operator=(const dClass& d)
  {
//    assert(id != d.id);
    assert(  _init == INIT_CODE || _init == MOVE_CODE);
    assert(d._init == INIT_CODE);
    ++copies;
    if (!quiet)
      std::cout << "AssignCpy: " << d.id << " overrides " << id
                << " (val: " << d.val << " overrides " << val << ")" << std::endl;
    val = d.val;
    _init = INIT_CODE;
    return *this;
  }
  dClass& operator=(dClass&& d) noexcept
  {
//    assert(id != d.id);
    assert(  _init == INIT_CODE || _init == MOVE_CODE);
    assert(d._init == INIT_CODE);
    ++moves;
    if (!quiet)
      std::cout << "AssignMve: " << d.id << " overrides " << id
                << " (val: " << d.val << " overrides " << val << ")" << std::endl;
    val = d.val;
    _init = INIT_CODE;
    
    d._init = MOVE_CODE;
    d.val = MVE_VAL_CODE;
    return *this;
  }
  
  std::string toString() const { return std::to_string(id) + " (val: " + std::to_string(val) + ")\n"; }
  
  // Non-member functions
  friend bool operator<(const dClass& lhs, const dClass& rhs) { return lhs.val < rhs.val; }
  friend std::ostream& operator<<(std::ostream &os, const dClass& d) { return os << d.val; }
  
  friend bool operator==(const dClass& lhs, const dClass& rhs) { return lhs.val == rhs.val; }
  friend bool operator!=(const dClass& lhs, const dClass& rhs) { return !(lhs.val == rhs.val); }
  
  // Members
  int val;
  int id;
  int _init;
  static int count;
  static int decount;
  static uint64_t copies;
  static uint64_t moves;
  static bool quiet;
  
  static void resetStats() noexcept
  {
    count   = 0;
    decount = 0;
    copies = 0u;
    moves  = 0u;
    quiet  = true;
  }
};


// Error Class, throws on:
template<bool Ctr0=false,   bool Ctr1=false,
         bool CtrCpy=false, bool CtrMve=false,
         bool AsgCpy=false, bool AsgMve=false,
         int WaitCount=0>
class eClass
{
public:
  eClass()                : val(-1),    id(++count) {
    if (Ctr0 && WaitCount < ++countCtr0) {
      --count;
      throw std::runtime_error("Ctr0: " + std::to_string(id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "Ctr0: "   << id << " (val: " << val << ")" << std::endl;
  }
  eClass(int v)           : val(v),     id(++count) {
    bool failure = Ctr1 && WaitCount < ++countCtr1;
    if (failure) {
      --count;
      throw std::runtime_error("Ctr1: " + std::to_string(id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "Ctr1: "   << id << " (val: " << val << ")" << std::endl;
  }
  eClass(const eClass& d) : val(d.val), id(++count) {
    assert(id != d.id);
    bool failure = CtrCpy && WaitCount < ++countCtrCpy;
    if (failure) {
      --count;
      throw std::runtime_error("CtrCpy: " + std::to_string(id) + " from " + std::to_string(d.id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "CtrCpy: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  eClass(eClass&& d)      : val(d.val), id(++count) {
    assert(id != d.id);
    bool failure = CtrMve && WaitCount < ++countCtrMve;
    if (failure) {
      --count;
      throw std::runtime_error("CtrMve: " + std::to_string(id) + " from " + std::to_string(d.id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "CtrMve: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  
  ~eClass() {
    assert((id != DTR_ID_CODE || val != DTR_VAL_CODE) && "Error: double destruct");
    assert(id > 0 && id <= count && "Error: garbage destruct");
    if (!quiet) std::cout << "Dtr: " << id << " (val: " << val << ")" << std::endl;
    id  = DTR_ID_CODE;
    val = DTR_VAL_CODE;
    ++decount;
  }
  
  eClass& operator=(const eClass& d)
  {
    assert(id != d.id);
    bool failure = AsgCpy && WaitCount < ++countAsgCpy;
    if (failure)
      throw std::runtime_error("AsgCpy: "  + std::to_string(d.id)  + " overrides " + std::to_string(id)
                               + " (val: " + std::to_string(d.val) + " overrides " + std::to_string(val) + ")");
    if (!quiet)
      std::cout << "AsgCpy: " << d.id << " overrides " << id
                << " (val: " << d.val << " overrides " << val << ")" << std::endl;
    val = d.val;
    return *this;
  }
  eClass& operator=(eClass&& d)
  {
    assert(id != d.id);
    bool failure = AsgMve && WaitCount < ++countAsgMve;
    if (failure)
      throw std::runtime_error("AsgMve: "  + std::to_string(d.id)  + " overrides " + std::to_string(id)
                               + " (val: " + std::to_string(d.val) + " overrides " + std::to_string(val) + ")");
    if (!quiet)
      std::cout << "AsgMve: " << d.id << " overrides " << id
                << " (val: " << d.val << " overrides " << val << ")" << std::endl;
    val = d.val;
    return *this;
  }
  
  std::string toString() const { return std::to_string(id) + " (val: " + std::to_string(val) + ")\n"; }
  
  static void resetOpCounters() { countCtr0 = countCtr1 = countCtrCpy = countCtrMve = countAsgCpy = countAsgMve = 0; }
  
  
  // Members
  int val;
  int id;
  static int countCtr0, countCtr1, countCtrCpy, countCtrMve, countAsgCpy, countAsgMve;
  static int count;
  static int decount;
  static bool quiet;
};

// Init
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::countCtr0   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::countCtr1   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::countCtrCpy   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::countCtrMve   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::countAsgCpy   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::countAsgMve   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::count   = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
int eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::decount = 0;
template<bool Ctr0, bool Ctr1, bool CtrCpy, bool CtrMve, bool AsgCpy, bool AsgMve, int WaitCount>
bool eClass<Ctr0, Ctr1, CtrCpy, CtrMve, AsgCpy, AsgMve, WaitCount>::quiet  = true;

// Helpers (throw on 2nd operation)
typedef eClass<true, false, false, false, false, false, 1> eClassCtr0;
typedef eClass<false, true, false, false, false, false, 1> eClassCtr1;
typedef eClass<false, false, true, false, false, false, 1> eClassCtrCpy;
typedef eClass<false, false, false, true, false, false, 1> eClassCtrMve;
typedef eClass<false, false, false, false, true, false, 1> eClassAsgCpy;
typedef eClass<false, false, false, false, false, true, 1> eClassAsgMve;


// Test Allocator
template <typename T>
struct TestAllocator
{
  using value_type = T;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;

  int id = 0;
  size_t* alloc_count = nullptr;
  size_t* dealloc_count = nullptr;

  TestAllocator() = default;
  TestAllocator(int id, size_t* alloc = nullptr, size_t* dealloc = nullptr)
    : id(id), alloc_count(alloc), dealloc_count(dealloc)
  {}

  template <typename U>
  TestAllocator(const TestAllocator<U>& other) noexcept
      : id(other.id), alloc_count(other.alloc_count), dealloc_count(other.dealloc_count) {}

  T* allocate(size_t n) {
    if (alloc_count) ++(*alloc_count);
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, size_t) noexcept {
    if (dealloc_count) ++(*dealloc_count);
    ::operator delete(p);
  }

  friend bool operator==(const TestAllocator& a, const TestAllocator& b) noexcept {
    return a.id == b.id;
  }
  friend bool operator!=(const TestAllocator& a, const TestAllocator& b) noexcept {
    return !(a == b);
  }
};

template <typename T>
struct NonPropagatingAllocator : public TestAllocator<T>
{
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::false_type;

  using TestAllocator<T>::TestAllocator;

  template <typename U>
  NonPropagatingAllocator(const NonPropagatingAllocator<U>& other) noexcept
    : TestAllocator<T>(other)
  {}
};

template <typename T>
struct ThrowingAllocator : public TestAllocator<T>
{
  bool* should_throw = nullptr;

  using TestAllocator<T>::TestAllocator;

  ThrowingAllocator(int id, size_t* alloc, size_t* dealloc, bool* throw_flag)
    : TestAllocator<T>(id, alloc, dealloc), should_throw(throw_flag)
  {}

  template <typename U>
  ThrowingAllocator(const ThrowingAllocator<U>& other) noexcept
      : TestAllocator<T>(other), should_throw(other.should_throw) {}

  T* allocate(size_t n) {
    if (should_throw && *should_throw) {
      throw std::bad_alloc();
    }
    return TestAllocator<T>::allocate(n);
  }
};

template <typename T>
struct SocccAllocator : public TestAllocator<T>
{
  using TestAllocator<T>::TestAllocator;

  template <typename U>
  SocccAllocator(const SocccAllocator<U>& other) noexcept : TestAllocator<T>(other) {}

  // Override standard copy selection to assign a specific ID (999) to the copied container's allocator
  SocccAllocator select_on_container_copy_construction() const {
    SocccAllocator copy(*this);
    copy.id = 999;
    return copy;
  }
};

template <typename T>
struct MinimalAllocator
{
  using value_type = T;

  MinimalAllocator() = default;

  template <typename U>
  MinimalAllocator(const MinimalAllocator<U>&) noexcept {}

  T* allocate(size_t n) {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, size_t) noexcept {
    ::operator delete(p);
  }

  friend bool operator==(const MinimalAllocator&, const MinimalAllocator&) noexcept { return true; }
  friend bool operator!=(const MinimalAllocator&, const MinimalAllocator&) noexcept { return false; }
};

template <typename T>
struct CustomConstructAllocator : public TestAllocator<T>
{
  size_t* construct_count = nullptr;
  size_t* destroy_count = nullptr;

  using TestAllocator<T>::TestAllocator;

  CustomConstructAllocator(int id, size_t* c, size_t* d)
    : TestAllocator<T>(id), construct_count(c), destroy_count(d)
  {}

  template <typename U>
  CustomConstructAllocator(const CustomConstructAllocator<U>& other) noexcept
      : TestAllocator<T>(other),
      construct_count(other.construct_count),
      destroy_count(other.destroy_count) {}

  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    if (construct_count) ++(*construct_count);
    ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
  }

  template <typename U>
  void destroy(U* p) {
    if (destroy_count) ++(*destroy_count);
    p->~U();
  }
};

template <typename T>
struct AlwaysEqualAllocator : public TestAllocator<T>
{
  using is_always_equal = std::true_type;
  using propagate_on_container_move_assignment = std::false_type; // Should be bypassed because is_always_equal is true

  using TestAllocator<T>::TestAllocator;

  template <typename U>
  AlwaysEqualAllocator(const AlwaysEqualAllocator<U>& other) noexcept : TestAllocator<T>(other) {}
};

#endif // DEBUG_UTILS_H
