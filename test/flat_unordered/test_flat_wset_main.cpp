/**
 * Copyright 2025 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#include "gtest/gtest.h"

#define INDIVI_FLAT_W_DEBUG
#include "indivi/flat_wset.h"
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

TEST(FlatWSetTest, Constructor)
{
  {
    flat_wset<DbgClass> fws;
    EXPECT_FALSE(fws.contains(1));
  }
  {
    enum EN { AA, BB };
    
    flat_wset<EN> fws;
    EXPECT_FALSE(fws.contains(AA));
  }
  {
    flat_wset<std::shared_ptr<int>> fws;
    EXPECT_FALSE(fws.contains(nullptr));
  }
  {
    flat_wset<std::unique_ptr<int>> fws;
    EXPECT_FALSE(fws.contains(nullptr));
  }
  {
    flat_wset<int*> fws;
    EXPECT_FALSE(fws.contains(nullptr));
  }
  {
    flat_wset<int> fws;
    EXPECT_FALSE(fws.contains(0));
  }
  {
    flat_wset<std::string> fws;
    EXPECT_FALSE(fws.contains(""));
  }
  {
    flat_wset<const char*> fws;
    EXPECT_FALSE(fws.contains(""));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Constructor2)
{
  {
    flat_wset<DbgClass> fws(10);
    EXPECT_GE(fws.bucket_count(), 10u);
    EXPECT_FALSE(fws.contains(1));
  }
  {
    flat_wset<DbgClass, std::hash<DbgClass>> fws(0, std::hash<DbgClass>());
    EXPECT_FALSE(fws.contains(1));
  }
  {
    flat_wset<DbgClass, std::hash<DbgClass>, std::equal_to<DbgClass>> fws(0, std::hash<DbgClass>(), std::equal_to<DbgClass>());
    EXPECT_FALSE(fws.contains(1));
  }
  {
    std::vector<DbgClass> vec{1, 3};
    flat_wset<DbgClass> fws(vec.begin(), vec.end());
    EXPECT_EQ(fws.size(), 2u);
    EXPECT_TRUE(fws.contains(1));
    EXPECT_TRUE(fws.contains(3));
  }
  {
    flat_wset<DbgClass> fws{1};
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_TRUE(fws.contains(1));
    EXPECT_FALSE(fws.contains(2));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Constructor3)
{
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2 = fws1;
    EXPECT_EQ(fws1.size(), 0u);
    EXPECT_EQ(fws2.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2 = fws1;
    EXPECT_TRUE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
  }
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2 = std::move(fws1);
    EXPECT_EQ(fws1.size(), 0u);
    EXPECT_EQ(fws2.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2 = std::move(fws1);
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Assignment)
{
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2;
    fws2 = fws1;
    EXPECT_TRUE(fws1.empty());
    EXPECT_TRUE(fws2.empty());
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2;
    fws2 = fws1;
    EXPECT_TRUE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
  }
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2{1};
    fws2 = fws1;
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_FALSE(fws2.contains(1));
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2{{2, 4}};
    EXPECT_FALSE(fws2.contains(1));
    EXPECT_TRUE(fws2.contains(2));
    fws2 = fws1;
    EXPECT_TRUE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
    EXPECT_FALSE(fws2.contains(2));
  }
  {
    flat_wset<DbgClass> fws;
    EXPECT_TRUE(fws.empty());
    fws = {1, 3};
    EXPECT_TRUE(fws.contains(1));
    EXPECT_TRUE(fws.contains(3));
    fws = {};
    EXPECT_FALSE(fws.contains(1));
    EXPECT_FALSE(fws.contains(3));
  }
  {
    flat_wset<DbgClass> fws{{1, 4}};
    fws = {1, 3};
    EXPECT_TRUE(fws.contains(1));
    EXPECT_TRUE(fws.contains(3));
    fws = {};
    EXPECT_FALSE(fws.contains(1));
    EXPECT_FALSE(fws.contains(3));
    EXPECT_TRUE(fws.empty());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Assignment2)
{
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2;
    fws2 = std::move(fws1);
    EXPECT_TRUE(fws1.empty());
    EXPECT_TRUE(fws2.empty());
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2;
    fws2 = std::move(fws1);
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
  }
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2{1};
    fws2 = std::move(fws1);
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_FALSE(fws2.contains(1));
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2{1, 4};
    fws2 = std::move(fws1);
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
    EXPECT_FALSE(fws2.contains(4));
  }
  {
    flat_wset<int> fws1{1, 6};
    flat_wset<int> fws2{1, 4};
    fws2 = fws1;
    EXPECT_TRUE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
    EXPECT_FALSE(fws2.contains(4));
    EXPECT_TRUE(fws2.contains(6));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Capacity)
{
  {
    flat_wset<DbgClass> fws;
    EXPECT_TRUE(fws.empty());
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 0u);
    EXPECT_EQ(fws.load_factor(), 0.f);
    
    auto max_size = (flat_wset<DbgClass>::size_type)(fws.max_bucket_count() * fws.max_load_factor());
    EXPECT_EQ(fws.max_load_factor(), 0.8f);
    EXPECT_EQ(fws.max_size(), max_size);
    EXPECT_GT(fws.max_bucket_count(), 0u);
    fws.max_load_factor(0.f); // no-op
    EXPECT_GT(fws.max_bucket_count(), 0u);
    
    fws = {1, 3};
    EXPECT_EQ(fws.size(), 2u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    EXPECT_EQ(fws.load_factor(), 1.f);
  }
  {
    flat_wset<DbgClass> fws{{1, 1}};
    EXPECT_FALSE(fws.empty());
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    EXPECT_EQ(fws.load_factor(), 0.5f);
    
    fws.insert(2);
    EXPECT_EQ(fws.size(), 2u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    EXPECT_EQ(fws.load_factor(), 1.f);
    
    fws.insert(3);
    EXPECT_EQ(fws.size(), 3u);
    EXPECT_EQ(fws.bucket_count(), 4u);
    EXPECT_EQ(fws.load_factor(), 0.75f);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Capacity2)
{
  {
    flat_wset<DbgClass> fws;
    fws.reserve(0);
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 0u);
    EXPECT_EQ(fws.load_factor(), 0.f);
  }
  {
    flat_wset<DbgClass> fws;
    fws.reserve(7);
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 8u);
    EXPECT_EQ(fws.load_factor(), 0.f);
  }
  {
    flat_wset<DbgClass> fws;
    fws.reserve(31);
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 64u);
    EXPECT_EQ(fws.load_factor(), 0.f);
  }
  {
    flat_wset<DbgClass> fws;
    fws.rehash(12);
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 16u);
    EXPECT_EQ(fws.load_factor(), 0.f);
  }
  {
    flat_wset<DbgClass> fws;
    fws.rehash(31);
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 32u);
    EXPECT_EQ(fws.load_factor(), 0.f);
  }
  {
    flat_wset<DbgClass> fws(3);
    fws.insert(1);
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 4u);
    
    fws.rehash(0);
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    
    fws.clear();
    fws.rehash(0);
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 0u);
    
    fws.insert(3);
    EXPECT_TRUE(fws.contains(3));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    fws.rehash(4);
    EXPECT_TRUE(fws.contains(3));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 4u);
    
    fws.reserve(4);
    EXPECT_TRUE(fws.contains(3));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 4u);
    
    fws.reserve(5);
    EXPECT_TRUE(fws.contains(3));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 8u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Observers)
{
  {
    flat_wset<DbgClass> fws;
    auto hash = fws.hash_function();
    auto keq = fws.key_eq();
    
    EXPECT_NE(hash(1), 1);
    EXPECT_TRUE(keq(1, 1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Iterator)
{
  {
    flat_wset<DbgClass> fws;
    EXPECT_EQ(fws.begin(), fws.end());
    EXPECT_EQ(fws.cbegin(), fws.cend());
    EXPECT_EQ(fws.cbegin(), fws.end());
    EXPECT_EQ(fws.begin(), fws.cend());
    EXPECT_EQ(fws.begin(), fws.cbegin());
    EXPECT_EQ(fws.end(), fws.cend());
  }
  {
    flat_wset<DbgClass> fws{1};
    EXPECT_NE(fws.begin(), fws.end());
    EXPECT_NE(fws.cbegin(), fws.cend());
    EXPECT_EQ(++fws.begin(), fws.end());
    EXPECT_EQ(++fws.cbegin(), fws.cend());
    
    auto it = fws.begin();
    EXPECT_EQ(*it, 1);
    
    fws.insert(3);
    it = fws.begin();
    EXPECT_TRUE(*it == 3 || *it == 1);
  }
  {
    flat_wset<DbgClass> fws{1, 3};
    auto it1 = fws.find(1);
    EXPECT_NE(it1, fws.end());
    auto it2 = fws.find(3);
    EXPECT_NE(it2, fws.end());
    EXPECT_NE(it2, it1);
    auto it3 = fws.find(5);
    EXPECT_EQ(it3, fws.end());
    EXPECT_NE(it3, it1);
    EXPECT_NE(it3, it2);
  }
  {
    flat_wset<DbgClass> fws;
    for (int i = 1; i <= 111; ++i)
      fws.insert(i);
    
    int count = 0;
    for (const auto& val : fws)
    {
      EXPECT_TRUE(fws.contains(val));
      ++count;
    }
    EXPECT_EQ(count, (int)fws.size());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, ElementAccess)
{
  {
    flat_wset<DbgClass> fws{1, 3, 5};
    EXPECT_TRUE(fws.contains(5));
    EXPECT_FALSE(fws.contains(6));
    EXPECT_EQ(fws.count(1), 1u);
    EXPECT_EQ(fws.count(2), 0u);
    
    auto it = fws.find(3);
    ASSERT_NE(it, fws.end());
    EXPECT_EQ(*it, 3);
    
    it = fws.find(2);
    EXPECT_EQ(it, fws.end());
  }
  {
    flat_wset<std::string> fws{"1", "", "3"};
    EXPECT_TRUE(fws.contains("1"));
    EXPECT_TRUE(fws.contains("3"));
    EXPECT_TRUE(fws.contains(""));
    EXPECT_FALSE(fws.contains("2"));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

// TEST(FlatWSetTest, ElementAccess2)
// {
//   {
//     flat_wset<DbgClass> fws;
//     DbgClass k(1);
    
//     fws.emplace(k);
//     EXPECT_EQ(k, 1);
//     EXPECT_EQ(fws.count(k), 1u);
//     EXPECT_EQ(fws.size(), 1u);
    
//     fws.insert(std::move(k));
//     EXPECT_EQ(k, 1);
//     EXPECT_EQ(fws.count(1), 1u);
//     EXPECT_EQ(fws.size(), 1u);
    
//     DbgClass k2(2);
//     fws.insert(std::move(k2));
//     EXPECT_NE(k2, 2);
//     EXPECT_EQ(fws.count(2), 1u);
//     EXPECT_EQ(fws.size(), 2u);
//   }
//   {
//     flat_wset<std::string> fws;
//     auto p1 = fws.insert("a");
//     EXPECT_EQ(*p1.first, "a");
//     EXPECT_TRUE(p1.second);
//     EXPECT_EQ(fws.size(), 1u);
    
//     auto p2 = fws.insert("a");
//     EXPECT_EQ(*p2.first, "a");
//     EXPECT_FALSE(p2.second);
//     EXPECT_EQ(fws.size(), 1u);
//   }
//   // No object leak
//   EXPECT_EQ(DbgClass::count, 0);
// }

TEST(FlatWSetTest, Clear)
{
  {
    flat_wset<DbgClass> fws;
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 0u);
    
    fws.clear();
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 0u);
  }
  {
    flat_wset<DbgClass> fws{1, 3, 5};
    EXPECT_EQ(fws.size(), 3u);
    EXPECT_EQ(fws.bucket_count(), 4u);
    
    fws.clear();
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 4u);
    fws.clear();
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 4u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Insert)
{
  {
    flat_wset<DbgClass> fws;
    EXPECT_FALSE(fws.contains(1));
    auto it = fws.insert(1);
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fws.contains(1));
    
    fws.insert(3);
    EXPECT_TRUE(fws.contains(3));
    
    it = fws.insert(3);
    EXPECT_TRUE(fws.contains(3));
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, 3);
    EXPECT_FALSE(it.second);
  }
  {
    flat_wset<DbgClass> fws;
    DbgClass v(10);
    
    fws.insert(v);
    EXPECT_TRUE(fws.contains(10));
    EXPECT_EQ(v, 10);
  }
  {
    flat_wset<DbgClass> fws;
    const DbgClass v(10);
    
    fws.insert(v);
    EXPECT_TRUE(fws.contains(10));
    EXPECT_EQ(v, 10);
  }
  {
    flat_wset<DbgClass> fws;
    DbgClass v(10);

    fws.insert(std::move(v));
    EXPECT_TRUE(fws.contains(10));
    EXPECT_NE(v, 10);
  }
  {
    flat_wset<DbgClass> fws;
    const DbgClass v(10);
    
    fws.insert(std::move(v));
    EXPECT_TRUE(fws.contains(10));
    EXPECT_EQ(v, 10);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Insert2)
{
  {
    std::vector<DbgClass> vec{1, 1};
    flat_wset<DbgClass> fws;
    fws.insert(vec.begin(), vec.end());
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0], 1);
  }
  {
    std::vector<DbgClass> vec{1, 1};
    flat_wset<DbgClass> fws;
    fws.insert(std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
    
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_NE(vec[0], 1);
    EXPECT_EQ(vec[1], 1);
  }
  {
    flat_wset<DbgClass> fws;
    std::initializer_list<DbgClass> ilist{1, 1};
    fws.insert(ilist);
    
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(ilist.size(), 2u);
    EXPECT_EQ(*ilist.begin(), 1);
  }
  {
    flat_wset<int> fws;
    fws.insert({1, 1});
    
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Emplace)
{
  {
    flat_wset<DbgClass> fws;
    auto it = fws.emplace(1);
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
    
    it = fws.emplace(1);
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_FALSE(it.second);
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
    
    it = fws.emplace(2);
    ASSERT_NE(it.first, fws.end());
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fws.contains(2));
    EXPECT_EQ(fws.size(), 2u);
  }
  {
    flat_wset<DbgClass> fws;
    DbgClass k(1);
    
    auto it = fws.emplace(k);
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(k, 1);
  }
  {
    flat_wset<DbgClass> fws;
    DbgClass k(1);
    
    auto it = fws.emplace(std::move(k));
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, 1);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fws.contains(1));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_NE(k, 1);
  }
  {
    flat_wset<std::string> fws;
    const char* k = "1";

    auto it = fws.emplace(k);
    ASSERT_NE(it.first, fws.end());
    EXPECT_EQ(*it.first, k);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fws.contains(k));
    EXPECT_EQ(fws.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Erase)
{
  {
    flat_wset<DbgClass> fws{1, 3};
    EXPECT_EQ(fws.erase(1), 1u);
    EXPECT_FALSE(fws.contains(1));
    EXPECT_TRUE(fws.contains(3));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    
    EXPECT_EQ(fws.erase(1), 0u);
    EXPECT_FALSE(fws.contains(1));
    EXPECT_TRUE(fws.contains(3));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 2u);
    
    EXPECT_EQ(fws.erase(3), 1u);
    EXPECT_FALSE(fws.contains(1));
    EXPECT_FALSE(fws.contains(3));
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 2u);
  }
  {
    flat_wset<DbgClass> fws;
    EXPECT_EQ(fws.erase(1), 0u);
    EXPECT_FALSE(fws.contains(1));
    EXPECT_EQ(fws.size(), 0u);
    EXPECT_EQ(fws.bucket_count(), 0u);
    
    fws.emplace(2);
    EXPECT_EQ(fws.erase(1), 0u);
    EXPECT_TRUE(fws.contains(2));
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_EQ(fws.bucket_count(), 2u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, EraseIter)
{
  {
    flat_wset<DbgClass> fws{1, 3};
    auto first = fws.begin();
    bool is1 = *first == 1;
    
    fws.erase(first);
    EXPECT_EQ(fws.contains(1), !is1);
    EXPECT_EQ(fws.contains(3), is1);
    EXPECT_EQ(fws.size(), 1u);
    
    first = fws.begin();
    fws.erase(first);
    EXPECT_FALSE(fws.contains(1));
    EXPECT_FALSE(fws.contains(3));
    EXPECT_EQ(fws.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws{1, 3};
    auto first = fws.begin();
    bool is1 = *first == 1;
    
    auto it = fws.erase_(first);
    ASSERT_NE(it, fws.end());
    EXPECT_EQ(*it, is1 ? 3 : 1);
    EXPECT_EQ(fws.contains(1), !is1);
    EXPECT_EQ(fws.contains(3), is1);
    EXPECT_EQ(fws.size(), 1u);
    
    first = fws.begin();
    it = fws.erase_(first);
    ASSERT_EQ(it, fws.end());
    EXPECT_FALSE(fws.contains(1));
    EXPECT_FALSE(fws.contains(3));
    EXPECT_EQ(fws.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws;
    for (int i = 1; i <= 100; ++i)
      fws.emplace(i);
    EXPECT_EQ(fws.size(), 100u);
    
    auto it = fws.begin();
    while (!fws.empty())
      it = fws.erase_(it);
    EXPECT_EQ(fws.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, EraseIf)
{
  {
    flat_wset<DbgClass> fws;
    
    erase_if(fws, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fws.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws{1, 3};
    
    erase_if(fws, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fws.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws{1, 2, 3};
    
    erase_if(fws, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fws.size(), 1u);
    EXPECT_FALSE(fws.contains(1));
    EXPECT_TRUE(fws.contains(2));
    EXPECT_FALSE(fws.contains(3));
  }
  {
    flat_wset<DbgClass> fws{2, 4};
    
    erase_if(fws, [](const auto& item){ return (item.id % 2); });
    EXPECT_EQ(fws.size(), 2u);
    EXPECT_TRUE(fws.contains(2));
    EXPECT_TRUE(fws.contains(4));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Swap)
{
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2;
    
    fws2.swap(fws1);
    EXPECT_EQ(fws1.size(), 0u);
    EXPECT_EQ(fws2.size(), 0u);
    
    fws2.swap(fws1);
    EXPECT_EQ(fws1.size(), 0u);
    EXPECT_EQ(fws2.size(), 0u);
  }
  {
    flat_wset<DbgClass> fws1{1, 3, 5};
    flat_wset<DbgClass> fws2;

    fws2.swap(fws1);
    EXPECT_EQ(fws1.size(), 0u);
    EXPECT_EQ(fws2.size(), 3u);
    EXPECT_FALSE(fws1.contains(3));
    EXPECT_TRUE(fws2.contains(3));

    fws2.swap(fws1);
    EXPECT_EQ(fws1.size(), 3u);
    EXPECT_EQ(fws2.size(), 0u);
    EXPECT_TRUE(fws1.contains(3));
    EXPECT_FALSE(fws2.contains(3));
  }
  {
    flat_wset<DbgClass> fws1{1, 3};
    flat_wset<DbgClass> fws2{5};
    
    fws2.swap(fws1);
    EXPECT_EQ(fws1.size(), 1u);
    EXPECT_EQ(fws2.size(), 2u);
    EXPECT_TRUE(fws1.contains(5));
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
    EXPECT_FALSE(fws2.contains(5));
    
    fws2.swap(fws1);
    EXPECT_EQ(fws1.size(), 2u);
    EXPECT_EQ(fws2.size(), 1u);
    EXPECT_TRUE(fws1.contains(3));
    EXPECT_FALSE(fws1.contains(5));
    EXPECT_TRUE(fws2.contains(5));
    EXPECT_FALSE(fws2.contains(3));
  }
  {
    using std::swap;
    
    flat_wset<DbgClass> fws1{1, 3};
    flat_wset<DbgClass> fws2{5};

    swap(fws1, fws2);
    EXPECT_EQ(fws1.size(), 1u);
    EXPECT_EQ(fws2.size(), 2u);
    EXPECT_TRUE(fws1.contains(5));
    EXPECT_FALSE(fws1.contains(1));
    EXPECT_TRUE(fws2.contains(1));
    EXPECT_FALSE(fws2.contains(5));

    swap(fws1, fws2);
    EXPECT_EQ(fws1.size(), 2u);
    EXPECT_EQ(fws2.size(), 1u);
    EXPECT_TRUE(fws1.contains(3));
    EXPECT_FALSE(fws1.contains(5));
    EXPECT_TRUE(fws2.contains(5));
    EXPECT_FALSE(fws2.contains(3));
  }
  {
    using std::swap;
    
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2{5, 4};
    
    swap(fws1, fws1);
    EXPECT_EQ(fws1.size(), 1u);
    EXPECT_TRUE(fws1.contains(1));
    
    swap(fws2, fws1);
    fws1.swap(fws2);
    EXPECT_EQ(fws1.size(), 1u);
    EXPECT_EQ(fws2.size(), 2u);
    EXPECT_TRUE(fws1.contains(1));
    EXPECT_FALSE(fws1.contains(5));
    EXPECT_TRUE(fws2.contains(5));
    EXPECT_FALSE(fws2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, Equality)
{
  {
    flat_wset<DbgClass> fws1;
    flat_wset<DbgClass> fws2;
    EXPECT_EQ(fws1, fws2);
    EXPECT_EQ(fws2, fws1);
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2{1};
    EXPECT_EQ(fws1, fws2);
    EXPECT_EQ(fws2, fws1);
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2{5};
    EXPECT_NE(fws1, fws2);
    EXPECT_NE(fws2, fws1);
  }
  {
    flat_wset<DbgClass> fws1{};
    flat_wset<DbgClass> fws2{1};
    EXPECT_NE(fws1, fws2);
    EXPECT_NE(fws2, fws1);
  }
  {
    flat_wset<DbgClass> fws1{1};
    flat_wset<DbgClass> fws2{1, 3};
    EXPECT_NE(fws1, fws2);
    EXPECT_NE(fws2, fws1);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWSetTest, BadHash)
{
  struct bad_hash {
    size_t operator()(const int& x) const { return x; }
  };
  
  flat_wset<int, bad_hash> fws;
  std::unordered_set<int, bad_hash> set;
  for (int i = 0; i < 100000; ++i)
  {
    fws.emplace(i);
    set.emplace(i);
  }
  EXPECT_EQ(fws.size(), set.size());
  
  for (const auto& item : set)
  {
    auto itF = fws.find(item);
    ASSERT_NE(itF, fws.end());
  }
}

TEST(FlatWSetTest, Stress)
{
  {
    auto seed = time(NULL);
    auto get_rand = []() {
      int r = rand() + 1;
      return (RAND_MAX > 32768) ? r : r * rand() + 1;
    };
    
    std::cout << "Stress seed: " << seed << "\n";
    srand((unsigned int)seed);

    flat_wset<DbgClass> fws;
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
          fws.insert(k);
          set.insert(k);
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fws.size(), set.size()) << "insert";
          break;
        }
        case 1: // emplace
        case 2:
        {
          int k = get_rand();
          fws.emplace(k);
          set.emplace(k);
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fws.size(), set.size()) << "emplace";
          break;
        }
        case 3: // count
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            EXPECT_EQ(fws.count(k), set.count(k));
            ++findCount;
          }
          EXPECT_EQ(fws.size(), set.size()) << "count";
          break;
        }
        case 4: // erase
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto countF = fws.erase(k);
            auto countS = set.erase(k);
            ++removeCount;
            EXPECT_EQ(countF, countS);
          }
          EXPECT_EQ(fws.size(), set.size()) << "erase";
          break;
        }
        case 5: // erase first
        {
          if (!set.empty())
          {
            auto itS = set.begin();
            auto itF = fws.find(*itS);
            ASSERT_NE(itF, fws.end());
            ASSERT_NE(itS, set.end());
            fws.erase_(itF);
            set.erase(itS);
            ++removeCount;
          }
          EXPECT_EQ(fws.size(), set.size()) << "erase first";
          break;
        }
        case 6: // find
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto itF = fws.find(k);
            auto itS = set.find(k);
            if (itF != fws.end())
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
            fws.rehash(j);
            ++rehashCount;
          }
          EXPECT_EQ(fws.size(), set.size()) << "rehash";
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
    EXPECT_EQ(fws.size(), set.size()) << "final";

    for (const auto& item : set)
    {
      auto itF = fws.find(item);
      ASSERT_NE(itF, fws.end());
    }
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}
