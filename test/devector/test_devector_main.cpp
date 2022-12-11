/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 *
 */

#include "gtest/gtest.h"

#include "utils/debug_utils.h"
#include "utils/bump_allocator.h"
#include "indivi/devector.h"

#include <cmath>
#include <memory>

#define CHECK_OFFSET  // for devector<T, NEAR, START>

using namespace indivi;
typedef devector<dClass, devector_opt<>, bump_allocator<dClass>> devector_alc;

static size_t next_capacity(size_t capacity, size_t minimum, float growth_factor = devector_opt<>::GROWTH_FACTOR())
{
  while (capacity < minimum)
    capacity = (size_t)(::ceilf((float)capacity * growth_factor));
  
  return capacity;
}


TEST(DevectorTest, Constructor)
{
  {
    //
    devector<dClass> v0(2, 1);
    EXPECT_FALSE(v0.empty());
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v0[0].val, 1);
    EXPECT_EQ(v0[1].val, 1);
    
    //
    devector<dClass> v1(v0);
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.size(), v0.size());
    EXPECT_EQ(v1[1].val, 1);
    
    //
    auto pv2 = std::unique_ptr<devector<dClass>>(new devector<dClass>(2, 10));
    devector<dClass> v2(std::forward<devector<dClass>>(*pv2));
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[1].val, 10);
    
    //
    devector<dClass> v3({1,2,3});
    EXPECT_EQ(v3.size(), 3);
    EXPECT_EQ(v3[0].val, 1);
    EXPECT_EQ(v3[2].val, 3);
    
    //
    devector<dClass> v4;
    EXPECT_EQ(v4.size(), 0);
    EXPECT_EQ(v4.capacity(), 0);
    EXPECT_EQ(v4.offset(), 0);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Constructor_Allocator)
{
  {
    //
    devector_alc v0(2, 1);
    EXPECT_FALSE(v0.empty());
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v0[0].val, 1);
    EXPECT_EQ(v0[1].val, 1);
    
    //
    devector_alc v1(v0);
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.size(), v0.size());
    EXPECT_EQ(v1[1].val, 1);
    
    //
    auto pv2 = std::unique_ptr<devector_alc>(new devector_alc(2, 10));
    devector_alc v2(std::forward<devector_alc>(*pv2));
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[1].val, 10);
    
    //
    devector_alc v3({1,2,3});
    EXPECT_EQ(v3.size(), 3);
    EXPECT_EQ(v3[0].val, 1);
    EXPECT_EQ(v3[2].val, 3);
    
    //
    devector_alc v4;
    EXPECT_EQ(v4.size(), 0);
    EXPECT_EQ(v4.capacity(), 0);
    EXPECT_EQ(v4.offset(), 0);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Assignment)
{
  {
    //
    devector<dClass> v0(2, 1);
    devector<dClass> v1(1, 3);
    EXPECT_EQ(v1.size(), 1);
    v1 = v0;
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.size(), v0.size());
    EXPECT_EQ(v1[1].val, v0[1].val);
    
    //
    devector<dClass> v2(3);
    EXPECT_EQ(v2.size(), 3);
    v2 = v1;
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2.size(), v0.size());
    EXPECT_EQ(v2[1].val, v0[1].val);
    
    //
    auto pv3 = std::unique_ptr<devector<dClass>>(new devector<dClass>(1, 11));
    devector<dClass> v3(2, -11);
    v3 = std::forward<devector<dClass>>(*pv3);
    EXPECT_EQ(v3.size(), 1);
    EXPECT_EQ(v3[0].val, 11);
    
    //
    devector<dClass> v4(1, 13);
    v4 = {1,2,3};
    EXPECT_EQ(v4.size(), 3);
    EXPECT_EQ(v4[0].val, 1);
    EXPECT_EQ(v4[2].val, 3);
    
    //
    devector<dClass> v5(3, 14);
    v5 = {1,2};
    EXPECT_EQ(v5.size(), 2);
    EXPECT_EQ(v5[0].val, 1);
    EXPECT_EQ(v5[1].val, 2);
    
    //
    devector<dClass> v6(3, 14);
    devector<dClass> v7;
    v6 = v7;
    EXPECT_EQ(v6.size(), 0);
    EXPECT_EQ(v6.capacity(), 3);
    
    devector<dClass> v8;
    v8.reserve(2);
    v6 = v8;
    EXPECT_EQ(v6.size(), 0);
    EXPECT_EQ(v6.capacity(), 3);
    
    v7 = v8;
    EXPECT_EQ(v7.size(), 0);
    EXPECT_EQ(v7.capacity(), 0);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Assignment_Allocator)
{
  {
    //
    devector_alc v0(2, 1);
    devector_alc v1(1, 3);
    EXPECT_EQ(v1.size(), 1);
    v1 = v0;
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.size(), v0.size());
    EXPECT_EQ(v1[1].val, v0[1].val);
    
    //
    devector_alc v2(3);
    EXPECT_EQ(v2.size(), 3);
    v2 = v1;
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2.size(), v0.size());
    EXPECT_EQ(v2[1].val, v0[1].val);
    
    //
    auto pv3 = std::unique_ptr<devector_alc>(new devector_alc(1, 11));
    devector_alc v3(2, -11);
    v3 = std::forward<devector_alc>(*pv3);
    EXPECT_EQ(v3.size(), 1);
    EXPECT_EQ(v3[0].val, 11);
    
    //
    devector_alc v4(1, 13);
    v4 = {1,2,3};
    EXPECT_EQ(v4.size(), 3);
    EXPECT_EQ(v4[0].val, 1);
    EXPECT_EQ(v4[2].val, 3);
    
    //
    devector_alc v5(3, 14);
    v5 = {1,2};
    EXPECT_EQ(v5.size(), 2);
    EXPECT_EQ(v5[0].val, 1);
    EXPECT_EQ(v5[1].val, 2);
    
    //
    devector_alc v6(3, 14);
    devector_alc v7;
    v6 = v7;
    EXPECT_EQ(v6.size(), 0);
    EXPECT_EQ(v6.capacity(), 3);
    
    devector_alc v8;
    v8.reserve(2);
    v6 = v8;
    EXPECT_EQ(v6.size(), 0);
    EXPECT_EQ(v6.capacity(), 3);
    
    v7 = v8;
    EXPECT_EQ(v7.size(), 0);
    EXPECT_EQ(v7.capacity(), 0);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Assign)
{
  {
    //
    devector<dClass> v0(1);
    v0.assign(2, 18);
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v0[1].val, 18);
    
    //
    devector<dClass> v1(3);
    v1.assign(2, -18);
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1[0].val, -18);
    
    //
    devector<dClass> v2(3);
    v2.assign({19,-19});
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[1].val, -19);
    
    //
    devector<dClass> v3(3);
    v3.pop_front();
    v3.assign(v2.begin(), v2.end());
    EXPECT_EQ(v3.size(), 2);
    EXPECT_EQ(v3[1].val, -19);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, ElementAccess)
{
  {
    //
    devector<dClass> v0(2, 1);
    EXPECT_FALSE(v0.empty());
    EXPECT_EQ(v0.size(), 2);
    
    EXPECT_EQ(v0[0].val, 1);
    EXPECT_EQ(v0[1].val, 1);
    v0[1].val = 2;
    EXPECT_EQ(v0[1].val, 2);
    
    //
    devector<dClass> v1(1, 15);
    EXPECT_EQ(v1.at(0).val, 15);
    EXPECT_THROW(v1.at(1), std::out_of_range);
    
    //
    devector<dClass> v2({16,-16});
    EXPECT_EQ(v2.front().val, 16);
    EXPECT_EQ(v2.back().val, -16);
    
    //
    devector<dClass> v3({17,-17});
    EXPECT_EQ(v3.data()[0].val,  17);
    EXPECT_EQ(v3.data()[1].val, -17);
    
    //
    devector<dClass> v4;
    EXPECT_EQ(v4.data(), nullptr);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Capacity)
{
  {
    //
    devector<dClass> v0;
    EXPECT_TRUE(v0.empty());
    EXPECT_EQ(v0.size(), 0);
    EXPECT_EQ(v0.capacity(), 0);
    v0.reserve(2);
    EXPECT_TRUE(v0.empty());
    EXPECT_EQ(v0.size(), 0);
    EXPECT_EQ(v0.capacity(), 2);
    
    //
    devector<dClass> v1(1, 6);
    v1.reserve(2);
    EXPECT_EQ(v1.size(), 1);
    EXPECT_EQ(v1[0].val, 6);
    
    //
    devector<int> v2({1,2});
    v2.reserve(3);
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2.capacity(), 3);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
    
    //
    devector<dClass> v3;
    v3.reserve(2);
    EXPECT_EQ(v3.capacity(), 2);
    v3.shrink_to_fit();
    EXPECT_EQ(v3.capacity(), 0);
    
    //
    devector<dClass> v4(1);
    v4.reserve(2);
    EXPECT_EQ(v4.capacity(), 2);
    v4.shrink_to_fit();
    EXPECT_EQ(v4.size(), 1);
    EXPECT_EQ(v4.capacity(), 1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Resize)
{
  {
    //
    devector<dClass> v0(3, 7);
    v0.resize(1);
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0[0].val, 7);
    
    //
    devector<dClass> v1(1, 8);
    v1.resize(3);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1.capacity(), 3);
    EXPECT_EQ(v1[0].val, 8);
    EXPECT_EQ(v1[1].val, -1);
    EXPECT_EQ(v1[2].val, -1);
    auto it = v1.erase(v1.begin());
    EXPECT_EQ(v1.offset(), 1);
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.capacity(), 3);
    EXPECT_EQ(v1[0].val, -1);
    EXPECT_EQ(v1[1].val, -1);
    EXPECT_EQ(it, v1.begin());
    v1.resize(3,8);
    EXPECT_EQ(v1.offset(), 0);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1.capacity(), 3);
    EXPECT_EQ(v1[0].val, -1);
    EXPECT_EQ(v1[1].val, -1);
    EXPECT_EQ(v1[2].val,  8);
    v1.resize(0,9);
  #ifdef CHECK_OFFSET
    EXPECT_EQ(v1.offset(), 0);
  #endif
    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 3);
    
    //
    devector<dClass> v2;
    v2.resize(0);
    EXPECT_EQ(v2.size(), 0);
    v2.resize(1);
    EXPECT_EQ(v2.size(), 1);
    v2.reserve(2);
    EXPECT_EQ(v2.size(), 1);
    EXPECT_EQ(v2.capacity(), 2);
    v2.resize(2);
    EXPECT_EQ(v2.size(), 2);
    v2.resize(0);
    EXPECT_EQ(v2.size(), 0);
    
    //
    devector<dClass> v3(1, 9);
    v3.resize(2, 1);
    EXPECT_EQ(v3.size(), 2);
    EXPECT_EQ(v3[0].val, 9);
    EXPECT_EQ(v3[1].val, 1);
    
    //
    devector<dClass> v4(3,1);
    v4.resize(3);
    EXPECT_EQ(v4.size(), 3);
    EXPECT_EQ(v4.capacity(), 3);
    EXPECT_EQ(v4[0].val, 1);
    
    //
    devector<dClass> v5(3,0);
    v5.clear();
    v5.resize(3);
    EXPECT_EQ(v5.size(), 3);
    EXPECT_EQ(v5.capacity(), 3);
    EXPECT_EQ(v5[0].val, -1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Clear)
{
  {
    //
    devector<dClass> v0(1);
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0[0].val, -1);
    v0.clear();
    EXPECT_TRUE(v0.empty());
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Iterator)
{
  {
    //
    devector<dClass> v0({0,1,2});
    int i = 0;
    for (auto it = v0.begin(); it != v0.end(); ++it, ++i)
      EXPECT_EQ(it->val, i);
    i = 2;
    for (auto it = v0.rbegin(); it != v0.rend(); ++it, --i)
      EXPECT_EQ(it->val, i);
    EXPECT_EQ(v0.cbegin()->val,   (v0.crend()-1)->val);
    EXPECT_EQ((v0.cend()-1)->val, (v0.crbegin())->val);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, BackModifier)
{
  {
    //
    devector<dClass> v0;
    v0.push_back(22);
    size_t capacity = 1;
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0.capacity(), capacity);
    v0.push_back(23);
    v0.push_back(24);
    capacity = next_capacity(capacity, 3);
    EXPECT_EQ(v0.size(), 3);
    EXPECT_EQ(v0.capacity(), capacity);
    EXPECT_EQ(v0[0].val, 22);
    EXPECT_EQ(v0[2].val, 24);
    v0.push_back(25);
    capacity = next_capacity(capacity, 4);
    EXPECT_EQ(v0.size(), 4);
    EXPECT_EQ(v0.capacity(), capacity);
    EXPECT_EQ(v0[0].val, 22);
    EXPECT_EQ(v0[3].val, 25);
    
    //
    devector<dClass> v1;
    dClass d23(23);
    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    capacity = 1;
    v1.push_back(d23);
    v1.push_back(d23);
    capacity = next_capacity(capacity, 2);
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.capacity(), capacity);
    
    //
    devector<dClass> v2(1, 24);
    v2.pop_back();
    EXPECT_EQ(v2.size(), 0);
    EXPECT_EQ(v2.capacity(), 1);
    v2 = {24,-24};
    v2.pop_back();
    EXPECT_EQ(v2.size(), 1);
    EXPECT_EQ(v2.capacity(), 2);
    EXPECT_EQ(v2[0].val, 24);
  
    //
    devector<dClass> v3;
    v3.emplace_back(25);
    capacity = 1;
    EXPECT_EQ(v3.size(), 1);
    EXPECT_EQ(v3.capacity(), capacity);
    v3.emplace_back(26);
    capacity = next_capacity(capacity, 2);
    EXPECT_EQ(v3.size(), 2);
    EXPECT_EQ(v3.capacity(), capacity);
    EXPECT_EQ(v3[0].val, 25);
    EXPECT_EQ(v3[1].val, 26);
    
    //
    devector<dClass> v4;
    v4.reserve(3);
    v4.emplace_back(26);
    EXPECT_EQ(v4.size(), 1);
    EXPECT_EQ(v4.capacity(), 3);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, FrontModifier)
{
  {
    //
    devector<dClass> v0;
    v0.push_front(10);
    size_t capacity = 1;
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0.capacity(), capacity);
    v0.push_front(11);
    v0.push_front(12);
    capacity = next_capacity(capacity, 3);
    EXPECT_EQ(v0.size(), 3);
    EXPECT_EQ(v0.capacity(), capacity);
    EXPECT_EQ(v0[0].val, 12);
    EXPECT_EQ(v0[1].val, 11);
    EXPECT_EQ(v0[2].val, 10);
    v0.push_front(13);
    capacity = next_capacity(capacity, 4);
    EXPECT_EQ(v0.size(), 4);
    EXPECT_EQ(v0.capacity(), capacity);
    EXPECT_EQ(v0[0].val, 13);
    EXPECT_EQ(v0[3].val, 10);
    
    //
    devector<dClass> v1;
    dClass d23(23);
    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    capacity = 1;
    v1.push_front(d23);
    v1.push_front(d23);
    capacity = next_capacity(capacity, 2);
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1.capacity(), capacity);
    v1 = {1,2,3};
    v1.shrink_to_fit();
    v1.pop_back();
    v1.pop_back();
    v1.push_front(2);
    v1.push_front(1);
    EXPECT_EQ(v1[1].val, 2);
    
    //
    devector<dClass> v2(1, 24);
    v2.pop_front();
    EXPECT_EQ(v2.size(), 0);
    EXPECT_EQ(v2.capacity(), 1);
    v2 = {24,-24};
    v2.pop_front();
    EXPECT_EQ(v2.size(), 1);
    EXPECT_EQ(v2.capacity(), 2);
    EXPECT_EQ(v2[0].val, -24);
    
    //
    devector<dClass> v3;
    v3.emplace_front(25);
    capacity = 1;
    EXPECT_EQ(v3.size(), 1);
    EXPECT_EQ(v3.capacity(), capacity);
    v3.emplace_front(26);
    capacity = next_capacity(capacity, 2);
    EXPECT_EQ(v3.size(), 2);
    EXPECT_EQ(v3.capacity(), capacity);
    EXPECT_EQ(v3[0].val, 26);
    EXPECT_EQ(v3[1].val, 25);
    
    //
    devector<dClass> v4;
    v4.reserve(3);
    v4.emplace_front(26);
    EXPECT_EQ(v4.size(), 1);
    EXPECT_EQ(v4.capacity(), 3);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Erase)
{
  {
    //
    devector<dClass> v0 = {1,2,3,4};
    auto it = v0.erase(v0.begin() + 1); //1,3,4
    EXPECT_EQ(v0.size(), 3);
    EXPECT_EQ(v0[0].val, 1);
    EXPECT_EQ(v0[1].val, 3);
    EXPECT_EQ(it, v0.begin()+1);
    
    it = v0.erase(v0.begin());     //3,4
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v0[0].val, 3);
    EXPECT_EQ(v0[1].val, 4);
    EXPECT_EQ(it, v0.begin());
    
    it = v0.erase(v0.begin() + 1); //3
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0[0].val, 3);
    EXPECT_EQ(it, v0.begin()+1);
    
    it = v0.erase(v0.begin());     //nil
    EXPECT_EQ(v0.size(), 0);
    EXPECT_EQ(it, v0.begin());
    
    //
    devector<dClass> v1 = {1,2,3,4};
    it = v1.erase(v1.end() - 1); //1,2,3
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1[0].val, 1);
    EXPECT_EQ(v1[2].val, 3);
    EXPECT_EQ(it, v1.end());
    
    it = v1.erase(v1.end() - 2); //1,3
    EXPECT_EQ(v1.size(), 2);
    EXPECT_EQ(v1[0].val, 1);
    EXPECT_EQ(v1[1].val, 3);
    EXPECT_EQ(it, v1.end() - 1);
    
    //
    devector<dClass> v2({1,2,3});
    it = v2.erase(v2.begin(), v2.end());
    EXPECT_EQ(v2.size(), 0);
  #ifdef CHECK_OFFSET
    EXPECT_EQ(v2.offset(), 0);
  #endif
    EXPECT_EQ(it, v2.begin());
    EXPECT_EQ(it, v2.end());
    
    //
    devector<dClass> v3({1,2,3,4});
    it = v3.erase(v3.begin(), v3.end()-1);
    EXPECT_EQ(v3.size(), 1);
    EXPECT_EQ(v3.offset(), 3);
    EXPECT_EQ(it, v3.begin());
    EXPECT_EQ(it, v3.end()-1);
    EXPECT_EQ(v3[0].val, 4);
    
    //
    devector<dClass> v4({1,2,3,4,5,6});
    v4.pop_front();
    v4.pop_back();
    it = v4.erase(v4.begin()+1, v4.end()-1);
    EXPECT_EQ(v4.size(), 2);
    EXPECT_EQ(v4.offset(), 1);
    EXPECT_EQ(it, v4.begin()+1);
    EXPECT_EQ(it, v4.end()-1);
    EXPECT_EQ(v4[0].val, 2);
    EXPECT_EQ(v4[1].val, 5);
    
    //
    devector<dClass> v5({1,2,3,-4,-5,6});
    it = v5.erase(v5.begin()+3, v5.begin()+5);
    EXPECT_EQ(v5.size(), 4);
    EXPECT_EQ(v5.offset(), 0);
    EXPECT_EQ(it, v5.end()-1);
    
    //
    devector<dClass> v6({1,-2,-3,4,5,6});
    it = v6.erase(v6.begin()+1, v6.begin()+3);
    EXPECT_EQ(v6.size(), 4);
    EXPECT_EQ(v6.offset(), 2);
    EXPECT_EQ(it, v6.begin()+1);
    
    //
    devector<dClass> v7(1, 1);
    it = v7.erase(v7.begin(), v7.begin()+1);
    EXPECT_EQ(v7.size(), 0);
  #ifdef CHECK_OFFSET
    EXPECT_EQ(v7.offset(), 0);
  #endif
    EXPECT_EQ(it, v7.begin());
    
    //
    devector<dClass> v8({1,2,3,4,5});
    auto v8_2 = v8;
    it = v8.erase(v8.begin()+2, v8.begin()+2);
    EXPECT_EQ(v8.size(), 5);
    EXPECT_EQ(v8.offset(), 0);
    EXPECT_EQ(it, v8.begin()+2);
    EXPECT_EQ(v8, v8_2);
    
    //
    devector<dClass> v9;
    it = v9.erase(v9.begin(), v9.begin());
    EXPECT_EQ(v9.size(), 0);
    EXPECT_EQ(v9.offset(), 0);
    EXPECT_EQ(it, v9.begin());
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Insert)
{
  {
    //
    devector<dClass> v0;
    auto it = v0.insert(v0.begin(), 29);
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0[0].val, 29);
    EXPECT_EQ(it, v0.begin());
    
    it = v0.insert(v0.begin(), 28);
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v0[0].val, 28);
    EXPECT_EQ(v0[1].val, 29);
    EXPECT_EQ(it, v0.begin());
    
    it = v0.insert(v0.end(), 32);
    EXPECT_EQ(v0[2].val, 32);
    EXPECT_EQ(it, v0.end()-1);
    
    it = v0.insert(v0.end()-1, 31);
    EXPECT_EQ(v0[2].val, 31);
    EXPECT_EQ(it, v0.end()-2);
    
    v0.reserve(5);
    
    it = v0.insert(v0.end()-2, 30);
    EXPECT_EQ(v0[1].val, 29);
    EXPECT_EQ(v0[2].val, 30);
    EXPECT_EQ(v0[3].val, 31);
    EXPECT_EQ(v0[4].val, 32);
    EXPECT_EQ(it, v0.end()-3);
    
    v0.push_back(33);
    v0.push_back(34);
    it = v0.erase(v0.begin()+1);
    EXPECT_EQ(it, v0.begin()+1);
    
    it = v0.insert(v0.begin()+1, 35);
    EXPECT_EQ(v0[1].val, 35);
    EXPECT_EQ(it, v0.begin()+1);
    
    //
    devector<dClass> v1({1,2,3});
    it = v1.erase(v1.begin());
    EXPECT_EQ(it, v1.begin());
    it = v1.insert(v1.end(), 29);
    EXPECT_EQ(v1[0].val, 2);
    EXPECT_EQ(v1[1].val, 3);
    EXPECT_EQ(v1[2].val, 29);
    EXPECT_EQ(it, v1.end()-1);
    
    //
    devector<int> v0i;
    auto iti = v0i.insert(v0i.begin(), 29);
    EXPECT_EQ(v0i.size(), 1);
    EXPECT_EQ(v0i[0], 29);
    EXPECT_EQ(iti, v0i.begin());
    
    iti = v0i.insert(v0i.begin(), 28);
    EXPECT_EQ(v0i.size(), 2);
    EXPECT_EQ(v0i[0], 28);
    EXPECT_EQ(v0i[1], 29);
    EXPECT_EQ(iti, v0i.begin());
    
    iti = v0i.insert(v0i.end(), 32);
    EXPECT_EQ(v0i[2], 32);
    EXPECT_EQ(iti, v0i.end()-1);
    
    iti = v0i.insert(v0i.end()-1, 31);
    EXPECT_EQ(v0i[2], 31);
    EXPECT_EQ(iti, v0i.end()-2);
    
    v0i.reserve(5);
    
    iti = v0i.insert(v0i.end()-2, 30);
    EXPECT_EQ(v0i[1], 29);
    EXPECT_EQ(v0i[2], 30);
    EXPECT_EQ(v0i[3], 31);
    EXPECT_EQ(v0i[4], 32);
    EXPECT_EQ(iti, v0i.end()-3);
    
    v0i.push_back(33);
    v0i.push_back(34);
    iti = v0i.erase(v0i.begin()+1);
    EXPECT_EQ(iti, v0i.begin()+1);
    
    iti = v0i.insert(v0i.begin()+1, 35);
    EXPECT_EQ(v0i[1], 35);
    EXPECT_EQ(iti, v0i.begin()+1);
    
    //
    devector<int> v1i({1,2,3});
    iti = v1i.erase(v1i.begin());
    EXPECT_EQ(iti, v1i.begin());
    iti = v1i.insert(v1i.end(), 29);
    EXPECT_EQ(v1i[0], 2);
    EXPECT_EQ(v1i[1], 3);
    EXPECT_EQ(v1i[2], 29);
    EXPECT_EQ(iti, v1i.end()-1);
    
    //
    devector<dClass> v2({1,2,3});
    v2.pop_front();
    v2.pop_front();
    it = v2.insert(v2.end(), 37);
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2.offset(), 1);
    EXPECT_EQ(v2[1].val, 37);
    EXPECT_EQ(it, v2.end()-1);
    
    //
    devector<dClass> v3({1,2,3});
    v3.pop_front();
    v3.pop_front();
    it = v3.insert(v3.end(), 37);
    EXPECT_EQ(v3.size(), 2);
    EXPECT_EQ(v3.offset(), 1);
    EXPECT_EQ(v3[1].val, 37);
    EXPECT_EQ(it, v3.end()-1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, InsertN_Copy)
{
  {
    // Left-shift
    //
    devector<dClass> v0({1,2,3});
    v0.pop_front();
    auto it = v0.insert(v0.begin(), 1, 11);
    EXPECT_EQ(v0.size(), 3);
    EXPECT_EQ(v0.offset(), 0);
    EXPECT_EQ(v0[0].val, 11);
    EXPECT_EQ(v0[1].val, 2);
    EXPECT_EQ(it, v0.begin());
    it = v0.insert(v0.end(), 0, 22);
    EXPECT_EQ(it, v0.end());
    
    //
    devector<dClass> v1({1,2,3,4,5});
    v1.pop_front();
    it = v1.insert(v1.begin()+1, 1, 11);
    EXPECT_EQ(v1.size(), 5);
    EXPECT_EQ(v1.offset(), 0);
    EXPECT_EQ(v1[0].val, 2);
    EXPECT_EQ(v1[1].val, 11);
    EXPECT_EQ(v1[2].val, 3);
    EXPECT_EQ(it, v1.begin()+1);
    
    //
    devector<dClass> v2({1,2,3,4,5});
    v2.pop_front();
    v2.pop_front();
    it = v2.insert(v2.begin()+1, 2, 11);
    EXPECT_EQ(v2.size(), 5);
    EXPECT_EQ(v2.offset(), 0);
    EXPECT_EQ(v2[0].val, 3);
    EXPECT_EQ(v2[1].val, 11);
    EXPECT_EQ(v2[2].val, 11);
    EXPECT_EQ(v2[3].val, 4);
    EXPECT_EQ(it, v2.begin()+1);
    
    //
    devector<dClass> v3({1,2,3,4});
    v3.pop_front();
    v3.pop_front();
    v3.pop_back();
    it = v3.insert(v3.end(), 3, 11);
    EXPECT_EQ(v3.size(), 4);
    EXPECT_EQ(v3.offset(), 0);
    EXPECT_EQ(v3[0].val, 3);
    EXPECT_EQ(v3[1].val, 11);
    EXPECT_EQ(v3[3].val, 11);
    EXPECT_EQ(it, v3.begin()+1);
    
    //
    devector<dClass> v4({1,2,3,4});
    v4.pop_front();
    v4.pop_front();
    it = v4.insert(v4.end(), 2, 11);
    EXPECT_EQ(v4.size(), 4);
    EXPECT_EQ(v4.offset(), 0);
    EXPECT_EQ(v4[0].val, 3);
    EXPECT_EQ(v4[1].val, 4);
    EXPECT_EQ(v4[2].val, 11);
    EXPECT_EQ(v4[3].val, 11);
    EXPECT_EQ(it, v4.begin()+2);
    
    //
    devector<dClass> v5;
    v5.reserve(3);
    v5.shift_data_center();
    EXPECT_EQ(v5.offset(), 1);
    it = v5.insert(v5.begin(), 2, 11);
    EXPECT_EQ(v5.size(), 2);
    EXPECT_EQ(v5.offset(), 0);
    EXPECT_EQ(v5[0].val, 11);
    EXPECT_EQ(v5[1].val, 11);
    EXPECT_EQ(it, v5.begin());
    
    //
    devector<dClass> v6{-1,0,1,2,3,4};
    v6.pop_front();
    v6.pop_back();
    it = v6.insert(v6.begin()+2, 1, 11);
    EXPECT_EQ(v6.size(), 5);
    EXPECT_EQ(v6.offset(), 0);
    EXPECT_EQ(v6[0].val, 0);
    EXPECT_EQ(v6[1].val, 1);
    EXPECT_EQ(v6[2].val, 11);
    EXPECT_EQ(v6[3].val, 2);
    EXPECT_EQ(it, v6.begin()+2);
    
    //
    devector<dClass> v7{1,2,3,4};
    v7.pop_back();
    v7.pop_back();
    it = v7.insert(v7.end(), 2, 11);
    EXPECT_EQ(v7.size(), 4);
    EXPECT_EQ(v7.offset(), 0);
    EXPECT_EQ(v7[0].val, 1);
    EXPECT_EQ(v7[1].val, 2);
    EXPECT_EQ(v7[2].val, 11);
    EXPECT_EQ(v7[3].val, 11);
    EXPECT_EQ(it, v7.begin()+2);
    
    
    // Right-shift
    //
    devector<dClass> v8{1,2,3,4};
    v8.pop_back();
    it = v8.insert(v8.end()-1, 1, 11);
    EXPECT_EQ(v8.size(), 4);
    EXPECT_EQ(v8.offset(), 0);
    EXPECT_EQ(v8[0].val, 1);
    EXPECT_EQ(v8[1].val, 2);
    EXPECT_EQ(v8[2].val, 11);
    EXPECT_EQ(v8[3].val, 3);
    EXPECT_EQ(it, v8.end() - 2);
    it = v8.insert(v8.begin(), 0, 22);
    EXPECT_EQ(it, v8.begin());
    
    //
    devector<dClass> v9{0,1,2,3,4,5};
    v9.pop_back();
    it = v9.insert(v9.begin()+3, 1, 11);
    EXPECT_EQ(v9.size(), 6);
    EXPECT_EQ(v9.offset(), 0);
    EXPECT_EQ(v9[0].val, 0);
    EXPECT_EQ(v9[2].val, 2);
    EXPECT_EQ(v9[3].val, 11);
    EXPECT_EQ(v9[4].val, 3);
    EXPECT_EQ(it, v9.begin()+3);
    
    //
    devector<dClass> v10{0,1,2,3,4,5,6};
    v10.pop_front();
    v10.pop_front();
    v10.pop_back();
    v10.pop_back();
    v10.pop_back();
    it = v10.insert(v10.begin(), 3, 11);
    EXPECT_EQ(v10.size(), 5);
    EXPECT_EQ(v10.offset(), 2);
    EXPECT_EQ(v10[0].val, 11);
    EXPECT_EQ(v10[2].val, 11);
    EXPECT_EQ(v10[3].val, 2);
    EXPECT_EQ(v10[4].val, 3);
    EXPECT_EQ(it, v10.begin());
    
    //
    devector<dClass> v11{0,1,2,3};
    v11.pop_front();
    v11.pop_back();
    it = v11.insert(v11.begin(), 2, 11);
    EXPECT_EQ(v11.size(), 4);
    EXPECT_EQ(v11.offset(), 0);
    EXPECT_EQ(v11[0].val, 11);
    EXPECT_EQ(v11[1].val, 11);
    EXPECT_EQ(v11[2].val, 1);
    EXPECT_EQ(v11[3].val, 2);
    EXPECT_EQ(it, v11.begin());
    
    
    // Both-shift
    //
    devector<dClass> v20{1,2,3,4,5,6};
    v20.pop_front();
    v20.pop_back();
    it = v20.insert(v20.begin()+1, 2, 11);
    EXPECT_EQ(v20.size(), 6);
    EXPECT_EQ(v20.offset(), 0);
    EXPECT_EQ(v20[0].val, 2);
    EXPECT_EQ(v20[1].val, 11);
    EXPECT_EQ(v20[2].val, 11);
    EXPECT_EQ(v20[3].val, 3);
    EXPECT_EQ(v20[5].val, 5);
    EXPECT_EQ(it, v20.begin()+1);
    
    //
    devector<dClass> v21{1,2,3,4,5};
    v21.pop_front();
    v21.pop_back();
    v21.pop_back();
    it = v21.insert(v21.begin()+1, 3, 11);
    EXPECT_EQ(v21.size(), 5);
    EXPECT_EQ(v21.offset(), 0);
    EXPECT_EQ(v21[0].val, 2);
    EXPECT_EQ(v21[1].val, 11);
    EXPECT_EQ(v21[3].val, 11);
    EXPECT_EQ(v21[4].val, 3);
    EXPECT_EQ(it, v21.begin()+1);
    
    //
    devector<dClass> v22{1,2,3,4,5,6};
    v22.pop_front();
    v22.pop_front();
    v22.pop_back();
    v22.pop_back();
    it = v22.insert(v22.begin()+1, 3, 11);
    EXPECT_EQ(v22.size(), 5);
    EXPECT_EQ(v22.offset(), 0);
    EXPECT_EQ(v22[0].val, 3);
    EXPECT_EQ(v22[1].val, 11);
    EXPECT_EQ(v22[3].val, 11);
    EXPECT_EQ(v22[4].val, 4);
    EXPECT_EQ(it, v22.begin()+1);
    
    //
    devector<dClass> v23{1,2,3,4,5,6,7};
    v23.pop_front();
    v23.pop_back();
    it = v23.insert(v23.begin()+3, 2, 11);
    EXPECT_EQ(v23.size(), 7);
    EXPECT_EQ(v23.offset(), 0);
    EXPECT_EQ(v23[0].val, 2);
    EXPECT_EQ(v23[1].val, 3);
    EXPECT_EQ(v23[3].val, 11);
    EXPECT_EQ(v23[4].val, 11);
    EXPECT_EQ(v23[5].val, 5);
    EXPECT_EQ(v23[6].val, 6);
    EXPECT_EQ(it, v23.begin()+3);
    
    
    // Re-alloc
    //
    devector<dClass> v30;
    it = v30.insert(v30.begin(), 0, 22);
    EXPECT_EQ(it, v30.begin());
    EXPECT_EQ(v30.capacity(), 0);
    it = v30.insert(v30.begin(), 3, 11);
    EXPECT_EQ(v30.size(), 3);
    EXPECT_EQ(v30.offset(), 0);
    EXPECT_EQ(v30[0].val, 11);
    EXPECT_EQ(v30[2].val, 11);
    EXPECT_EQ(it, v30.begin());
    
    //
    devector<dClass> v31 = {1,2};
    it = v31.insert(v31.begin()+1, 2, 11);
    EXPECT_EQ(v31.size(), 4);
    EXPECT_EQ(v31.offset(), 0);
    EXPECT_EQ(v31[0].val, 1);
    EXPECT_EQ(v31[1].val, 11);
    EXPECT_EQ(v31[3].val, 2);
    EXPECT_EQ(it, v31.begin()+1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, InsertN_Range)
{
  {
    // Left-shift
    //
    devector<dClass> v0({1,2,3});
    v0.pop_front();
    auto it = v0.insert(v0.begin(), {11});
    EXPECT_EQ(v0.size(), 3);
    EXPECT_EQ(v0.offset(), 0);
    EXPECT_EQ(v0[0].val, 11);
    EXPECT_EQ(v0[1].val, 2);
    EXPECT_EQ(it, v0.begin());
    it = v0.insert(v0.end(), 0, 22);
    EXPECT_EQ(it, v0.end());
    
    //
    devector<dClass> v1({1,2,3,4,5});
    v1.pop_front();
    it = v1.insert(v1.begin()+1, {11});
    EXPECT_EQ(v1.size(), 5);
    EXPECT_EQ(v1.offset(), 0);
    EXPECT_EQ(v1[0].val, 2);
    EXPECT_EQ(v1[1].val, 11);
    EXPECT_EQ(v1[2].val, 3);
    EXPECT_EQ(it, v1.begin()+1);
    
    //
    devector<dClass> v2({1,2,3,4,5});
    v2.pop_front();
    v2.pop_front();
    it = v2.insert(v2.begin()+1, {11, 11});
    EXPECT_EQ(v2.size(), 5);
    EXPECT_EQ(v2.offset(), 0);
    EXPECT_EQ(v2[0].val, 3);
    EXPECT_EQ(v2[1].val, 11);
    EXPECT_EQ(v2[2].val, 11);
    EXPECT_EQ(v2[3].val, 4);
    EXPECT_EQ(it, v2.begin()+1);
    
    //
    devector<dClass> v3({1,2,3,4});
    v3.pop_front();
    v3.pop_front();
    v3.pop_back();
    it = v3.insert(v3.end(), {11, 11, 11});
    EXPECT_EQ(v3.size(), 4);
    EXPECT_EQ(v3.offset(), 0);
    EXPECT_EQ(v3[0].val, 3);
    EXPECT_EQ(v3[1].val, 11);
    EXPECT_EQ(v3[3].val, 11);
    EXPECT_EQ(it, v3.begin()+1);
    
    //
    devector<dClass> v4({1,2,3,4});
    v4.pop_front();
    v4.pop_front();
    it = v4.insert(v4.end(), {11, 11});
    EXPECT_EQ(v4.size(), 4);
    EXPECT_EQ(v4.offset(), 0);
    EXPECT_EQ(v4[0].val, 3);
    EXPECT_EQ(v4[1].val, 4);
    EXPECT_EQ(v4[2].val, 11);
    EXPECT_EQ(v4[3].val, 11);
    EXPECT_EQ(it, v4.begin()+2);
    
    //
    devector<dClass> v5;
    v5.reserve(3);
    v5.shift_data_center();
    EXPECT_EQ(v5.offset(), 1);
    it = v5.insert(v5.begin(), {11, 11});
    EXPECT_EQ(v5.size(), 2);
    EXPECT_EQ(v5.offset(), 0);
    EXPECT_EQ(v5[0].val, 11);
    EXPECT_EQ(v5[1].val, 11);
    EXPECT_EQ(it, v5.begin());
    
    //
    devector<dClass> v6{-1,0,1,2,3,4};
    v6.pop_front();
    v6.pop_back();
    it = v6.insert(v6.begin()+2, {11});
    EXPECT_EQ(v6.size(), 5);
    EXPECT_EQ(v6.offset(), 0);
    EXPECT_EQ(v6[0].val, 0);
    EXPECT_EQ(v6[1].val, 1);
    EXPECT_EQ(v6[2].val, 11);
    EXPECT_EQ(v6[3].val, 2);
    EXPECT_EQ(it, v6.begin()+2);
    
    //
    devector<dClass> v7{1,2,3,4};
    v7.pop_back();
    v7.pop_back();
    it = v7.insert(v7.end(), {11, 11});
    EXPECT_EQ(v7.size(), 4);
    EXPECT_EQ(v7.offset(), 0);
    EXPECT_EQ(v7[0].val, 1);
    EXPECT_EQ(v7[1].val, 2);
    EXPECT_EQ(v7[2].val, 11);
    EXPECT_EQ(v7[3].val, 11);
    EXPECT_EQ(it, v7.begin()+2);
    
    
    // Right-shift
    //
    devector<dClass> v8{1,2,3,4};
    v8.pop_back();
    it = v8.insert(v8.end()-1, {11});
    EXPECT_EQ(v8.size(), 4);
    EXPECT_EQ(v8.offset(), 0);
    EXPECT_EQ(v8[0].val, 1);
    EXPECT_EQ(v8[1].val, 2);
    EXPECT_EQ(v8[2].val, 11);
    EXPECT_EQ(v8[3].val, 3);
    EXPECT_EQ(it, v8.end() - 2);
    it = v8.insert(v8.begin(), 0, 22);
    EXPECT_EQ(it, v8.begin());
    
    //
    devector<dClass> v9{0,1,2,3,4,5};
    v9.pop_back();
    it = v9.insert(v9.begin()+3, {11});
    EXPECT_EQ(v9.size(), 6);
    EXPECT_EQ(v9.offset(), 0);
    EXPECT_EQ(v9[0].val, 0);
    EXPECT_EQ(v9[2].val, 2);
    EXPECT_EQ(v9[3].val, 11);
    EXPECT_EQ(v9[4].val, 3);
    EXPECT_EQ(it, v9.begin()+3);
    
    //
    devector<dClass> v10{0,1,2,3,4,5,6};
    v10.pop_front();
    v10.pop_front();
    v10.pop_back();
    v10.pop_back();
    v10.pop_back();
    it = v10.insert(v10.begin(), {11, 11, 11});
    EXPECT_EQ(v10.size(), 5);
    EXPECT_EQ(v10.offset(), 2);
    EXPECT_EQ(v10[0].val, 11);
    EXPECT_EQ(v10[2].val, 11);
    EXPECT_EQ(v10[3].val, 2);
    EXPECT_EQ(v10[4].val, 3);
    EXPECT_EQ(it, v10.begin());
    
    //
    devector<dClass> v11{0,1,2,3};
    v11.pop_front();
    v11.pop_back();
    it = v11.insert(v11.begin(), {11, 11});
    EXPECT_EQ(v11.size(), 4);
    EXPECT_EQ(v11.offset(), 0);
    EXPECT_EQ(v11[0].val, 11);
    EXPECT_EQ(v11[1].val, 11);
    EXPECT_EQ(v11[2].val, 1);
    EXPECT_EQ(v11[3].val, 2);
    EXPECT_EQ(it, v11.begin());
    
    
    // Both-shift
    //
    devector<dClass> v20{1,2,3,4,5,6};
    v20.pop_front();
    v20.pop_back();
    it = v20.insert(v20.begin()+1, {11, 11});
    EXPECT_EQ(v20.size(), 6);
    EXPECT_EQ(v20.offset(), 0);
    EXPECT_EQ(v20[0].val, 2);
    EXPECT_EQ(v20[1].val, 11);
    EXPECT_EQ(v20[2].val, 11);
    EXPECT_EQ(v20[3].val, 3);
    EXPECT_EQ(v20[5].val, 5);
    EXPECT_EQ(it, v20.begin()+1);
    
    //
    devector<dClass> v21{1,2,3,4,5};
    v21.pop_front();
    v21.pop_back();
    v21.pop_back();
    it = v21.insert(v21.begin()+1, {11, 11, 11});
    EXPECT_EQ(v21.size(), 5);
    EXPECT_EQ(v21.offset(), 0);
    EXPECT_EQ(v21[0].val, 2);
    EXPECT_EQ(v21[1].val, 11);
    EXPECT_EQ(v21[3].val, 11);
    EXPECT_EQ(v21[4].val, 3);
    EXPECT_EQ(it, v21.begin()+1);
    
    //
    devector<dClass> v22{1,2,3,4,5,6};
    v22.pop_front();
    v22.pop_front();
    v22.pop_back();
    v22.pop_back();
    it = v22.insert(v22.begin()+1, {11, 11, 11});
    EXPECT_EQ(v22.size(), 5);
    EXPECT_EQ(v22.offset(), 0);
    EXPECT_EQ(v22[0].val, 3);
    EXPECT_EQ(v22[1].val, 11);
    EXPECT_EQ(v22[3].val, 11);
    EXPECT_EQ(v22[4].val, 4);
    EXPECT_EQ(it, v22.begin()+1);
    
    //
    devector<dClass> v23{1,2,3,4,5,6,7};
    v23.pop_front();
    v23.pop_back();
    it = v23.insert(v23.begin()+3, {11, 11});
    EXPECT_EQ(v23.size(), 7);
    EXPECT_EQ(v23.offset(), 0);
    EXPECT_EQ(v23[0].val, 2);
    EXPECT_EQ(v23[1].val, 3);
    EXPECT_EQ(v23[3].val, 11);
    EXPECT_EQ(v23[4].val, 11);
    EXPECT_EQ(v23[5].val, 5);
    EXPECT_EQ(v23[6].val, 6);
    EXPECT_EQ(it, v23.begin()+3);
    
    
    // Re-alloc
    //
    devector<dClass> v30;
    it = v30.insert(v30.begin(), 0, 22);
    EXPECT_EQ(it, v30.begin());
    EXPECT_EQ(v30.capacity(), 0);
    it = v30.insert(v30.begin(), {11, 11, 11});
    EXPECT_EQ(v30.size(), 3);
    EXPECT_EQ(v30.offset(), 0);
    EXPECT_EQ(v30[0].val, 11);
    EXPECT_EQ(v30[2].val, 11);
    EXPECT_EQ(it, v30.begin());
    
    //
    devector<dClass> v31 = {1,2};
    it = v31.insert(v31.begin()+1, {11, 11});
    EXPECT_EQ(v31.size(), 4);
    EXPECT_EQ(v31.offset(), 0);
    EXPECT_EQ(v31[0].val, 1);
    EXPECT_EQ(v31[1].val, 11);
    EXPECT_EQ(v31[3].val, 2);
    EXPECT_EQ(it, v31.begin()+1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Emplace)
{
  {
    //
    devector<dClass> v0;
    auto it = v0.emplace(v0.begin(), 29);
    EXPECT_EQ(v0.size(), 1);
    EXPECT_EQ(v0[0].val, 29);
    EXPECT_EQ(it, v0.begin());
    
    it = v0.emplace(v0.begin(), 28);
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v0[0].val, 28);
    EXPECT_EQ(v0[1].val, 29);
    EXPECT_EQ(it, v0.begin());
    
    it = v0.emplace(v0.end(), 32);
    EXPECT_EQ(v0[2].val, 32);
    EXPECT_EQ(it, v0.end()-1);
    
    it = v0.emplace(v0.end()-1, 31);
    EXPECT_EQ(v0[2].val, 31);
    EXPECT_EQ(it, v0.end()-2);
    
    v0.push_back(33);
    v0.push_back(34);
    it = v0.erase(v0.begin()+1);
    EXPECT_EQ(it, v0.begin()+1);
    
    it = v0.emplace(v0.begin()+1, 30);
    EXPECT_EQ(v0[1].val, 30);
    EXPECT_EQ(it, v0.begin()+1);
  
    //
    devector<dClass> v1({1,2,3});
    v1.pop_front();
    v1.pop_front();
    it = v1.emplace(v1.end(), 37);
    EXPECT_EQ(v1.size(), 2);
  #ifdef CHECK_OFFSET
    EXPECT_EQ(v1.offset(), 1);
  #endif
    EXPECT_EQ(v1[0].val, 3);
    EXPECT_EQ(v1[1].val, 37);
    EXPECT_EQ(it, v1.end()-1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, AddRemove)
{
  {
    //
    devector<dClass> v0(2);
    auto it = v0.erase(v0.begin());
    EXPECT_EQ(it, v0.begin());
    v0.push_back(34);
    
    devector<dClass> v1(3);
    it = v1.erase(v1.begin());
    EXPECT_EQ(it, v1.begin());
    it = v1.erase(v1.begin());
    EXPECT_EQ(it, v1.begin());
    v1.push_back(34);
    
    //
    devector<dClass> v2(2);
    it = v2.erase(v2.begin());
    EXPECT_EQ(it, v2.begin());
    v2.emplace_back(35);
    
    devector<dClass> v3(3);
    it = v3.erase(v3.begin());
    EXPECT_EQ(it, v3.begin());
    it = v3.erase(v3.begin());
    EXPECT_EQ(it, v3.begin());
    v3.emplace_back(35);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, NonMember)
{
  {
    //
    devector<int> v0({1,2,3});
    devector<int> v1({1,2,3});
    devector<int> v2({4,5,6});
    
    EXPECT_TRUE( v0 == v1);
    EXPECT_FALSE(v0 != v1);
    EXPECT_TRUE( v0 <  v2);
    EXPECT_FALSE(v0 <  v1);
    EXPECT_TRUE( v0 <= v2);
    EXPECT_TRUE( v2 >  v1);
    EXPECT_FALSE(v0 >  v1);
    EXPECT_TRUE( v0 >= v1);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Swap)
{
  {
    //
    devector<dClass> v0 = {1,2,3};
    devector<dClass> v1 = {4,5};
    v0.reserve(5);
    v0.swap(v1);
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v0.capacity(), 2);
    EXPECT_EQ(v1.capacity(), 5);
    EXPECT_EQ(v0[1].val, 5);
    EXPECT_EQ(v1[2].val, 3);
    
    //
    std::swap(v0, v1);
    v0.emplace_back(4);
    v1.emplace_back(3);
    EXPECT_EQ(v0.size(), 4);
    EXPECT_EQ(v1.size(), 3);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Swap_Allocator)
{
  {
    //
    devector_alc v0 = {1,2,3};
    devector_alc v1 = {4,5};
    v0.reserve(5);
    v0.swap(v1);
    EXPECT_EQ(v0.size(), 2);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v0.capacity(), 2);
    EXPECT_EQ(v1.capacity(), 5);
    EXPECT_EQ(v0[1].val, 5);
    EXPECT_EQ(v1[2].val, 3);
    
    //
    std::swap(v0, v1);
    v0.emplace_back(4);
    v1.emplace_back(3);
    EXPECT_EQ(v0.size(), 4);
    EXPECT_EQ(v1.size(), 3);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, NonStandard)
{
  {
    //
    devector<int> v0({1,2,3});
    EXPECT_EQ(v0.offset(), 0);
    auto it2 = v0.erase(v0.begin());
    EXPECT_EQ(v0.offset(), 1);
    EXPECT_EQ(it2, v0.begin());
    
    //
    devector<int> v1(2);
    EXPECT_EQ(v1.offset(), 0);
    v1.pop_front();
    EXPECT_EQ(v1.offset(), 1);
    
    //
    devector<dClass> v2({1,2,3});
    auto it = v2.erase(v2.begin());
    EXPECT_EQ(v2.offset(), 1);
    EXPECT_EQ(it, v2.begin());
    EXPECT_EQ(v2[0].val, 2);
    EXPECT_EQ(v2[1].val, 3);
    v2.shift_data_start();
    EXPECT_EQ(v2.offset(), 0);
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[0].val, 2);
    EXPECT_EQ(v2[1].val, 3);
    EXPECT_EQ(v2.begin(), v2.end()-2);
    EXPECT_EQ((v2.end()-1), &v2[1]);
    
    //
    devector<dClass> v3;
    v3.shift_data_start();
    EXPECT_EQ(v3.size(), 0);
    
    //
    devector<dClass> v4(1);
    v4.shift_data_start();
    EXPECT_EQ(v4.size(), 1);
    EXPECT_EQ(v4[0].val, -1);
    
    //
    devector<dClass> v5({1,2,3});
    v5.pop_front();
    v5.pop_front();
    EXPECT_EQ(v5.offset(), 2);
    v5.shift_data_start();
    EXPECT_EQ(v5.size(), 1);
    EXPECT_EQ(v5[0].val, 3);
    
    //
    devector<dClass> v6({1,2,3});
    v6.pop_back();
    v6.pop_front();
    EXPECT_EQ(v6.size(), 1);
    EXPECT_EQ(v6.capacity(), 3);
    EXPECT_EQ(v6[0].val, 2);
    v6.pop_front();
    EXPECT_EQ(v6.size(), 0);
  #ifdef CHECK_OFFSET
    EXPECT_EQ(v6.offset(), 0);
  #endif
    EXPECT_EQ(v6.capacity(), 3);
    
    //
    devector<dClass> v7;
    v7.shift_data_end();
    EXPECT_EQ(v7.size(), 0);
    v7.reserve(4);
    v7.shift_data_end();
    EXPECT_EQ(v7.offset(), 4);
    v7.push_front(0);
    EXPECT_EQ(v7.size(), 1);
    EXPECT_EQ(v7.offset(), 3);
    
    //
    devector<dClass> v8;
    v8.shift_data_center();
    EXPECT_EQ(v8.size(), 0);
    v8.reserve(4);
    v8.shift_data_center();
    EXPECT_EQ(v8.offset(), 1);
    v8.push_back(1);
    EXPECT_EQ(v8.size(), 1);
    EXPECT_EQ(v8.offset(), 1);
    v8.push_back(2);
    v8.shift_data_center();
    EXPECT_EQ(v8.offset(), 1);
    v8 = {1,2,3,4,5};
    EXPECT_EQ(v8.capacity(), 5);
    v8.pop_front();
    v8.pop_front();
    v8.shift_data_center();
    EXPECT_EQ(v8.offset(), 1);
    v8.pop_front();
    v8.shift_data_center();
    EXPECT_EQ(v8.offset(), 1);
    v8 = {1,2,3,4,5};
    v8.erase(v8.begin()+1, v8.end());
    EXPECT_EQ(v8.offset(), 0);
    EXPECT_EQ(v8.size(), 1);
    v8.shift_data_center();
    EXPECT_EQ(v8.offset(), 2);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, ReallocMode)
{
  {
    //
    devector<dClass, devector_opt<ShiftMode::NEAR, ReallocMode::END>> v0;
    v0.reserve(3);
    EXPECT_EQ(v0.offset(), 3);
    v0.emplace_front(1);
    EXPECT_EQ(v0.offset(), 2);
    v0.clear();
  #ifdef INDIVI_DV_SHIFT_EMPTY
    EXPECT_EQ(v0.offset(), 3);
  #else
    EXPECT_EQ(v0.offset(), 2);
  #endif
    v0.emplace_back(2);
    EXPECT_EQ(v0.offset(), 2);
    v0.pop_back();
    EXPECT_TRUE(v0.empty());
  #ifdef INDIVI_DV_SHIFT_EMPTY
    EXPECT_EQ(v0.offset(), 3);
  #else
    EXPECT_EQ(v0.offset(), 2);
  #endif
    
    //
    devector<dClass, devector_opt<ShiftMode::NEAR, ReallocMode::CENTER>> v1;
    v1.reserve(3);
    EXPECT_EQ(v1.offset(), 1);
    v1.emplace_front(1);
    EXPECT_EQ(v1.offset(), 0);
    v1.clear();
  #ifdef INDIVI_DV_SHIFT_EMPTY
    EXPECT_EQ(v1.offset(), 1);
  #else
    EXPECT_EQ(v1.offset(), 0);
  #endif
    v1.emplace_back(2);
  #ifdef INDIVI_DV_SHIFT_EMPTY
    EXPECT_EQ(v1.offset(), 1);
  #else
    EXPECT_EQ(v1.offset(), 0);
  #endif
    v1.pop_back();
    EXPECT_TRUE(v1.empty());
  #ifdef INDIVI_DV_SHIFT_EMPTY
    EXPECT_EQ(v1.offset(), 1);
  #else
    EXPECT_EQ(v1.offset(), 0);
  #endif
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(DevectorTest, Throwing)
{
  {
    //
    EXPECT_ANY_THROW(devector<eClassCtr0> v0(3));
    eClassCtr0::resetOpCounters();
    
    //
    EXPECT_ANY_THROW(devector<eClassCtrCpy> v1(3,0));
    eClassCtrCpy::resetOpCounters();
    
    //
    EXPECT_ANY_THROW(devector<eClassCtrCpy> v2({1,2,3}));
    eClassCtrCpy::resetOpCounters();
    
    //
    devector<eClassCtrCpy> v3;
    v3.reserve(2);
    v3.emplace_back(1);
    v3.emplace_back(2);
    EXPECT_ANY_THROW(devector<eClassCtrCpy> v4(v3));
    EXPECT_NO_THROW(devector<eClassCtrCpy> v4(std::move(v3)));
    eClassCtrCpy::resetOpCounters();
    
    //
    devector<eClassCtrCpy> v5;
    v5.reserve(2);
    v5.emplace_back(1);
    v5.emplace_back(2);
    devector<eClassCtrCpy> v6;
    EXPECT_ANY_THROW(v6 = v5);
    eClassCtrCpy::resetOpCounters();
    
    //
    devector<eClassAsgCpy> v7(3,0);
    devector<eClassAsgCpy> v8(4,0);
    v8.pop_front();
    EXPECT_ANY_THROW(v8 = v7);
    eClassAsgCpy::resetOpCounters();
    EXPECT_ANY_THROW(v8 = std::initializer_list<eClassAsgCpy>({0,0,0}));
    EXPECT_NO_THROW(v8 = std::move(v7));
    eClassAsgCpy::resetOpCounters();
    
    //
    devector<eClassAsgCpy> v9(3,0);
    EXPECT_ANY_THROW(v9.assign(2,-1));
    eClassAsgCpy::resetOpCounters();
    
    devector<eClassAsgCpy> v10(2,0);
    EXPECT_ANY_THROW(v9.assign(v10.begin(),v10.end()));
    eClassAsgCpy::resetOpCounters();
    
    //
    devector<eClassCtrMve> v11(5,0);
    v11.erase(v11.begin(), v11.begin()+3);
    EXPECT_EQ(v11.offset(), 3);
    EXPECT_ANY_THROW(v11.shift_data_start());
    eClassCtrMve::resetOpCounters();
    
    //
    devector<eClassCtrMve> v12(5,0);
    v12.erase(v12.end()-3, v12.end());
    EXPECT_EQ(v12.offset(), 0);
    EXPECT_ANY_THROW(v12.shift_data_end());
    eClassCtrMve::resetOpCounters();
    
    //TODO: improve coverage
  }
  // No object leak
  EXPECT_EQ(eClassCtr0::count,   eClassCtr0::decount);
  EXPECT_EQ(eClassCtr1::count,   eClassCtr1::decount);
  EXPECT_EQ(eClassCtrCpy::count, eClassCtrCpy::decount);
  EXPECT_EQ(eClassCtrMve::count, eClassCtrMve::decount);
  EXPECT_EQ(eClassAsgCpy::count, eClassAsgCpy::decount);
  EXPECT_EQ(eClassAsgMve::count, eClassAsgMve::decount);
}
