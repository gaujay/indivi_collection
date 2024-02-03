/**
 * Copyright 2023 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#include "gtest/gtest.h"

#include "indivi/sparque.h"
#include "utils/bump_allocator.h"
#include "utils/debug_utils.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <numeric>
#include <ostream>
#include <utility>
#include <vector>

#include <cstdlib>
#include <ctime>

using namespace indivi;

template <class T, class Allocator, uint16_t N = (4u * sizeof(T) >= 1024u) ? 4u : 1024u / sizeof(T), uint16_t B = 16u>
using sparque_alc = sparque<T, N, B, Allocator>;

void PrintTo(const dClass& val, ::std::ostream* os)
{
  *os << "(";
  *os << "val:" << val.val;
  *os << ", ";
  *os << "id:" << val.id;
  *os << ", ";
  *os << "init:" << val._init;
  *os << ")";
}

TEST(SparqueTest, Constructor)
{
  {
    sparque<dClass> sq0;
    EXPECT_TRUE(sq0.empty());
    EXPECT_EQ(sq0.size(), 0u);
  }
  {
    sparque<dClass> sq1(0);
    EXPECT_TRUE(sq1.empty());
    EXPECT_EQ(sq1.size(), 0u);
  }
  {
    sparque<dClass> sq2(1);
    EXPECT_FALSE(sq2.empty());
    EXPECT_EQ(sq2.size(), 1u);
  }
  {
    sparque<dClass, 2, 3> sq0(7);
    EXPECT_EQ(sq0.size(), 7u);
    
    sparque<dClass, 2, 3> sq1(19);
    EXPECT_EQ(sq1.size(), 19u);
    
    sparque<dClass, 2, 2> sq2(19);
    EXPECT_EQ(sq2.size(), 19u);
    
    sparque<dClass, 2, 3> sq3(67);
    EXPECT_EQ(sq3.size(), 67u);
  }
  {
    sparque<dClass> sq(10000);
    EXPECT_EQ(sq.size(), 10000u);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Constructor2)
{
  { // copy ctr
    sparque<dClass, 3, 3> sq0(100u);
    EXPECT_EQ(sq0.size(), 100u);
    int i = 0;
    for (auto& v : sq0)
      v = i++;
    
    sparque<dClass, 3, 3> sq1(sq0);
    ASSERT_EQ(sq1.size(), 100u);
    EXPECT_EQ(sq1[0], 0);
    EXPECT_EQ(sq1[99], 99);
    EXPECT_EQ(sq1[9], sq0[9]);
    EXPECT_EQ(sq1[50], sq0[50]);
  }
  {
    sparque<dClass> sq0;
    EXPECT_EQ(sq0.size(), 0u);
    
    sparque<dClass> sq1(sq0);
    EXPECT_EQ(sq1.size(), 0u);
  }
  { // move ctr
    sparque<dClass, 2, 3> sq0(100u);
    EXPECT_EQ(sq0.size(), 100u);
    int i = 0;
    for (auto& v : sq0)
      v = i++;
    
    sparque<dClass, 2, 3> sq1(std::move(sq0));
    EXPECT_EQ(sq0.size(), 0u);
    ASSERT_EQ(sq1.size(), 100u);
    EXPECT_EQ(sq1[0], 0);
    EXPECT_EQ(sq1[99], 99);
  }
  {
    sparque<dClass> sq0;
    sparque<dClass> sq1(std::move(sq0));
    EXPECT_EQ(sq0.size(), 0u);
    EXPECT_EQ(sq1.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Constructor3)
{
  { // range ctr
    int sz = 251;
    std::vector<dClass> vec(sz);
    int i = 0;
    for (auto& v : vec)
      v = i++;
    
    sparque<dClass, 2, 5> sq0(vec.begin(), vec.end());
    ASSERT_EQ(sq0.size(), sz);
    EXPECT_EQ(sq0[0], 0);
    EXPECT_EQ(sq0[sz-1], sz-1);
    EXPECT_EQ(sq0[9], vec[9]);
    EXPECT_EQ(sq0[sz/2], vec[sz/2]);
    
    sparque<dClass, 3, 3> sq1(vec.data(), vec.data() + vec.size());
    EXPECT_EQ(sq0, sq1);
  }
  {
    std::vector<dClass> vec(52u, 77);
    EXPECT_EQ(vec[0], 77);
    
    sparque<dClass, 5, 7> sq(std::make_move_iterator(vec.begin()),
                             std::make_move_iterator(vec.end()));
    ASSERT_EQ(sq.size(), vec.size());
    EXPECT_EQ(sq[51], 77);
    EXPECT_EQ(vec[0], MVE_VAL_CODE); // moved
  }
  {
    std::vector<dClass> vec;
    sparque<dClass, 2, 5> sq0(vec.begin(), vec.end());
    EXPECT_TRUE(sq0.empty());
    
    sparque<dClass, 4, 5> sq1(vec.end(), vec.end());
    EXPECT_TRUE(sq1.empty());
  }
  {
    int sz = 197;
    sparque<dClass, 4, 3> sq0(sz);
    int i = 0;
    for (auto& v : sq0)
      v = i++;
    
    sparque<dClass, 2, 5> sq1(sq0.begin(), sq0.end());
    ASSERT_EQ(sq1.size(), sz);
    EXPECT_EQ(sq1[0], 0);
    EXPECT_EQ(sq1[sz-1], sz-1);
    EXPECT_EQ(sq1[9], sq0[9]);
    EXPECT_EQ(sq1[sz/2], sq0[sz/2]);
  }
  {
    std::list<dClass> list(46u, 21);
    
    sparque<dClass, 7, 4> sq(list.begin(), list.end());
    ASSERT_EQ(sq.size(), list.size());
    EXPECT_EQ(sq[45], 21);
    
    sparque<dClass, 5, 7> sq2(list.begin(), list.begin());
    EXPECT_EQ(sq2.size(), 0u);
  }
  { // initializer list
    sparque<dClass, 2, 2> sq0(
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19});
    ASSERT_EQ(sq0.size(), 20);
    for (int i = 0; i < (int)sq0.size(); ++i)
      EXPECT_EQ(sq0[i], i);
  }
  {
    sparque<dClass, 4, 2> sq0({});
    EXPECT_TRUE(sq0.empty());
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Constructor4)
{
  { // allocator
    sparque_alc<dClass, bump_allocator<dClass>> sq0;
    EXPECT_TRUE(sq0.empty());
    EXPECT_EQ(sq0.size(), 0u);
    
    bump_allocator<dClass> alc;
    sparque_alc<dClass, bump_allocator<dClass>> sq1(alc);
    EXPECT_TRUE(sq1.empty());
    EXPECT_EQ(sq1.size(), 0u);
  }
  {
    sparque_alc<dClass, bump_allocator<dClass>> sq0(0);
    EXPECT_TRUE(sq0.empty());
    EXPECT_EQ(sq0.size(), 0u);
    
    bump_allocator<dClass> alc;
    sparque_alc<dClass, bump_allocator<dClass>> sq1(0, alc);
    EXPECT_TRUE(sq1.empty());
    EXPECT_EQ(sq1.size(), 0u);
  }
  {
    sparque_alc<dClass, bump_allocator<dClass>> sq0(1, 5);
    EXPECT_FALSE(sq0.empty());
    ASSERT_EQ(sq0.size(), 1u);
    EXPECT_EQ(sq0[0], 5);
    
    bump_allocator<dClass> alc;
    sparque_alc<dClass, bump_allocator<dClass>> sq1(1, dClass(6), alc);
    EXPECT_FALSE(sq1.empty());
    ASSERT_EQ(sq1.size(), 1u);
    EXPECT_EQ(sq1[0], 6);
  }
  {
    std::vector<dClass> vec(25u, -4);
    sparque_alc<dClass, bump_allocator<dClass>> sq0(vec.cbegin(), vec.cend());
    EXPECT_FALSE(sq0.empty());
    ASSERT_EQ(sq0.size(), vec.size());
    EXPECT_EQ(sq0[0], -4);
    
    bump_allocator<dClass> alc;
    sparque_alc<dClass, bump_allocator<dClass>> sq1(vec.cbegin(), vec.cend(), alc);
    EXPECT_FALSE(sq1.empty());
    ASSERT_EQ(sq1.size(), vec.size());
    EXPECT_EQ(sq1[0], -4);
  }
  {
    sparque_alc<dClass, bump_allocator<dClass>> sq(3, 7);
    sparque_alc<dClass, bump_allocator<dClass>> sq0(sq);
    EXPECT_FALSE(sq0.empty());
    ASSERT_EQ(sq0.size(), sq.size());
    EXPECT_EQ(sq0[0], 7);
    
    bump_allocator<dClass> alc;
    sparque_alc<dClass, bump_allocator<dClass>> sq1(sq, alc);
    EXPECT_FALSE(sq1.empty());
    ASSERT_EQ(sq1.size(), sq.size());
    EXPECT_EQ(sq1[0], 7);
  }
  {
    {
      sparque_alc<dClass, bump_allocator<dClass>> sq(4, 1);
      sparque_alc<dClass, bump_allocator<dClass>> sq0(std::move(sq));
      EXPECT_EQ(sq.size(), 0u);
      ASSERT_EQ(sq0.size(), 4u);
      EXPECT_EQ(sq0[0], 1);
    }
    {
      bump_allocator<dClass> alc;
      sparque_alc<dClass, bump_allocator<dClass>> sq(4, 1);
      sparque_alc<dClass, bump_allocator<dClass>> sq1(std::move(sq), alc);
      ASSERT_EQ(sq.size(), 4u);
      EXPECT_EQ(sq[0], MVE_VAL_CODE);
      ASSERT_EQ(sq1.size(), 4u);
      EXPECT_EQ(sq1[0], 1);
    }
    {
      sparque_alc<dClass, std::allocator<dClass>> sq(4, 1);
      sparque_alc<dClass, std::allocator<dClass>> sq1(std::move(sq), std::allocator<dClass>());
      EXPECT_EQ(sq.size(), 0u);
      ASSERT_EQ(sq1.size(), 4u);
      EXPECT_EQ(sq1[0], 1);
    }
  }
  {
    sparque_alc<dClass, bump_allocator<dClass>> sq0({0, 1, 2, 3});
    ASSERT_EQ(sq0.size(), 4u);
    EXPECT_EQ(sq0[1], 1);
    
    bump_allocator<dClass> alc;
    sparque_alc<dClass, bump_allocator<dClass>> sq1({0, -1, -2}, alc);
    ASSERT_EQ(sq1.size(), 3u);
    EXPECT_EQ(sq1[2], -2);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, OpEqual)
{
  { // copy assign
    sparque<dClass, 3, 3> sq0(100u);
    EXPECT_EQ(sq0.size(), 100u);
    int i = 0;
    for (auto& v : sq0)
      v = i++;
    
    sparque<dClass, 3, 3> sq1 (5u, -7);
    EXPECT_EQ(sq1.size(), 5u);
    
    sq1 = sq0;
    ASSERT_EQ(sq1.size(), 100u);
    EXPECT_EQ(sq1[0], 0);
    EXPECT_EQ(sq1[99], 99);
    EXPECT_EQ(sq1[9], sq0[9]);
    EXPECT_EQ(sq1[50], sq0[50]);
    
    sparque<dClass, 3, 3> sq2(0u, 55);
    EXPECT_EQ(sq2.size(), 0u);
    sq2 = sq1;
    EXPECT_EQ(sq2.size(), 100u);
    
    sq1.clear();
    sparque<dClass, 3, 3> sq3(49u);
    sq3 = sq1;
    EXPECT_EQ(sq3.size(), 0u);
    
    sparque<dClass, 3, 3> sq4;
    sq4 = sq1;
    EXPECT_EQ(sq4.size(), 0u);
  }
  {
    sparque<dClass> sq0;
    EXPECT_EQ(sq0.size(), 0u);
    
    sparque<dClass> sq1;
    sq1 = sq0;
    EXPECT_EQ(sq1.size(), 0u);
    
    sparque<dClass> sq2(49u);
    sq2 = sq1;
    EXPECT_EQ(sq2.size(), 0u);
  }
  {
    sparque<dClass> sq(10u);
    auto& sq2 = sq;
    sq = sq2;
    EXPECT_EQ(sq.size(), 10u);
  }
  { // move assign
    sparque<dClass, 2, 3> sq0(100u);
    EXPECT_EQ(sq0.size(), 100u);
    int i = 0;
    for (auto& v : sq0)
      v = i++;
    
    sparque<dClass, 2, 3> sq1;
    sq1 = std::move(sq0);
    EXPECT_EQ(sq0.size(), 0u);
    ASSERT_EQ(sq1.size(), 100u);
    EXPECT_EQ(sq1[0], 0);
    EXPECT_EQ(sq1[99], 99);
    
    sparque<dClass, 2, 3> sq2(12u);
    sq2 = std::move(sq0);
    EXPECT_EQ(sq0.size(), 0u);
    EXPECT_EQ(sq2.size(), 0u);
  }
  {
    sparque<dClass> sq0;
    sparque<dClass> sq1;
    sq1 = std::move(sq0);
    EXPECT_EQ(sq0.size(), 0u);
    EXPECT_EQ(sq1.size(), 0u);
  }
  {
    sparque<dClass> sq0;
    sq0 = {1, 2, 3};
    ASSERT_EQ(sq0.size(), 3u);
    EXPECT_EQ(sq0[0], 1);
    EXPECT_EQ(sq0[1], 2);
    EXPECT_EQ(sq0[2], 3);
  }
  { // allocator
    sparque_alc<dClass, bump_allocator<dClass>> sq1(11u, 11);
    sparque_alc<dClass, bump_allocator<dClass>> sq2(22u, 22);
    
    sq1 = sq2;
    ASSERT_EQ(sq1.size(), sq2.size());
    EXPECT_EQ(sq1[10], 22);
    EXPECT_EQ(sq2[10], 22);
  }
  {
    std::allocator<dClass> alc;
    sparque_alc<dClass, std::allocator<dClass>> sq1(11u, 11, alc);
    sparque_alc<dClass, std::allocator<dClass>> sq2(22u, 22, alc);
    
    sq1 = sq2;
    ASSERT_EQ(sq1.size(), sq2.size());
    EXPECT_EQ(sq1[10], 22);
    EXPECT_EQ(sq2[10], 22);
  }
  {
    sparque_alc<dClass, bump_allocator<dClass>> sq1(11u, 11);
    sparque_alc<dClass, bump_allocator<dClass>> sq2(22u, 22);
    
    sq1 = std::move(sq2);
    ASSERT_EQ(sq1.size(), sq2.size());
    EXPECT_EQ(sq1[10], 22);
    EXPECT_EQ(sq2[10], MVE_VAL_CODE);
  }
  {
    std::allocator<dClass> alc;
    sparque_alc<dClass, std::allocator<dClass>> sq1(11u, 11, alc);
    sparque_alc<dClass, std::allocator<dClass>> sq2(22u, 22, alc);
    
    sq1 = std::move(sq2);
    EXPECT_EQ(sq1.size(), 22u);
    EXPECT_EQ(sq2.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Assign)
{
  { // value
    sparque<dClass, 2, 6> sq0;
    EXPECT_EQ(sq0.size(), 0u);
    
    sq0.assign(10, -10);
    ASSERT_EQ(sq0.size(), 10);
    EXPECT_EQ(sq0[0], -10);
    EXPECT_EQ(sq0[9], -10);
  }
  {
    sparque<dClass, 2, 6> sq0(99);
    EXPECT_EQ(sq0.size(), 99u);
    
    sq0.assign(0, -5);
    EXPECT_EQ(sq0.size(), 0u);
    
    sq0.assign(4, 20);
    ASSERT_EQ(sq0.size(), 4u);
    EXPECT_EQ(sq0[0], 20);
    EXPECT_EQ(sq0[3], 20);
  }
  { // iterators
    sparque<dClass, 2, 6> sq0;
    EXPECT_EQ(sq0.size(), 0u);
    
    std::vector<dClass> vec {4, 5, 6, 7};
    sq0.assign(vec.begin(), vec.end());
    ASSERT_EQ(sq0.size(), vec.size());
    EXPECT_EQ(sq0[0], vec[0]);
    EXPECT_EQ(sq0[3], vec[3]);
    
    sparque<dClass, 5, 3> sq1(12u, -7);
    EXPECT_EQ(sq1.size(), 12u);
    sq0.assign(sq1.begin(), sq1.end());
    ASSERT_EQ(sq0.size(), sq1.size());
    EXPECT_EQ(sq0[0], sq1[0]);
    EXPECT_EQ(sq0[11], sq1[11]);
  }
  {
    sparque<dClass, 4, 3> sq0;
    EXPECT_EQ(sq0.size(), 0u);
    
    std::vector<dClass> vec {4, 5, 6, 7};
    sq0.assign(std::make_move_iterator(vec.begin()),
               std::make_move_iterator(vec.end()));
    ASSERT_EQ(sq0.size(), 4u);
    EXPECT_EQ(sq0[0], 4);
    EXPECT_EQ(sq0[3], 7);
    EXPECT_EQ(vec[0], MVE_VAL_CODE); // moved
  }
  {
    sparque<dClass, 2, 6> sq0(99);
    EXPECT_EQ(sq0.size(), 99u);
    
    std::vector<dClass> vec;
    EXPECT_EQ(vec.size(), 0u);
    sq0.assign(vec.begin(), vec.end());
    EXPECT_EQ(sq0.size(), vec.size());
    
    sq0.resize(1u, -55);
    
    std::list<dClass> list(87u);
    std::iota(list.begin(), list.end(), 10);
    sq0.assign(list.begin(), list.end());
    ASSERT_EQ(sq0.size(), list.size());
    EXPECT_EQ(sq0[0], 10);
    EXPECT_EQ(sq0[33], 43);
    EXPECT_EQ(sq0[86], 96);
    
    list.resize(10u);
    sq0.assign(list.begin(), list.end());
    ASSERT_EQ(sq0.size(), list.size());
    EXPECT_EQ(sq0[9], 19);
    
    sq0.assign(list.end(), list.end());
    EXPECT_EQ(sq0.size(), 0u);
  }
  { // initializer list
    sparque<dClass, 2, 6> sq0;
    EXPECT_EQ(sq0.size(), 0u);
    
    sq0.assign({8, -9});
    ASSERT_EQ(sq0.size(), 2u);
    EXPECT_EQ(sq0[0], 8);
    EXPECT_EQ(sq0[1], -9);
    
    sq0.assign({-8, 9});
    ASSERT_EQ(sq0.size(), 2u);
    EXPECT_EQ(sq0[0], -8);
    EXPECT_EQ(sq0[1], 9);
  }
  {
    sparque<dClass, 7, 3> sq0(84u);
    EXPECT_EQ(sq0.size(), 84u);
    
    sq0.assign({});
    EXPECT_EQ(sq0.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Capacity)
{
  {
    sparque<dClass> sq;
    EXPECT_TRUE(sq.empty());
    EXPECT_EQ(sq.size(), 0u);
    
    sq.push_back(1);
    EXPECT_FALSE(sq.empty());
    EXPECT_EQ(sq.size(), 1u);
    
    sq.pop_back();
    EXPECT_TRUE(sq.empty());
    EXPECT_EQ(sq.size(), 0u);
    
    EXPECT_EQ(sq.max_size(),
              (size_t)std::numeric_limits<sparque<dClass>::difference_type>::max());
  }
  { // non-standard
    sparque<dClass, 4, 2> sq(250u);
    EXPECT_EQ(sq.size(), 250u);
    
    EXPECT_EQ(sq.height(), 6u);
    EXPECT_EQ(sq.node_count(), 31u);
    EXPECT_EQ(sq.leaf_count(), 32u);
    EXPECT_EQ(sq.count_chunks(), 63u);
    
    sq.clear();
    EXPECT_EQ(sq.height(), 0u);
    EXPECT_EQ(sq.node_count(), 0u);
    EXPECT_EQ(sq.leaf_count(), 0u);
    EXPECT_EQ(sq.count_chunks(), 0u);
    
    sq.push_back(58);
    EXPECT_EQ(sq.height(), 1u);
    EXPECT_EQ(sq.node_count(), 0u);
    EXPECT_EQ(sq.leaf_count(), 1u);
    EXPECT_EQ(sq.count_chunks(), 1u);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Swap)
{
  {
    sparque<dClass, 7> sq0(54u, 54);
    sparque<dClass, 7> sq1(12u, 12);
    
    sq0.swap(sq1);
    ASSERT_EQ(sq0.size(), 12u);
    ASSERT_EQ(sq1.size(), 54u);
    EXPECT_EQ(sq0[5], 12);
    EXPECT_EQ(sq1[6], 54);
  }
  {
    sparque<dClass, 4> sq0;
    sparque<dClass, 4> sq1(10u, 10);
    
    sq0.swap(sq1);
    EXPECT_EQ(sq0.size(), 10u);
    EXPECT_EQ(sq1.size(), 0u);
    
    sq1.swap(sq0);
    EXPECT_EQ(sq0.size(), 0u);
    EXPECT_EQ(sq1.size(), 10u);
  }
  {
    sparque<dClass, 4> sq0;
    sparque<dClass, 4> sq1(16u, 16);
    
    std::swap(sq0, sq1);
    EXPECT_EQ(sq0.size(), 16u);
    EXPECT_EQ(sq1.size(), 0u);
    
    std::swap(sq1, sq0);
    EXPECT_EQ(sq0.size(), 0u);
    EXPECT_EQ(sq1.size(), 16u);
    
    sq0.push_back(-1);
    sq0.push_back(3);
    
    std::swap(sq1, sq0);
    EXPECT_EQ(sq0.size(), 16u);
    EXPECT_EQ(sq1.size(), 2u);
  }
  { // allocator
    sparque_alc<dClass, bump_allocator<dClass>> sq1(11u, 11);
    sparque_alc<dClass, bump_allocator<dClass>> sq2(22u, 22);
    
    sq1.swap(sq2);
    EXPECT_EQ(sq1.size(), 22u);
    EXPECT_EQ(sq2.size(), 11u);
  }
  {
    sparque_alc<dClass, std::allocator<dClass>> sq1(11u, 11);
    sparque_alc<dClass, std::allocator<dClass>> sq2(22u, 22);
    
    sq1.swap(sq2);
    EXPECT_EQ(sq1.size(), 22u);
    EXPECT_EQ(sq2.size(), 11u);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Compare)
{
  {
    sparque<int, 8> sq0(24u);
    sparque<int, 8> sq1(sq0);
    
    EXPECT_EQ(sq0, sq1);
    EXPECT_EQ(sq1, sq0);
    EXPECT_FALSE(sq0 != sq1);
    EXPECT_LE(sq0, sq1);
    EXPECT_LE(sq1, sq0);
    EXPECT_GE(sq0, sq1);
    EXPECT_GE(sq1, sq0);
    EXPECT_FALSE(sq0 < sq1);
    EXPECT_FALSE(sq1 < sq0);
    EXPECT_FALSE(sq0 > sq1);
    EXPECT_FALSE(sq1 > sq0);
  }
  {
    sparque<int, 8> sq0(22u);
    sparque<int, 5> sq1;
    
    EXPECT_NE(sq0, sq1);
    EXPECT_NE(sq1, sq0);
    EXPECT_FALSE(sq0 == sq1);
    EXPECT_GT(sq0, sq1);
    EXPECT_LT(sq1, sq0);
    EXPECT_GE(sq0, sq1);
    EXPECT_LE(sq1, sq0);
    EXPECT_FALSE(sq0 < sq1);
    EXPECT_FALSE(sq1 > sq0);
    EXPECT_FALSE(sq0 <= sq1);
    EXPECT_FALSE(sq1 >= sq0);
  }
  {
    sparque<int, 2, 3> sq0(2u);
    sparque<int, 3, 4> sq1(3u);
    sq0[0] = sq1[0] = 0;
    sq0[1] = sq1[1] = 1;
    sq1[2] = 2;
    
    EXPECT_NE(sq0, sq1);
    EXPECT_NE(sq1, sq0);
    EXPECT_FALSE(sq0 == sq1);
    EXPECT_LT(sq0, sq1);
    EXPECT_GT(sq1, sq0);
    EXPECT_LE(sq0, sq1);
    EXPECT_GE(sq1, sq0);
    EXPECT_FALSE(sq0 > sq1);
    EXPECT_FALSE(sq1 < sq0);
    EXPECT_FALSE(sq0 >= sq1);
    EXPECT_FALSE(sq1 <= sq0);
  }
  {
    sparque<double> sq0(2u);
    sparque<double> sq1(3u);
    sq0[0] = 1.;
    sq1[0] = 0.;
    
    EXPECT_NE(sq0, sq1);
    EXPECT_NE(sq1, sq0);
    EXPECT_FALSE(sq0 == sq1);
    EXPECT_GT(sq0, sq1);
    EXPECT_LT(sq1, sq0);
    EXPECT_GE(sq0, sq1);
    EXPECT_LE(sq1, sq0);
    EXPECT_FALSE(sq0 < sq1);
    EXPECT_FALSE(sq1 > sq0);
    EXPECT_FALSE(sq0 <= sq1);
    EXPECT_FALSE(sq1 >= sq0);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, ElementAccess)
{
  { // subscript
    sparque<dClass> sq(10u);
    for (int i = 0; i < (int)sq.size(); ++i)
      sq[i] = i;
    for (int i = 0; i < (int)sq.size(); ++i)
      EXPECT_EQ(sq[i], i);
  }
  {
    const sparque<dClass> sq(10u, -2);
    for (int i = 0; i < (int)sq.size(); ++i)
      EXPECT_EQ(sq[i], -2);
  }
  { // at
    sparque<dClass> sq(10u);
    for (int i = 0; i < (int)sq.size(); ++i)
      sq[i] = i;
    for (int i = 0; i < (int)sq.size(); ++i)
      EXPECT_EQ(sq.at(i), i);
  }
  {
    const sparque<dClass> sq(10u, -2);
    for (int i = 0; i < (int)sq.size(); ++i)
      EXPECT_EQ(sq.at(i), -2);
  }
  {
    sparque<dClass> sq;
    EXPECT_THROW(sq.at(0), std::out_of_range);
  }
  {
    const sparque<dClass> sq(10);
    EXPECT_THROW(sq.at(10), std::out_of_range);
  }
  { // back
    sparque<dClass> sq(10u, 6);
    sq[9] = 9;
    EXPECT_EQ(sq.back(), 9);
    
    sq.back() = -9;
    EXPECT_EQ(sq.back(), -9);
  }
  {
    const sparque<dClass> sq(10u, 6);
    EXPECT_EQ(sq.back(), 6);
  }
  { // front
    sparque<dClass> sq(10u, 6);
    sq[0] = 9;
    EXPECT_EQ(sq.front(), 9);
    
    sq.front() = -9;
    EXPECT_EQ(sq.front(), -9);
  }
  {
    const sparque<dClass> sq(10u, 6);
    EXPECT_EQ(sq.front(), 6);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Iterator)
{
  {
    sparque<dClass, 2, 3> sq;
    EXPECT_TRUE(sq.empty());
    EXPECT_EQ(sq.begin(), sq.end());
  }
  {
    sparque<dClass> sq(3);
    sq[0] = 0;
    sq[1] = 1;
    sq[2] = 2;
    
    EXPECT_EQ(*sq.begin(), 0);
    EXPECT_EQ(*(--sq.end()), 2);
    EXPECT_EQ(sq.begin() + 2, sq.end() - 1);
    EXPECT_EQ(sq.begin() + sq.size(), sq.end());
    EXPECT_EQ(sq.end() - sq.size(), sq.begin());
    EXPECT_NE(sq.cbegin(), sq.cend());
    EXPECT_LT(sq.cbegin(), sq.cend());
    EXPECT_GT(sq.cend(), sq.cbegin());
    
    EXPECT_EQ(sq.begin() + 0, sq.begin());
    EXPECT_EQ(sq.begin() - 0, sq.begin());
    EXPECT_EQ(sq.end() + 0, sq.end());
    EXPECT_EQ(sq.end() - 0, sq.end());
  }
  {
    sparque<dClass, 3, 2> sq(7);
    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;
    
    size_t i = 0u;
    for (auto it = sq.begin(); it < sq.end(); ++it, ++i)
    {
      EXPECT_EQ(*it, (int)i);
      EXPECT_EQ(*it, sq[i]);
      EXPECT_EQ(it->val, sq[i].val);
    }
    EXPECT_EQ(i, sq.size());
    
    i = sq.size();
    for (auto it = sq.end(); it > sq.begin();)
    {
      --it;
      --i;
      EXPECT_EQ(*it, (int)i);
      EXPECT_EQ(*it, sq[i]);
    }
    EXPECT_EQ(i, 0u);
  }
  {
    sparque<dClass, 2, 3> sq(15);
    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;
    
    size_t i = 0u;
    for (auto it = sq.begin(); it < sq.end(); ++it)
    {
      EXPECT_EQ(*it, (int)i);
      EXPECT_EQ(*it, sq[i++]);
    }
    EXPECT_EQ(i, sq.size());
    
    EXPECT_EQ(*(++sq.begin()), sq[1]);
    EXPECT_EQ(*(sq.begin()++), sq[0]);
    EXPECT_EQ(*(--sq.end()),   sq[sq.size() - 1u]);
    
    EXPECT_EQ(sq.begin() - sq.begin(), 0);
    EXPECT_EQ(sq.end() - sq.end(), 0);
    EXPECT_EQ(sq.end() - sq.begin(), (std::ptrdiff_t)sq.size());
    EXPECT_EQ(sq.begin() + sq.size(), sq.end());
    EXPECT_EQ(--sq.end() - ++sq.begin(), (std::ptrdiff_t)sq.size() - 2);
  }
  {
    sparque<dClass, 3, 3> sq(10);
    sparque<dClass, 3, 3>::iterator it1(sq.begin());
    sparque<dClass, 3, 3>::iterator it2 = sq.end();
    EXPECT_EQ(it1, sq.begin());
    EXPECT_EQ(it2, sq.end());
  }
  { // sort
    srand(4598515u);

    std::vector<int> vec(299u);
    sparque<int, 4, 4> sq(299u);
    
    int i = 0;
    for (auto& v : sq)
    {
      int rng = rand();
      vec[i++] = rng;
      v = rng;
    }
    EXPECT_EQ(sq.front(), vec.front());
    EXPECT_EQ(sq.back(), vec.back());
    
    std::sort(vec.begin(), vec.end());
    std::sort(sq.begin(), sq.end());
    
    i = 0;
    for (const auto& v : sq)
      EXPECT_EQ(v, vec[i++]);
  }
  { // nth
    sparque<dClass, 6, 3> sq(301u);
    std::iota(sq.begin(), sq.end(), 0);
    
    EXPECT_EQ(sq.begin(), sq.nth(0u));
    EXPECT_EQ(sq.begin() + 11, sq.nth(11u));
    EXPECT_EQ(sq.end() - 27, sq.nth(sq.size() - 27u));
    EXPECT_EQ(sq.end(), sq.nth(sq.size()));
  }
  { // sparse
    srand(13495u);
    
    sparque<dClass, 3, 6> sq(310u);
    std::iota(sq.begin(), sq.end(), 0);
    
    for (int i = 0; i < 87; ++i)
    {
      int rng = rand() % (int)sq.size();
      sq.erase(sq.nth(rng));
    }
    
    auto last = --sq.end();
    for (auto it = sq.begin(); it != last; ++it)
      EXPECT_LT(it->val, (it + 1)->val);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, ReverseIterator)
{
  {
    sparque<dClass, 2, 3> sq;
    EXPECT_TRUE(sq.empty());
    EXPECT_EQ(sq.rbegin(), sq.rend());
  }
  {
    sparque<dClass> sq(3);
    sq[0] = 0;
    sq[1] = 1;
    sq[2] = 2;
    
    EXPECT_EQ(*sq.rbegin(), 2);
    EXPECT_EQ(*(--sq.rend()), 0);
    EXPECT_EQ(sq.rbegin() + 2, sq.rend() - 1);
    EXPECT_EQ(sq.rbegin() + sq.size(), sq.rend());
    EXPECT_EQ(sq.rend() - sq.size(), sq.rbegin());
    EXPECT_NE(sq.crbegin(), sq.crend());
    EXPECT_LT(sq.crbegin(), sq.crend());
    EXPECT_GT(sq.crend(), sq.crbegin());
    
    auto it = sq.crbegin() + 2;
    ++it;
    --it;
    EXPECT_EQ(*it, 0);
  }
  {
    sparque<dClass, 3, 2> sq(7);
    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;
    
    int i = (int)sq.size() - 1;
    for (auto it = sq.rbegin(); it != sq.rend(); ++it, --i)
    {
      EXPECT_EQ(*it, i);
      EXPECT_EQ(*it, sq[i]);
      EXPECT_EQ(it->val, sq[i].val);
    }
    EXPECT_EQ(i, -1);
    
    i = -1;
    for (auto it = sq.rend(); it > sq.rbegin();)
    {
      --it;
      ++i;
      EXPECT_EQ(*it, (int)i);
      EXPECT_EQ(*it, sq[i]);
    }
    EXPECT_EQ(i, (int)sq.size() - 1);
  }
  {
    sparque<dClass> sq(17u);
    
    EXPECT_EQ(*(++sq.rbegin()), sq[sq.size() - 2u]);
    EXPECT_EQ(*(sq.rbegin()++), sq[sq.size() - 1u]);
    EXPECT_EQ(*(--sq.rend()),   sq[0]);
    
    EXPECT_EQ(sq.rbegin() - sq.rbegin(), 0);
    EXPECT_EQ(sq.rend() - sq.rend(), 0);
    EXPECT_EQ(sq.rend() - sq.rbegin(), (std::ptrdiff_t)sq.size());
    EXPECT_EQ(sq.rbegin() + sq.size(), sq.rend());
    EXPECT_EQ(--sq.rend() - ++sq.rbegin(), (std::ptrdiff_t)sq.size() - 2);
  }
  {
    sparque<dClass, 4> sq(10);
    sparque<dClass, 4>::reverse_iterator it1(sq.rbegin());
    sparque<dClass, 4>::reverse_iterator it2 = sq.rend();
    EXPECT_EQ(it1, sq.rbegin());
    EXPECT_EQ(it2, sq.rend());
    EXPECT_EQ(sq.rbegin(), sq.crbegin());
    EXPECT_EQ(sq.rend(), sq.crend());
  }
  { // sparse
    srand(62897u);
    
    sparque<dClass, 5, 6> sq(365u);
    std::iota(sq.begin(), sq.end(), 0);
    
    for (int i = 0; i < 134; ++i)
    {
      int rng = rand() % (int)sq.size();
      sq.erase(sq.nth(rng));
    }
    
    auto second = --sq.rend();
    for (auto it = sq.rbegin(); it != second; ++it)
      EXPECT_GT(it->val, (it + 1)->val);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Clear)
{
  {
    sparque<dClass> sq0;
    sq0.clear();
    EXPECT_TRUE(sq0.empty());
    EXPECT_EQ(sq0.size(), 0u);
    
    sparque<dClass> sq1(1);
    sq1.clear();
    EXPECT_TRUE(sq1.empty());
    EXPECT_EQ(sq1.size(), 0u);
  }
  {
    sparque<dClass, 2, 3> sq0(7);
    sq0.clear();
    EXPECT_EQ(sq0.size(), 0u);
    
    sparque<dClass, 2, 3> sq1(19);
    sq1.clear();
    EXPECT_EQ(sq1.size(), 0u);
  }
  {
    sparque<dClass, 3, 2> sq0(111);
    sq0.clear();
    EXPECT_TRUE(sq0.empty());
    
    sq0.push_back(1);
    sq0.push_front(0);
    sq0.push_back(2);
    
    EXPECT_EQ(sq0.size(), 3u);
    EXPECT_EQ(sq0[0], 0);
    EXPECT_EQ(sq0[1], 1);
    EXPECT_EQ(sq0[2], 2);
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, BackModifier)
{
  // Push
  {
    sparque<dClass, 2, 2> sq;
    EXPECT_TRUE(sq.empty());
    
    sq.push_back(1);
    EXPECT_EQ(sq.size(), 1u);
    EXPECT_EQ(sq[0], 1);
    
    sq.push_back(2);
    sq.push_back(3);
    EXPECT_EQ(sq.size(), 3u);
    EXPECT_EQ(sq[0], 1);
    EXPECT_EQ(sq[1], 2);
    EXPECT_EQ(sq[2], 3);
    
    for (int i = 4; i < 100; ++i)
    {
      sq.push_back(i);
      EXPECT_EQ(sq.size(), (size_t)i);
      EXPECT_EQ(sq[i-1], i);
    }
    int i = 0;
    for (const auto& v : sq)
    {
      EXPECT_EQ(v.val, ++i);
    }
  }
  {
    sparque<dClass, 6, 2> sq;
    dClass dc(1);
    sq.push_back(dc);
    
    ++dc.val;
    sq.push_back(dc);
    
    ++dc.val;
    sq.push_back(dc);
    
    int i = 0;
    for (const auto& v : sq)
    {
      EXPECT_EQ(v.val, ++i);
    }
  }
  // Emplace
  {
    sparque<dClass, 5, 3> sq;
    EXPECT_TRUE(sq.empty());
    
    sq.emplace_back(1);
    EXPECT_EQ(sq.size(), 1u);
    EXPECT_EQ(sq[0], 1);
    
    sq.emplace_back(2);
    sq.emplace_back(3);
    EXPECT_EQ(sq.size(), 3u);
    EXPECT_EQ(sq[0], 1);
    EXPECT_EQ(sq[1], 2);
    EXPECT_EQ(sq[2], 3);
    
    for (int i = 4; i < 100; ++i)
    {
      sq.emplace_back(i);
      EXPECT_EQ(sq.size(), (size_t)i);
      EXPECT_EQ(sq[i-1], i);
    }
    int i = 0;
    for (const auto& v : sq)
    {
      EXPECT_EQ(v.val, ++i);
    }
  }
  {
    sparque<std::pair<dClass, double>> sq;
    
    sq.emplace_back(1, 2.);
    EXPECT_EQ(sq.size(), 1u);
    
    auto pair = std::pair<dClass, double>(1, 2.);
    EXPECT_EQ(sq[0], pair);
  }
  // Pop
  {
    sparque<dClass, 5, 2> sq(54);
    EXPECT_EQ(sq.size(), 54u);
    
    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;
    
    while (!sq.empty())
    {
      sq.pop_back();
      
      size_t i = 0u;
      for (; i < sq.size(); ++i)
        EXPECT_EQ(sq[i], (int)i);
      EXPECT_EQ(i, sq.size());
    }
  }
  {
    sparque<dClass, 2, 4> sq(21);
    EXPECT_EQ(sq.size(), 21u);
     
    sq.erase(sq.nth(sq.size() / 3));
    sq.erase(sq.nth(sq.size() / 2));
    sq.erase(sq.nth(sq.size() * 3/4));
     
    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;
     
    while (!sq.empty())
    {
      sq.pop_back();
       
      size_t i = 0u;
      for (; i < sq.size(); ++i)
        EXPECT_EQ(sq[i], (int)i);
      EXPECT_EQ(i, sq.size());
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, FrontModifier)
{
  // Push
  {
    sparque<dClass, 2, 4> sq;
    EXPECT_TRUE(sq.empty());
    
    sq.push_front(1);
    EXPECT_EQ(sq.size(), 1u);
    EXPECT_EQ(sq[0], 1);
    
    sq.push_front(2);
    sq.push_front(3);
    EXPECT_EQ(sq.size(), 3u);
    EXPECT_EQ(sq[0], 3);
    EXPECT_EQ(sq[1], 2);
    EXPECT_EQ(sq[2], 1);
    
    for (int i = 4; i < 100; ++i)
    {
      sq.push_front(i);
      EXPECT_EQ(sq.size(), (size_t)i);
      EXPECT_EQ(sq[0], i);
    }
    int i = (int)sq.size();
    for (const auto& v : sq)
    {
      EXPECT_EQ(v.val, i--);
    }
  }
  {
    sparque<dClass, 6, 2> sq;
    dClass dc(1);
    sq.push_front(dc);
    
    ++dc.val;
    sq.push_front(dc);
    
    ++dc.val;
    sq.push_front(dc);
    
    int i = (int)sq.size();
    for (const auto& v : sq)
    {
      EXPECT_EQ(v.val, i--);
    }
  }
  // Emplace
  {
    sparque<dClass, 5, 3> sq;
    EXPECT_TRUE(sq.empty());
    
    sq.emplace_front(1);
    EXPECT_EQ(sq.size(), 1u);
    EXPECT_EQ(sq[0], 1);
    
    sq.emplace_front(2);
    sq.emplace_front(3);
    EXPECT_EQ(sq.size(), 3u);
    EXPECT_EQ(sq[0], 3);
    EXPECT_EQ(sq[1], 2);
    EXPECT_EQ(sq[2], 1);
    
    for (int i = 4; i < 100; ++i)
    {
      sq.emplace_front(i);
      EXPECT_EQ(sq.size(), (size_t)i);
      EXPECT_EQ(sq[0], i);
    }
    int i = (int)sq.size();
    for (const auto& v : sq)
    {
      EXPECT_EQ(v.val, i--);
    }
  }
  // Pop
  {
    sparque<dClass, 5, 2> sq(86);
    sq.push_front(0);
    EXPECT_EQ(sq.size(), 87u);

    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;

    int offset = 0;
    while (!sq.empty())
    {
      sq.pop_front();
      ++offset;

      size_t i = 0u;
      for (; i < sq.size(); ++i)
        EXPECT_EQ(sq[i], (int)i + offset) << "-> i: " << i + offset;
      EXPECT_EQ(i, sq.size()) << "-> i Vs size: " << i;
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, DoubleEndedModifier)
{
  srand(545229u);
  {
    sparque<dClass, 23, 9> sq(0u);
    
    for (int i = 0; i < 10000; ++i)
    {
      int rng = (int)(rand() % 6);
      int op = rng % 6;
      switch (op)
      {
        case 0: {
          sq.push_back(1);
          break;
        }
        case 1: {
          sq.push_front(-1);
          break;
        }
        case 2: {
          if (!sq.empty())
            sq.pop_back();
          break;
        }
        case 3: {
          if (!sq.empty())
            sq.pop_front();
          break;
        }
        case 4: {
          if (!sq.empty())
            sq.erase(sq.nth(rng % sq.size()));
          break;
        }
        case 5: {
          sq.insert(sq.nth(rng % (sq.size() + 1)), 2);
          break;
        }
        default:
          assert(false);
      }
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Erase)
{
  {
    sparque<dClass, 3, 2> sq(9);
    EXPECT_EQ(sq.size(), 9u);
    
    for (size_t i = 0u; i < sq.size(); ++i)
      sq[i] = (int)i;
    
    size_t i = 0u;
    for (auto it = sq.begin(); it < sq.end(); ++it)
      EXPECT_EQ(*it, sq[i++]);
    EXPECT_EQ(i, sq.size());
    
    {
      auto to_erase = sq.begin();
      auto next = *(to_erase + 1);
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(*erased, next);
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = sq.begin() + 2;
      auto next = *(to_erase + 1);
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(*erased, next);
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = ++sq.begin();
      auto next = *(to_erase + 1);
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(*erased, next);
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = sq.begin();
      auto next = *(++sq.begin());
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(*erased, next);
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = ++sq.begin();
      auto next = *(to_erase + 1);
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(*erased, next);
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = sq.begin();
      auto next = *(to_erase + 1);
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(*erased, next);
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = ++(++sq.begin());
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(erased, sq.end());
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = ++sq.begin();
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(erased, sq.end());
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
    {
      auto to_erase = sq.begin();
      auto erased = sq.erase(to_erase);
      EXPECT_EQ(erased, sq.end());
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, sq[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Erase2)
{
  srand(852257u);
  {
    size_t size = 2339u;
    std::vector<int> vc(size);
    sparque<dClass, 6, 4> sq(size);
    EXPECT_EQ(sq.size(), size);
    
    for (size_t i = 0u; i < sq.size(); ++i)
    {
      vc[i] = (int)i;
      sq[i] = (int)i;
    }
    
    size_t i = 0u;
    for (auto it = sq.begin(); it < sq.end(); ++it)
      EXPECT_EQ(*it, vc[i++]);
    EXPECT_EQ(i, sq.size());
    
    while (!sq.empty())
    {
      size_t pos = (size_t)(rand() % sq.size());
      bool last = pos + 1u == sq.size();
      
      vc.erase(vc.begin() + pos);
      
      auto to_erase = sq.nth(pos);
      auto next = !last ? *(to_erase + 1) : dClass(-1);
      auto itNext = sq.erase(to_erase);
      if (!last)
        EXPECT_EQ(*itNext, next);
      else
        EXPECT_EQ(itNext, sq.end());
      
      EXPECT_EQ(sq.size(), vc.size());
      
      i = 0u;
      for (auto it = sq.begin(); it < sq.end(); ++it)
      {
        EXPECT_EQ(*it, vc[i++]);
      }
      EXPECT_EQ(i, sq.size());
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Insert)
{
  {
    size_t sz = 23u;
    sparque<dClass, 3, 3> sq;
    EXPECT_TRUE(sq.empty());

    for (size_t i = 0u; i < sz; ++i)
    {
      auto it = sq.insert(sq.cend(), (int)i);
      EXPECT_EQ(it, --sq.end());
    }

    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(sq[i], (int)i);
    for (size_t i = 0u; i < sz; ++i)
    {
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]) << i;
      EXPECT_EQ(*(sq.end() - (sz - (int)i)), sq[i]) << i;
    }
  }
  {
    size_t sz = 57u;
    sparque<dClass, 2, 5> sq;
    EXPECT_TRUE(sq.empty());

    for (size_t i = 0u; i < sz; ++i)
    {
      dClass dc((int)(sz - i - 1u));
      auto it = sq.insert(sq.begin(), dc);
      EXPECT_EQ(it, sq.begin());
    }
    
    sq.pop_front();
    auto it = sq.insert(sq.begin(), 0);
    EXPECT_EQ(it, sq.begin());
    

    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(sq[i], (int)i);
    for (size_t i = 0u; i < sz; ++i)
    {
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
      EXPECT_EQ(*(sq.end() - (sz - (int)i)), sq[i]);
    }
  }
  {
    size_t sz = 25u;
    sparque<dClass, 5, 2> sq(sz, dClass(0));
    EXPECT_EQ(sq.size(), sz);

    auto it = sq.insert(sq.begin() + 3, 3);
    EXPECT_EQ(it, sq.begin() + 3);
    EXPECT_EQ(*it, sq[3]);

    it = sq.insert(sq.begin() + 6, 6);
    EXPECT_EQ(it, sq.begin() + 6);
    EXPECT_EQ(*it, sq[6]);

    it = sq.insert(sq.begin() + 5, 5);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);
    
    sz = sq.size();
    for (size_t i = 0u; i < sz; ++i)
    {
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
      EXPECT_EQ(*(sq.end() - (sz - (int)i)), sq[i]);
    }
  }
  {
    size_t sz = 9u;
    sparque<dClass, 3> sq(sz, dClass(0));
    EXPECT_EQ(sq.size(), sz);

    auto it = sq.insert(sq.begin() + 5, 5);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);

    it = sq.insert(sq.begin() + 9, 9);
    EXPECT_EQ(it, sq.begin() + 9);
    EXPECT_EQ(*it, sq[9]);
    
    dClass dc = 5;
    it = sq.insert(sq.begin() + 5, dc);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);
    
    sz = sq.size();
    for (size_t i = 0u; i < sz; ++i)
    {
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
      EXPECT_EQ(*(sq.end() - (sz - (int)i)), sq[i]);
    }
  }
  {
    size_t sz = 9u;
    sparque<dClass, 3> sq(sz, dClass(0));
    EXPECT_EQ(sq.size(), sz);

    auto it = sq.insert(sq.begin() + 4, 4);
    EXPECT_EQ(it, sq.begin() + 4);
    EXPECT_EQ(*it, sq[4]);

    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);

    it = sq.insert(sq.begin() + 5, 5);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);

    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);

    it = sq.insert(sq.begin() + 4, 44);
    EXPECT_EQ(it, sq.begin() + 4);
    EXPECT_EQ(*it, sq[4]);
    
    sz = sq.size();
    for (size_t i = 0u; i < sz; ++i)
    {
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
      EXPECT_EQ(*(sq.end() - (sz - (int)i)), sq[i]);
    }
  }
  {
    size_t sz = 410u;
    sparque<dClass, 12> sq(sz, dClass(10));
    
    dClass dc(-10);
    auto it = sq.insert(sq.begin() + 4, std::move(dc));
    EXPECT_EQ(*it, -10);
    EXPECT_EQ(dc, MVE_VAL_CODE); // moved
  }
  {
    srand(852467u);
    int sz = 100;
    sparque<dClass, 4> sq(sz, -100);

    for (const auto& v : sq)
        EXPECT_EQ(v, -100);

    for (int i = 0; i < sz/2; ++i)
    {
      const int delta = (int)(rand() % sq.size());
      auto it = sq.erase(sq.begin() + delta);
      EXPECT_EQ(it, sq.begin() + delta) << "erase at: " << delta;

      int j = 0;
      for (const auto& v : sq)
      {
        if (v != -100) {
          EXPECT_EQ(v, -100) << j << " -> erase at: " << delta;
        }
        ++j;
      }
    }

    for (int i = 0; i < sz/2; ++i)
    {
      const int delta = (int)(rand() % sq.size());
      auto it = sq.insert(sq.begin() + delta, 100);
      EXPECT_EQ(it, sq.begin() + delta) << "insert at: " << delta;

      int j = 0;
      for (const auto& v : sq)
      {
        if (v != -100 && v != 100) {
          EXPECT_TRUE(v == -100 || v == 100) << j << " -> insert at: " << delta;
        }
        ++j;
      }
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Emplace)
{
  {
    size_t sz = 23u;
    sparque<dClass, 3, 3> sq;
    EXPECT_TRUE(sq.empty());
    
    for (size_t i = 0u; i < sz; ++i)
    {
      auto it = sq.emplace(sq.cend(), (int)i);
      EXPECT_EQ(it, --sq.end());
    }
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(sq[i], (int)i);
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]) << i;
  }
  {
    sparque<std::pair<dClass, double>> sq(1);
    
    sq.emplace(sq.begin(), 1, 2.);
    EXPECT_EQ(sq.size(), 2u);
    
    auto pair = std::pair<dClass, double>(1, 2.);
    EXPECT_EQ(sq[0], pair);
  }
  {
    size_t sz = 57u;
    sparque<dClass, 2, 5> sq;
    EXPECT_TRUE(sq.empty());
    
    for (size_t i = 0u; i < sz; ++i)
    {
      dClass dc((int)(sz - i - 1u));
      auto it = sq.emplace(sq.begin(), dc);
      EXPECT_EQ(it, sq.begin());
    }
    
    sq.pop_front();
    auto it = sq.emplace(sq.begin(), 0);
    EXPECT_EQ(it, sq.begin());
    
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(sq[i], (int)i);
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
  }
  {
    size_t sz = 25u;
    sparque<dClass, 5, 2> sq(sz, dClass(0));
    EXPECT_EQ(sq.size(), sz);
    
    auto it = sq.emplace(sq.begin() + 3, 3);
    EXPECT_EQ(it, sq.begin() + 3);
    EXPECT_EQ(*it, sq[3]);
    
    it = sq.emplace(sq.begin() + 6, 6);
    EXPECT_EQ(it, sq.begin() + 6);
    EXPECT_EQ(*it, sq[6]);
    
    it = sq.emplace(sq.begin() + 5, 5);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
  }
  {
    size_t sz = 9u;
    sparque<dClass, 3> sq(sz, dClass(0));
    EXPECT_EQ(sq.size(), sz);
    
    auto it = sq.emplace(sq.begin() + 5, 5);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);
    
    it = sq.emplace(sq.begin() + 9, 9);
    EXPECT_EQ(it, sq.begin() + 9);
    EXPECT_EQ(*it, sq[9]);
    
    dClass dc = 5;
    it = sq.emplace(sq.begin() + 5, dc);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
  }
  {
    size_t sz = 9u;
    sparque<dClass, 3> sq(sz, dClass(0));
    EXPECT_EQ(sq.size(), sz);
    
    auto it = sq.emplace(sq.begin() + 4, 4);
    EXPECT_EQ(it, sq.begin() + 4);
    EXPECT_EQ(*it, sq[4]);
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
    
    it = sq.emplace(sq.begin() + 5, 5);
    EXPECT_EQ(it, sq.begin() + 5);
    EXPECT_EQ(*it, sq[5]);
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
    
    it = sq.emplace(sq.begin() + 4, 44);
    EXPECT_EQ(it, sq.begin() + 4);
    EXPECT_EQ(*it, sq[4]);
    
    for (size_t i = 0u; i < sz; ++i)
      EXPECT_EQ(*(sq.begin() + (int)i), sq[i]);
  }
  {
    srand(852467u);
    int sz = 100;
    sparque<dClass, 4> sq(sz, -100);
    
    for (const auto& v : sq)
      EXPECT_EQ(v, -100);
    
    for (int i = 0; i < sz/2; ++i)
    {
      const int delta = (int)(rand() % sq.size());
      auto it = sq.erase(sq.begin() + delta);
      EXPECT_EQ(it, sq.begin() + delta) << "erase at: " << delta;
      
      int j = 0;
      for (const auto& v : sq)
      {
        if (v != -100) {
          EXPECT_EQ(v, -100) << j << " -> erase at: " << delta;
        }
        ++j;
      }
    }
    
    for (int i = 0; i < sz/2; ++i)
    {
      const int delta = (int)(rand() % sq.size());
      auto it = sq.emplace(sq.begin() + delta, 100);
      EXPECT_EQ(it, sq.begin() + delta) << "emplace at: " << delta;
      
      int j = 0;
      for (const auto& v : sq)
      {
        if (v != -100 && v != 100) {
          EXPECT_TRUE(v == -100 || v == 100) << j << " -> emplace at: " << delta;
        }
        ++j;
      }
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, Resize)
{
 { // decrease size
   sparque<dClass, 3, 2> sq(40);
   EXPECT_EQ(sq.size(), 40u);
   std::iota(sq.begin(), sq.end(), 0);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], i) << i;
   
   sq.resize(30u);
   EXPECT_EQ(sq.size(), 30u);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], i) << i;
   
   sq.resize(2u);
   EXPECT_EQ(sq.size(), 2u);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], i) << i;
   
   sq.resize(1u);
   EXPECT_EQ(sq.size(), 1u);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], i) << i;
   
   sq.resize(0u);
   EXPECT_EQ(sq.size(), 0u);
 }
 { // increase size
   sparque<dClass, 2, 3> sq(10);
   EXPECT_EQ(sq.size(), 10u);
   std::iota(sq.begin(), sq.end(), 0);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], i) << i;
   
   sq.resize(30u, -3);
   EXPECT_EQ(sq.size(), 30u);
   for (int i = 0; i < 10; ++i)
     EXPECT_EQ(sq[i], i) << i;
   for (int i = 10; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], -3) << i;
   
   sq.resize(99u, -5);
   EXPECT_EQ(sq.size(), 99u);
   for (int i = 0; i < 10; ++i)
     EXPECT_EQ(sq[i], i) << i;
   for (int i = 10; i < 30; ++i)
     EXPECT_EQ(sq[i], -3) << i;
   for (int i = 30; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], -5) << i;
 }
 { // both
   sparque<dClass, 2, 2> sq;
   EXPECT_EQ(sq.size(), 0u);
   
   sq.resize(10u, 2);
   EXPECT_EQ(sq.size(), 10u);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], 2) << i;
   
   sq.resize(0u, 3);
   EXPECT_EQ(sq.size(), 0u);
   
   sq.resize(10u, 0);
   EXPECT_EQ(sq.size(), 10u);
   sq.resize(20u, 1);
   EXPECT_EQ(sq.size(), 20u);
   sq.resize(30u, 2);
   EXPECT_EQ(sq.size(), 30u);
   for (int i = 0; i < (int)sq.size(); ++i)
     EXPECT_EQ(sq[i], i/10) << i;
 }
 { // same size
   sparque<dClass, 3, 3> sq(10);
   EXPECT_EQ(sq.size(), 10u);
   sq.resize(sq.size());
   EXPECT_EQ(sq.size(), 10u);
   
   sq.clear();
   EXPECT_EQ(sq.size(), 0u);
   sq.resize(0u);
   EXPECT_EQ(sq.size(), 0u);
 }
 // No object leak
 EXPECT_EQ(dClass::count, dClass::decount);
}

TEST(SparqueTest, SortAll)
{
  srand(852467u);
  int sz = 9999;
  sparque<int> sq(sz);
  std::deque<int> dq(sz);
  
  int i = 0;
  for (auto& v : sq)
  {
    int rnd = rand();
    v = rnd;
    dq[i++] = rnd;
  }
  
  std::sort(sq.begin(), sq.end());
  std::sort(dq.begin(), dq.end());
  
  i = 0;
  for (const auto& v : sq)
    EXPECT_EQ(v, dq[i++]);
}

TEST(SparqueTest, RandomOps)
{
  unsigned int seed = (unsigned int)time(NULL);
  std::cout << "SparqueTest.RandomOps: seed=" << seed << std::endl;
  srand(seed);
  {
    sparque<dClass, 64, 8> sq(2001u);
    std::deque<dClass> dq(sq.size());
    
    for (int i = 0; i < 100000; ++i)
    {
      size_t rng = rand();
      int op = (int)rng % 11;
      switch (op)
      {
        case 0: {
          sq.push_back(1);
          dq.push_back(1);
          break;
        }
        case 1: {
          sq.push_front(-1);
          dq.push_front(-1);
          break;
        }
        case 2: {
          if (!sq.empty()) {
            sq.pop_back();
            dq.pop_back();
          }
          break;
        }
        case 3: {
          if (!sq.empty()) {
            sq.pop_front();
            dq.pop_front();
          }
          break;
        }
        case 4: {
          sq.resize(sq.size() * 3 / 4);
          dq.resize(dq.size() * 3 / 4);
          break;
        }
        case 5: {
          sq.resize(sq.size() * 5 / 4);
          dq.resize(dq.size() * 5 / 4);
          break;
        }
        case 6: {
          sq.assign(dq.crbegin(), dq.crend());
          dq.assign(sq.cbegin(), sq.cend());
          break;
        }
        case 7:
        case 8: {
          if (!sq.empty()) {
            size_t rng2 = (size_t)(((float)rng / RAND_MAX) * (sq.size() - 1u));
            sq.erase(sq.nth(rng2));
            dq.erase(dq.begin() + rng2);
          }
          break;
        }
        case 9:
        case 10: {
          size_t rng2 = (size_t)(((float)rng / RAND_MAX) * sq.size());
          sq.insert(sq.nth(rng2), 2);
          dq.insert(dq.begin() + rng2, 2);
          break;
        }
        default:
          assert(false);
      }
      ASSERT_EQ(sq.size(), dq.size());
      size_t j = 0u;
      for (const auto& v : sq)
        EXPECT_EQ(v, dq[j++]);
    }
  }
  // No object leak
  EXPECT_EQ(dClass::count, dClass::decount);
}
