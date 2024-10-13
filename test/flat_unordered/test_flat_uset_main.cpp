/**
 * Copyright 2024 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#include "gtest/gtest.h"

#define INDIVI_FLAT_U_DEBUG
#include "indivi/flat_uset.h"
#include "utils/debug_utils.h"

#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cassert>
#include <cstdlib>

using namespace indivi;


TEST(FlatUSetTest, Constructor)
{
  {
    flat_uset<DbgClass> fus;
    EXPECT_FALSE(fus.contains(1));
  }
  {
    enum EN { AA, BB };
    
    flat_uset<EN> fus;
    EXPECT_FALSE(fus.contains(AA));
  }
  {
    flat_uset<std::shared_ptr<int>> fus;
    EXPECT_FALSE(fus.contains(nullptr));
  }
  {
    flat_uset<std::unique_ptr<int>> fus;
    EXPECT_FALSE(fus.contains(nullptr));
  }
  {
    flat_uset<int*> fus;
    EXPECT_FALSE(fus.contains(nullptr));
  }
  {
    flat_uset<int> fus;
    EXPECT_FALSE(fus.contains(0));
  }
  {
    flat_uset<std::string> fus;
    EXPECT_FALSE(fus.contains(""));
  }
  {
    flat_uset<const char*> fus;
    EXPECT_FALSE(fus.contains(""));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Constructor2)
{
  {
    flat_uset<DbgClass> fus(10);
    EXPECT_GE(fus.bucket_count(), 10u);
    EXPECT_FALSE(fus.contains(1));
  }
  {
    flat_uset<DbgClass, std::hash<DbgClass>> fus(0, std::hash<DbgClass>());
    EXPECT_FALSE(fus.contains(1));
  }
  {
    flat_uset<DbgClass, std::hash<DbgClass>, std::equal_to<DbgClass>> fus(0, std::hash<DbgClass>(), std::equal_to<DbgClass>());
    EXPECT_FALSE(fus.contains(1));
  }
  {
    std::vector<DbgClass> vec{1, 3};
    flat_uset<DbgClass> fus(vec.begin(), vec.end());
    EXPECT_EQ(fus.size(), 2u);
    EXPECT_TRUE(fus.contains(1));
    EXPECT_TRUE(fus.contains(3));
  }
  {
    flat_uset<DbgClass> fus{1};
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_TRUE(fus.contains(1));
    EXPECT_FALSE(fus.contains(2));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Constructor3)
{
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2 = fus1;
    EXPECT_EQ(fus1.size(), 0u);
    EXPECT_EQ(fus2.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2 = fus1;
    EXPECT_TRUE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
  }
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2 = std::move(fus1);
    EXPECT_EQ(fus1.size(), 0u);
    EXPECT_EQ(fus2.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2 = std::move(fus1);
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Assignment)
{
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2;
    fus2 = fus1;
    EXPECT_TRUE(fus1.empty());
    EXPECT_TRUE(fus2.empty());
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2;
    fus2 = fus1;
    EXPECT_TRUE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
  }
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2{1};
    fus2 = fus1;
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_FALSE(fus2.contains(1));
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2{{2, 4}};
    EXPECT_FALSE(fus2.contains(1));
    EXPECT_TRUE(fus2.contains(2));
    fus2 = fus1;
    EXPECT_TRUE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
    EXPECT_FALSE(fus2.contains(2));
  }
  {
    flat_uset<DbgClass> fus;
    EXPECT_TRUE(fus.empty());
    fus = {1, 3};
    EXPECT_TRUE(fus.contains(1));
    EXPECT_TRUE(fus.contains(3));
    fus = {};
    EXPECT_FALSE(fus.contains(1));
    EXPECT_FALSE(fus.contains(3));
  }
  {
    flat_uset<DbgClass> fus{{1, 4}};
    fus = {1, 3};
    EXPECT_TRUE(fus.contains(1));
    EXPECT_TRUE(fus.contains(3));
    fus = {};
    EXPECT_FALSE(fus.contains(1));
    EXPECT_FALSE(fus.contains(3));
    EXPECT_TRUE(fus.empty());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Assignment2)
{
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2;
    fus2 = std::move(fus1);
    EXPECT_TRUE(fus1.empty());
    EXPECT_TRUE(fus2.empty());
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2;
    fus2 = std::move(fus1);
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
  }
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2{1};
    fus2 = std::move(fus1);
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_FALSE(fus2.contains(1));
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2{1, 4};
    fus2 = std::move(fus1);
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
    EXPECT_FALSE(fus2.contains(4));
  }
  {
    flat_uset<int> fus1{1, 6};
    flat_uset<int> fus2{1, 4};
    fus2 = fus1;
    EXPECT_TRUE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
    EXPECT_FALSE(fus2.contains(4));
    EXPECT_TRUE(fus2.contains(6));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Capacity)
{
  {
    flat_uset<DbgClass> fus;
    EXPECT_TRUE(fus.empty());
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 0u);
    EXPECT_EQ(fus.load_factor(), 0.f);
    
    auto max_size = (flat_uset<DbgClass>::size_type)(fus.max_bucket_count() * fus.max_load_factor());
    EXPECT_EQ(fus.max_load_factor(), 0.875f);
    EXPECT_EQ(fus.max_size(), max_size);
    EXPECT_GT(fus.max_bucket_count(), 0u);
    fus.max_load_factor(0.f); // no-op
    EXPECT_GT(fus.max_bucket_count(), 0u);
    
    fus = {1, 3};
    EXPECT_EQ(fus.size(), 2u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    EXPECT_EQ(fus.load_factor(), 1.f);
  }
  {
    flat_uset<DbgClass> fus{{1, 1}};
    EXPECT_FALSE(fus.empty());
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    EXPECT_EQ(fus.load_factor(), 0.5f);
    
    fus.insert(2);
    EXPECT_EQ(fus.size(), 2u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    EXPECT_EQ(fus.load_factor(), 1.f);
    
    fus.insert(3);
    EXPECT_EQ(fus.size(), 3u);
    EXPECT_EQ(fus.bucket_count(), 4u);
    EXPECT_EQ(fus.load_factor(), 0.75f);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Capacity2)
{
  {
    flat_uset<DbgClass> fus;
    fus.reserve(0);
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 0u);
    EXPECT_EQ(fus.load_factor(), 0.f);
  }
  {
    flat_uset<DbgClass> fus;
    fus.reserve(7);
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 8u);
    EXPECT_EQ(fus.load_factor(), 0.f);
  }
  {
    flat_uset<DbgClass> fus;
    fus.reserve(31);
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 64u);
    EXPECT_EQ(fus.load_factor(), 0.f);
  }
  {
    flat_uset<DbgClass> fus;
    fus.rehash(12);
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 16u);
    EXPECT_EQ(fus.load_factor(), 0.f);
  }
  {
    flat_uset<DbgClass> fus;
    fus.rehash(31);
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 32u);
    EXPECT_EQ(fus.load_factor(), 0.f);
  }
  {
    flat_uset<DbgClass> fus(3);
    fus.insert(1);
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 4u);
    
    fus.rehash(0);
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    
    fus.clear();
    fus.rehash(0);
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 0u);
    
    fus.insert(3);
    EXPECT_TRUE(fus.contains(3));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    fus.rehash(4);
    EXPECT_TRUE(fus.contains(3));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 4u);
    
    fus.reserve(4);
    EXPECT_TRUE(fus.contains(3));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 4u);
    
    fus.reserve(5);
    EXPECT_TRUE(fus.contains(3));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 8u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Observers)
{
  {
    flat_uset<DbgClass> fus;
    auto hash = fus.hash_function();
    auto keq = fus.key_eq();
    
    EXPECT_NE(hash(1), 1);
    EXPECT_TRUE(keq(1, 1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Iterator)
{
  {
    flat_uset<DbgClass> fus;
    EXPECT_EQ(fus.begin(), fus.end());
    EXPECT_EQ(fus.cbegin(), fus.cend());
    EXPECT_EQ(fus.cbegin(), fus.end());
    EXPECT_EQ(fus.begin(), fus.cend());
    EXPECT_EQ(fus.begin(), fus.cbegin());
    EXPECT_EQ(fus.end(), fus.cend());
  }
  {
    flat_uset<DbgClass> fus{1};
    EXPECT_NE(fus.begin(), fus.end());
    EXPECT_NE(fus.cbegin(), fus.cend());
    EXPECT_EQ(++fus.begin(), fus.end());
    EXPECT_EQ(++fus.cbegin(), fus.cend());
    
    auto it = fus.begin();
    EXPECT_EQ(*it, 1);
    
    fus.insert(3);
    it = fus.begin();
    EXPECT_TRUE(*it == 3 || *it == 1);
  }
  {
    flat_uset<DbgClass> fus{1, 3};
    auto it1 = fus.find(1);
    EXPECT_NE(it1, fus.end());
    auto it2 = fus.find(3);
    EXPECT_NE(it2, fus.end());
    EXPECT_NE(it2, it1);
    auto it3 = fus.find(5);
    EXPECT_EQ(it3, fus.end());
    EXPECT_NE(it3, it1);
    EXPECT_NE(it3, it2);
  }
  {
    flat_uset<DbgClass> fus;
    for (int i = 1; i <= 111; ++i)
      fus.insert(i);
    
    int count = 0;
    for (const auto& val : fus)
    {
      EXPECT_TRUE(fus.contains(val));
      ++count;
    }
    EXPECT_EQ(count, (int)fus.size());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, ElementAccess)
{
  {
    flat_uset<DbgClass> fus{1, 3, 5};
    EXPECT_TRUE(fus.contains(5));
    EXPECT_FALSE(fus.contains(6));
    EXPECT_EQ(fus.count(1), 1u);
    EXPECT_EQ(fus.count(2), 0u);
    
    auto it = fus.find(3);
    ASSERT_NE(it, fus.end());
    EXPECT_EQ(*it, 3);
    
    it = fus.find(2);
    EXPECT_EQ(it, fus.end());
  }
  {
    flat_uset<std::string> fus{"1", "", "3"};
    EXPECT_TRUE(fus.contains("1"));
    EXPECT_TRUE(fus.contains("3"));
    EXPECT_TRUE(fus.contains(""));
    EXPECT_FALSE(fus.contains("2"));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, ElementAccess2)
{
  {
    flat_uset<DbgClass> fus;
    DbgClass k(1);
    
    fus.emplace(k);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(fus.count(k), 1u);
    EXPECT_EQ(fus.size(), 1u);
    
    fus.insert(std::move(k));
    EXPECT_EQ(k, 1);
    EXPECT_EQ(fus.count(1), 1u);
    EXPECT_EQ(fus.size(), 1u);
    
    DbgClass k2(2);
    fus.insert(std::move(k2));
    EXPECT_NE(k2, 2);
    EXPECT_EQ(fus.count(2), 1u);
    EXPECT_EQ(fus.size(), 2u);
  }
  {
    flat_uset<std::string> fus;
    auto p1 = fus.insert("a");
    EXPECT_EQ(*p1.first, "a");
    EXPECT_TRUE(p1.second);
    EXPECT_EQ(fus.size(), 1u);
    
    auto p2 = fus.insert("a");
    EXPECT_EQ(*p2.first, "a");
    EXPECT_FALSE(p2.second);
    EXPECT_EQ(fus.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Clear)
{
  {
    flat_uset<DbgClass> fus;
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 0u);
    
    fus.clear();
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 0u);
  }
  {
    flat_uset<DbgClass> fus{1, 3, 5};
    EXPECT_EQ(fus.size(), 3u);
    EXPECT_EQ(fus.bucket_count(), 4u);
    
    fus.clear();
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 4u);
    fus.clear();
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 4u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Insert)
{
  {
    flat_uset<DbgClass> fus;
    EXPECT_FALSE(fus.contains(1));
    auto it = fus.insert(1);
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fus.contains(1));
    
    fus.insert(3);
    EXPECT_TRUE(fus.contains(3));
    
    it = fus.insert(3);
    EXPECT_TRUE(fus.contains(3));
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, 3);
    EXPECT_FALSE(it.second);
  }
  {
    flat_uset<DbgClass> fus;
    DbgClass v(10);
    
    fus.insert(v);
    EXPECT_TRUE(fus.contains(10));
    EXPECT_EQ(v, 10);
  }
  {
    flat_uset<DbgClass> fus;
    const DbgClass v(10);
    
    fus.insert(v);
    EXPECT_TRUE(fus.contains(10));
    EXPECT_EQ(v, 10);
  }
  {
    flat_uset<DbgClass> fus;
    DbgClass v(10);

    fus.insert(std::move(v));
    EXPECT_TRUE(fus.contains(10));
    EXPECT_NE(v, 10);
  }
  {
    flat_uset<DbgClass> fus;
    const DbgClass v(10);
    
    fus.insert(std::move(v));
    EXPECT_TRUE(fus.contains(10));
    EXPECT_EQ(v, 10);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Insert2)
{
  {
    std::vector<DbgClass> vec{1, 1};
    flat_uset<DbgClass> fus;
    fus.insert(vec.begin(), vec.end());
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0], 1);
  }
  {
    std::vector<DbgClass> vec{1, 1};
    flat_uset<DbgClass> fus;
    fus.insert(std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
    
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_NE(vec[0], 1);
    EXPECT_EQ(vec[1], 1);
  }
  {
    flat_uset<DbgClass> fus;
    std::initializer_list<DbgClass> ilist{1, 1};
    fus.insert(ilist);
    
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(ilist.size(), 2u);
    EXPECT_EQ(*ilist.begin(), 1);
  }
  {
    flat_uset<int> fus;
    fus.insert({1, 1});
    
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Emplace)
{
  {
    flat_uset<DbgClass> fus;
    auto it = fus.emplace(1);
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
    
    it = fus.emplace(1);
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_FALSE(it.second);
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
    
    it = fus.emplace(2);
    ASSERT_NE(it.first, fus.end());
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fus.contains(2));
    EXPECT_EQ(fus.size(), 2u);
  }
  {
    flat_uset<DbgClass> fus;
    DbgClass k(1);
    
    auto it = fus.emplace(k);
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(k, 1);
  }
  {
    flat_uset<DbgClass> fus;
    DbgClass k(1);
    
    auto it = fus.emplace(std::move(k));
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fus.contains(1));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_NE(k, 1);
  }
  {
    flat_uset<std::string> fus;
    const char* k = "1";

    auto it = fus.emplace(k);
    ASSERT_NE(it.first, fus.end());
    EXPECT_EQ(*it.first, k);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fus.contains(k));
    EXPECT_EQ(fus.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Erase)
{
  {
    flat_uset<DbgClass> fus{1, 3};
    EXPECT_EQ(fus.erase(1), 1u);
    EXPECT_FALSE(fus.contains(1));
    EXPECT_TRUE(fus.contains(3));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    
    EXPECT_EQ(fus.erase(1), 0u);
    EXPECT_FALSE(fus.contains(1));
    EXPECT_TRUE(fus.contains(3));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 2u);
    
    EXPECT_EQ(fus.erase(3), 1u);
    EXPECT_FALSE(fus.contains(1));
    EXPECT_FALSE(fus.contains(3));
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 2u);
  }
  {
    flat_uset<DbgClass> fus;
    EXPECT_EQ(fus.erase(1), 0u);
    EXPECT_FALSE(fus.contains(1));
    EXPECT_EQ(fus.size(), 0u);
    EXPECT_EQ(fus.bucket_count(), 0u);
    
    fus.emplace(2);
    EXPECT_EQ(fus.erase(1), 0u);
    EXPECT_TRUE(fus.contains(2));
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_EQ(fus.bucket_count(), 2u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, EraseIter)
{
  {
    flat_uset<DbgClass> fus{1, 3};
    auto first = fus.begin();
    bool is1 = *first == 1;
    
    fus.erase(first);
    EXPECT_EQ(fus.contains(1), !is1);
    EXPECT_EQ(fus.contains(3), is1);
    EXPECT_EQ(fus.size(), 1u);
    
    first = fus.begin();
    fus.erase(first);
    EXPECT_FALSE(fus.contains(1));
    EXPECT_FALSE(fus.contains(3));
    EXPECT_EQ(fus.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus{1, 3};
    auto first = fus.begin();
    bool is1 = *first == 1;
    
    auto it = fus.erase_(first);
    ASSERT_NE(it, fus.end());
    EXPECT_EQ(*it, is1 ? 3 : 1);
    EXPECT_EQ(fus.contains(1), !is1);
    EXPECT_EQ(fus.contains(3), is1);
    EXPECT_EQ(fus.size(), 1u);
    
    first = fus.begin();
    it = fus.erase_(first);
    ASSERT_EQ(it, fus.end());
    EXPECT_FALSE(fus.contains(1));
    EXPECT_FALSE(fus.contains(3));
    EXPECT_EQ(fus.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus;
    for (int i = 1; i <= 100; ++i)
      fus.emplace(i);
    EXPECT_EQ(fus.size(), 100u);
    
    auto it = fus.begin();
    while (!fus.empty())
      it = fus.erase_(it);
    EXPECT_EQ(fus.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, EraseIf)
{
  {
    flat_uset<DbgClass> fus;
    
    erase_if(fus, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fus.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus{1, 3};
    
    erase_if(fus, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fus.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus{1, 2, 3};
    
    erase_if(fus, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fus.size(), 1u);
    EXPECT_FALSE(fus.contains(1));
    EXPECT_TRUE(fus.contains(2));
    EXPECT_FALSE(fus.contains(3));
  }
  {
    flat_uset<DbgClass> fus{2, 4};
    
    erase_if(fus, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fus.size(), 2u);
    EXPECT_TRUE(fus.contains(2));
    EXPECT_TRUE(fus.contains(4));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Swap)
{
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2;
    
    fus2.swap(fus1);
    EXPECT_EQ(fus1.size(), 0u);
    EXPECT_EQ(fus2.size(), 0u);
    
    fus2.swap(fus1);
    EXPECT_EQ(fus1.size(), 0u);
    EXPECT_EQ(fus2.size(), 0u);
  }
  {
    flat_uset<DbgClass> fus1{1, 3, 5};
    flat_uset<DbgClass> fus2;

    fus2.swap(fus1);
    EXPECT_EQ(fus1.size(), 0u);
    EXPECT_EQ(fus2.size(), 3u);
    EXPECT_FALSE(fus1.contains(3));
    EXPECT_TRUE(fus2.contains(3));

    fus2.swap(fus1);
    EXPECT_EQ(fus1.size(), 3u);
    EXPECT_EQ(fus2.size(), 0u);
    EXPECT_TRUE(fus1.contains(3));
    EXPECT_FALSE(fus2.contains(3));
  }
  {
    flat_uset<DbgClass> fus1{1, 3};
    flat_uset<DbgClass> fus2{5};
    
    fus2.swap(fus1);
    EXPECT_EQ(fus1.size(), 1u);
    EXPECT_EQ(fus2.size(), 2u);
    EXPECT_TRUE(fus1.contains(5));
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
    EXPECT_FALSE(fus2.contains(5));
    
    fus2.swap(fus1);
    EXPECT_EQ(fus1.size(), 2u);
    EXPECT_EQ(fus2.size(), 1u);
    EXPECT_TRUE(fus1.contains(3));
    EXPECT_FALSE(fus1.contains(5));
    EXPECT_TRUE(fus2.contains(5));
    EXPECT_FALSE(fus2.contains(3));
  }
  {
    using std::swap;
    
    flat_uset<DbgClass> fus1{1, 3};
    flat_uset<DbgClass> fus2{5};

    swap(fus1, fus2);
    EXPECT_EQ(fus1.size(), 1u);
    EXPECT_EQ(fus2.size(), 2u);
    EXPECT_TRUE(fus1.contains(5));
    EXPECT_FALSE(fus1.contains(1));
    EXPECT_TRUE(fus2.contains(1));
    EXPECT_FALSE(fus2.contains(5));

    swap(fus1, fus2);
    EXPECT_EQ(fus1.size(), 2u);
    EXPECT_EQ(fus2.size(), 1u);
    EXPECT_TRUE(fus1.contains(3));
    EXPECT_FALSE(fus1.contains(5));
    EXPECT_TRUE(fus2.contains(5));
    EXPECT_FALSE(fus2.contains(3));
  }
  {
    using std::swap;
    
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2{5, 4};
    
    swap(fus1, fus1);
    EXPECT_EQ(fus1.size(), 1u);
    EXPECT_TRUE(fus1.contains(1));
    
    swap(fus2, fus1);
    fus1.swap(fus2);
    EXPECT_EQ(fus1.size(), 1u);
    EXPECT_EQ(fus2.size(), 2u);
    EXPECT_TRUE(fus1.contains(1));
    EXPECT_FALSE(fus1.contains(5));
    EXPECT_TRUE(fus2.contains(5));
    EXPECT_FALSE(fus2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, Equality)
{
  {
    flat_uset<DbgClass> fus1;
    flat_uset<DbgClass> fus2;
    EXPECT_EQ(fus1, fus2);
    EXPECT_EQ(fus2, fus1);
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2{1};
    EXPECT_EQ(fus1, fus2);
    EXPECT_EQ(fus2, fus1);
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2{5};
    EXPECT_NE(fus1, fus2);
    EXPECT_NE(fus2, fus1);
  }
  {
    flat_uset<DbgClass> fus1{};
    flat_uset<DbgClass> fus2{1};
    EXPECT_NE(fus1, fus2);
    EXPECT_NE(fus2, fus1);
  }
  {
    flat_uset<DbgClass> fus1{1};
    flat_uset<DbgClass> fus2{1, 3};
    EXPECT_NE(fus1, fus2);
    EXPECT_NE(fus2, fus1);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUSetTest, BadHash)
{
  struct bad_hash {
    size_t operator()(const int& x) const { return x; }
  };
  
  flat_uset<int, bad_hash> fus;
  std::unordered_set<int, bad_hash> set;
  for (int i = 0; i < 100000; ++i)
  {
    fus.emplace(i);
    set.emplace(i);
  }
  EXPECT_EQ(fus.size(), set.size());
  
  for (const auto& item : set)
  {
    auto itF = fus.find(item);
    ASSERT_NE(itF, fus.end());
  }
}

TEST(FlatUSetTest, Stress)
{
  {
    auto seed = time(NULL);
    auto get_rand = []() {
      int r = rand() + 1;
      return (RAND_MAX > 32768) ? r : r * rand() + 1;
    };
    
    std::cout << "Stress seed: " << seed << "\n";
    srand((unsigned int)seed);

    flat_uset<DbgClass> fus;
    std::unordered_set<DbgClass> set;
    std::vector<DbgClass> vec;
    
    int addCount = 0;
    int findCount = 0;
    int removeCount = 0;
    int rehashCount = 0;

    for (int i = 0; i < 1000000; ++i)
    {
      int op = rand() % 8;
      switch(op)
      {
        case 0: // insert
        {
          int k = get_rand();
          fus.insert(k);
          set.insert(k);
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fus.size(), set.size()) << "insert";
          break;
        }
        case 1: // emplace
        case 2:
        {
          int k = get_rand();
          fus.emplace(k);
          set.emplace(k);
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fus.size(), set.size()) << "emplace";
          break;
        }
        case 3: // count
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            EXPECT_EQ(fus.count(k), set.count(k));
            ++findCount;
          }
          EXPECT_EQ(fus.size(), set.size()) << "count";
          break;
        }
        case 4: // erase
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto countF = fus.erase(k);
            auto countS = set.erase(k);
            ++removeCount;
            EXPECT_EQ(countF, countS);
          }
          EXPECT_EQ(fus.size(), set.size()) << "erase";
          break;
        }
        case 5: // erase first
        {
          if (!set.empty())
          {
            auto itS = set.begin();
            auto itF = fus.find(*itS);
            ASSERT_NE(itF, fus.end());
            ASSERT_NE(itS, set.end());
            fus.erase_(itF);
            set.erase(itS);
            ++removeCount;
          }
          EXPECT_EQ(fus.size(), set.size()) << "erase first";
          break;
        }
        case 6: // find
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto itF = fus.find(k);
            auto itS = set.find(k);
            if (itF != fus.end())
            {
              EXPECT_NE(itS, set.end());
              EXPECT_EQ(*itF, *itS) << "find";
            }
            else
            {
              EXPECT_EQ(itS, set.end()) << "find none";
            }
            ++findCount;
          }
          break;
        }
        case 7: // rehash
        {
          if (rand() < RAND_MAX / 50)
          {
            auto j = vec.size() ? rand() % vec.size() : 0;
            fus.rehash(j);
            ++rehashCount;
          }
          EXPECT_EQ(fus.size(), set.size()) << "rehash";
          break;
        }
        default:
          assert(false);
      }
    }
    std::cout << "Stress final size: " << set.size()
              << ", add: " << addCount
              << ", find: " << findCount
              << ", remove: " << removeCount
              << ", rehash: " << rehashCount << "\n";
    EXPECT_EQ(fus.size(), set.size()) << "final";

    for (const auto& item : set)
    {
      auto itF = fus.find(item);
      ASSERT_NE(itF, fus.end());
    }
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}
