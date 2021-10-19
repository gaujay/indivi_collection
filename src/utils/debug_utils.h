/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 *
 */

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <string>

#define INIT_CODE 9876  // arbitrary


// Debug Class
class dClass
{
public:
  dClass()                : val(-1),    id(++count), _init(INIT_CODE) {
    if (!quiet) std::cout << "Ctr0: "   << id << " (val: " << val << ")" << std::endl;
  }
  dClass(int v)           : val(v),     id(++count), _init(INIT_CODE) {
    if (!quiet) std::cout << "Ctr1: "   << id << " (val: " << val << ")" << std::endl;
  }
  dClass(const dClass& d) : val(d.val), id(++count), _init(INIT_CODE) {
    assert(id != d.id);
    assert(d._init == INIT_CODE);
    if (!quiet) std::cout << "CtrCpy: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  dClass(dClass&& d)      : val(d.val), id(++count), _init(INIT_CODE) {
    assert(id != d.id);
    assert(d._init == INIT_CODE);
    if (!quiet) std::cout << "CtrMve: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  
  ~dClass() {
    assert((id != -1234 || val != -4321 || _init != -INIT_CODE) && "Error: double destruct");
    assert(id > 0 && id <= count && _init == INIT_CODE && "Error: garbage destruct");
    if (!quiet) std::cout << "Dtr: " << id << " (val: " << val << ")" << std::endl;
    id  = -1234; // arbitrary
    val = -4321;
    _init = -INIT_CODE;
    ++decount;
  }
  
  dClass& operator=(const dClass& d)
  {
//    assert(id != d.id);
    assert(  _init == INIT_CODE);
    assert(d._init == INIT_CODE);
    if (!quiet)
      std::cout << "AssignCpy: " << d.id << " overrides " << id
                << " (val: " << d.val << " overrides " << val << ")" << std::endl;
    val = d.val;
    return *this;
  }
  dClass& operator=(dClass&& d)
  {
//    assert(id != d.id);
    assert(  _init == INIT_CODE);
    assert(d._init == INIT_CODE);
    if (!quiet)
      std::cout << "AssignMve: " << d.id << " overrides " << id
                << " (val: " << d.val << " overrides " << val << ")" << std::endl;
    val = d.val;
    return *this;
  }
  
  std::string toString() const { return std::to_string(id) + " (val: " + std::to_string(val) + ")\n"; }
  
  
  // Members
  int val;
  int id;
  int _init;
  static int count;
  static int decount;
  static bool quiet;
};

// Init
int dClass::count   = 0;
int dClass::decount = 0;
bool dClass::quiet  = true;

// Non-member functions
bool operator==(const dClass& lhs, const dClass& rhs)
{
  return lhs.val == rhs.val;
}


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
    if (Ctr1 && WaitCount < ++countCtr1) {
      --count;
      throw std::runtime_error("Ctr1: " + std::to_string(id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "Ctr1: "   << id << " (val: " << val << ")" << std::endl;
  }
  eClass(const eClass& d) : val(d.val), id(++count) {
    assert(id != d.id);
    if (CtrCpy && WaitCount < ++countCtrCpy) {
      --count;
      throw std::runtime_error("CtrCpy: " + std::to_string(id) + " from " + std::to_string(d.id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "CtrCpy: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  eClass(eClass&& d)      : val(d.val), id(++count) {
    assert(id != d.id);
    if (CtrMve && WaitCount < ++countCtrMve) {
      --count;
      throw std::runtime_error("CtrMve: " + std::to_string(id) + " from " + std::to_string(d.id) + " (val: " + std::to_string(val) + ")");
    }
    if (!quiet) std::cout << "CtrMve: " << id << " from " << d.id << " (val: " << val << ")" << std::endl;
  }
  
  ~eClass() {
    assert((id != -1234 || val != -4321) && "Error: double destruct");
    assert(id > 0 && id <= count && "Error: garbage destruct");
    if (!quiet) std::cout << "Dtr: " << id << " (val: " << val << ")" << std::endl;
    id  = -1234; // random
    val = -4321;
    ++decount;
  }
  
  eClass& operator=(const eClass& d)
  {
    assert(id != d.id);
    if (AsgCpy && WaitCount < ++countAsgCpy)
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
    if (AsgMve && WaitCount < ++countAsgMve)
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


#endif // DEBUG_UTILS_H
