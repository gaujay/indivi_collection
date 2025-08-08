/**
 * Copyright 2025 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#include "gtest/gtest.h"

#define INDIVI_FLAT_W_DEBUG
#define INDIVI_FLAT_W_STATS
#include "indivi/flat_wmap.h"
#include "utils/debug_utils.h"

#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cassert>
#include <cstdlib>

using namespace indivi;

TEST(FlatWMapTest, Constructor)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(1));
  }
  {
    enum EN { AA, BB };
    
    flat_wmap<EN, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(AA));
  }
  {
    flat_wmap<std::shared_ptr<int>, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(nullptr));
  }
  {
    flat_wmap<std::unique_ptr<int>, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(nullptr));
  }
  {
    flat_wmap<int*, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(nullptr));
  }
  {
    flat_wmap<int, double> fwm;
    EXPECT_FALSE(fwm.contains(0));
  }
  {
    flat_wmap<std::string, std::string> fwm;
    EXPECT_FALSE(fwm.contains(""));
  }
  {
    flat_wmap<const char*, char*> fwm;
    EXPECT_FALSE(fwm.contains(""));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Constructor2)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm(6);
    EXPECT_EQ(fwm.bucket_count(), 8u);
    EXPECT_FALSE(fwm.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass, std::hash<DbgClass>> fwm(0, std::hash<DbgClass>());
    EXPECT_FALSE(fwm.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass, std::hash<DbgClass>, std::equal_to<DbgClass>> fwm(0, std::hash<DbgClass>(), std::equal_to<DbgClass>());
    EXPECT_FALSE(fwm.contains(1));
  }
  {
    std::vector<std::pair<DbgClass, DbgClass>> vec{{1, 2}, {3, 4}};
    flat_wmap<DbgClass, DbgClass> fwm(vec.begin(), vec.end());
    EXPECT_EQ(fwm.size(), 2u);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_TRUE(fwm.contains(3));
    EXPECT_FALSE(fwm.contains(2));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}};
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(2));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Constructor3)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2 = fwm1;
    EXPECT_EQ(fwm1.size(), 0u);
    EXPECT_EQ(fwm2.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2 = fwm1;
    EXPECT_TRUE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2 = std::move(fwm1);
    EXPECT_EQ(fwm1.size(), 0u);
    EXPECT_EQ(fwm2.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2 = std::move(fwm1);
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
    EXPECT_EQ(fwm1.size(), 0u);
    EXPECT_EQ(fwm2.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Assignment)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2;
    fwm2 = fwm1;
    EXPECT_TRUE(fwm1.empty());
    EXPECT_TRUE(fwm2.empty());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2;
    fwm2 = fwm1;
    EXPECT_TRUE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 2}};
    fwm2 = fwm1;
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_FALSE(fwm2.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2{{2, 4}};
    EXPECT_FALSE(fwm2.contains(1));
    EXPECT_TRUE(fwm2.contains(2));
    fwm2 = fwm1;
    EXPECT_TRUE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
    EXPECT_FALSE(fwm2.contains(2));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_TRUE(fwm.empty());
    fwm = {{1, 2}, {3, 2}};
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_TRUE(fwm.contains(3));
    fwm = {};
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(3));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 4}};
    fwm = {{1, 2}, {3, 5}};
    EXPECT_EQ(fwm.at(1), 2);
    EXPECT_EQ(fwm.at(3), 5);
    fwm = {};
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(3));
    EXPECT_TRUE(fwm.empty());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Assignment2)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2;
    fwm2 = std::move(fwm1);
    EXPECT_TRUE(fwm1.empty());
    EXPECT_TRUE(fwm2.empty());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2;
    fwm2 = std::move(fwm1);
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 2}};
    fwm2 = std::move(fwm1);
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_FALSE(fwm2.contains(1));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 3}};
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 2}, {4, 5}};
    fwm2 = std::move(fwm1);
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_EQ(fwm2.at(1), 3);
    EXPECT_FALSE(fwm2.contains(4));
  }
  {
    flat_wmap<int, int> fwm1{{1, 3}, {6, 7}};
    flat_wmap<int, int> fwm2{{1, 2}, {4, 5}};
    fwm2 = fwm1;
    EXPECT_EQ(fwm1.at(1), 3);
    EXPECT_EQ(fwm2.at(1), 3);
    EXPECT_FALSE(fwm2.contains(4));
    EXPECT_TRUE(fwm2.contains(6));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Capacity)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_TRUE(fwm.empty());
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 0u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
    
    auto max_size = (flat_wmap<DbgClass, DbgClass>::size_type)(fwm.max_bucket_count() * fwm.max_load_factor());
    EXPECT_EQ(fwm.max_load_factor(), 0.8f);
    EXPECT_EQ(fwm.max_size(), max_size);
    EXPECT_GT(fwm.max_bucket_count(), 0u);
    fwm.max_load_factor(0.f); // no-op
    EXPECT_GT(fwm.max_bucket_count(), 0u);
    
    fwm = {{1, 2}, {3, 4}};
    EXPECT_EQ(fwm.size(), 2u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    EXPECT_EQ(fwm.load_factor(), 1.f);
  }
  EXPECT_EQ(DbgClass::count, 0);
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 1}};
    EXPECT_FALSE(fwm.empty());
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    EXPECT_EQ(fwm.load_factor(), 0.5f);
    
    fwm.insert_or_assign(2, 1);
    EXPECT_EQ(fwm.size(), 2u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    EXPECT_EQ(fwm.load_factor(), 1.f);
    
    fwm.insert_or_assign(3, 1);
    EXPECT_EQ(fwm.size(), 3u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    EXPECT_EQ(fwm.load_factor(), 0.75f);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Capacity2)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(0);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 0u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(1);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(7);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 8u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(8);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(15);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(16);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 32u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(31);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 64u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Capacity3)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.rehash(8);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.rehash(15);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.rehash(16);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 32u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.rehash(31);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 32u);
    EXPECT_EQ(fwm.load_factor(), 0.f);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm(3);
    fwm[1] = 2;
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    
    fwm.rehash(0);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    
    fwm.clear();
    fwm.rehash(0);
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 0u);
    EXPECT_TRUE(fwm.is_cleared());
    
    fwm[3] = 4;
    EXPECT_EQ(fwm.at(3), 4);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    fwm.rehash(4);
    EXPECT_EQ(fwm.at(3), 4);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    
    fwm.reserve(4);
    EXPECT_EQ(fwm.at(3), 4);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    
    fwm.reserve(5);
    EXPECT_EQ(fwm.at(3), 4);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 8u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Observers)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    auto hash = fwm.hash_function();
    auto keq = fwm.key_eq();
    
    EXPECT_NE(hash(1), 1);
    EXPECT_TRUE(keq(1, 1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Iterator)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_EQ(fwm.begin(), fwm.end());
    EXPECT_EQ(fwm.cbegin(), fwm.cend());
    EXPECT_EQ(fwm.cbegin(), fwm.end());
    EXPECT_EQ(fwm.begin(), fwm.cend());
    EXPECT_EQ(fwm.begin(), fwm.cbegin());
    EXPECT_EQ(fwm.end(), fwm.cend());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}};
    EXPECT_NE(fwm.begin(), fwm.end());
    EXPECT_NE(fwm.cbegin(), fwm.cend());
    EXPECT_EQ(++fwm.begin(), fwm.end());
    EXPECT_EQ(++fwm.cbegin(), fwm.cend());
    
    auto it = fwm.begin();
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, 2);
    
    fwm[3] = 4;
    it = fwm.begin();
    EXPECT_TRUE(it->first == 3 || it->first == 1);
    EXPECT_EQ(it->second, (it->first == 3) ? 4 : 2);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}};
    auto it1 = fwm.find(1);
    EXPECT_NE(it1, fwm.end());
    auto it2 = fwm.find(3);
    EXPECT_NE(it2, fwm.end());
    EXPECT_NE(it2, it1);
    auto it3 = fwm.find(5);
    EXPECT_EQ(it3, fwm.end());
    EXPECT_NE(it3, it1);
    EXPECT_NE(it3, it2);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    for (int i = 1; i <= 80; ++i) {
      fwm[i] = 100 + i;
      EXPECT_EQ(fwm[i].id, 100 + i) << i;
      EXPECT_EQ(fwm[1].id, 101) << i;
    }
    
    int count = 0;
    for (const auto& val : fwm)
    {
      EXPECT_TRUE(fwm.contains(val.first)) << count;
      ++count;
    }
    EXPECT_EQ(count, (int)fwm.size());
  }
  {
    flat_wmap<int, int> fwm{{1,1}, {2,2}, {3,3}, {4,4}, {5,5}};
    for (auto& val : fwm)
      val.second += 5;
    for (int i = 1; i <= 5; ++i)
      EXPECT_EQ(fwm.at(i), i + 5) << i;
  }
  {
    flat_wmap<int, int> fwm{{1,1}, {2,2}, {3,3}};
    for (auto it = fwm.begin(); it != fwm.end(); ++it)
      it->second *= 2;
    for (int i = 1; i <= 3; ++i)
      EXPECT_EQ(fwm.at(i), i * 2) << i;
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Iterator2)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(20);
    EXPECT_EQ(fwm.bucket_count(), 32u);
    
    for (int i = 1; i <= 10000; ++i)
    {
      if (fwm.empty()) {
        fwm[i] = 2 * i;
        auto it = fwm.cbegin();
        ASSERT_NE(it, fwm.cend());
        EXPECT_EQ(it->first, i);
        EXPECT_EQ(it->second, 2 * i);
        EXPECT_EQ(++it, fwm.cend());
      }
      else {
        auto it = fwm.erase_(fwm.cbegin());
        EXPECT_EQ(it, fwm.cend());
      }
    }
    EXPECT_EQ(fwm.bucket_count(), 32u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(7);
    EXPECT_EQ(fwm.bucket_count(), 8u);
    
    for (int i = 1; i <= 10000; ++i)
    {
      if (fwm.empty()) {
        fwm[i] = 2 * i;
        auto it = fwm.cbegin();
        ASSERT_NE(it, fwm.cend());
        EXPECT_EQ(it->first, i);
        EXPECT_EQ(it->second, 2 * i);
        EXPECT_EQ(++it, fwm.cend());
      }
      else {
        auto it = fwm.erase_(fwm.cbegin());
        EXPECT_EQ(it, fwm.cend());
      }
    }
    EXPECT_EQ(fwm.bucket_count(), 8u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.reserve(15);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    
    for (int i = 1; i <= 10000; ++i)
    {
      if (fwm.empty()) {
        fwm[i] = 2 * i;
        auto it = fwm.cbegin();
        ASSERT_NE(it, fwm.cend());
        EXPECT_EQ(it->first, i);
        EXPECT_EQ(it->second, 2 * i);
        EXPECT_EQ(++it, fwm.cend());
      }
      else {
        auto it = fwm.erase_(fwm.cbegin());
        EXPECT_EQ(it, fwm.cend());
      }
    }
    EXPECT_EQ(fwm.bucket_count(), 16u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, ElementAccess)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(127));
    EXPECT_EQ(fwm.count(1), 0u);
    EXPECT_EQ(fwm.count(128), 0u);
    
    auto it = fwm.find(1);
    ASSERT_EQ(it, fwm.end());
    it = fwm.find(3);
    EXPECT_EQ(it, fwm.end());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}, {5, 6}};
    EXPECT_TRUE(fwm.contains(5));
    EXPECT_FALSE(fwm.contains(6));
    EXPECT_EQ(fwm.count(1), 1u);
    EXPECT_EQ(fwm.count(2), 0u);
    
    auto it = fwm.find(3);
    ASSERT_NE(it, fwm.end());
    EXPECT_EQ(it->first, 3);
    EXPECT_EQ(it->second, 4);
    
    it = fwm.find(2);
    EXPECT_EQ(it, fwm.end());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}, {5, 6}};
    EXPECT_NO_THROW(fwm.at(3));
    auto val = fwm.at(3);
    EXPECT_EQ(val, 4);
    EXPECT_THROW(fwm.at(7), std::out_of_range);
  }
  {
    flat_wmap<int, std::string> fwm{{1, "2"}, {3, "4"}};
    EXPECT_EQ(fwm[1], "2");
    EXPECT_EQ(fwm[5], "");
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, ElementAccess2)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    
    fwm[k] = v;
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
    EXPECT_EQ(fwm.at(k), v);
    EXPECT_EQ(fwm.size(), 1u);
    
    DbgClass v2(3);
    fwm[std::move(k)] = std::move(v2);
    EXPECT_EQ(k, 1);
    EXPECT_NE(v2, 3);
    EXPECT_EQ(fwm.at(k), 3);
    EXPECT_EQ(fwm.size(), 1u);
  }
  {
    flat_wmap<std::string, std::string> fwm;
    auto& v1 = fwm[""];
    EXPECT_EQ(v1, "");
    
    auto& v2 = fwm["a"];
    EXPECT_EQ(v2, "");
    
    v1 = "b";
    EXPECT_EQ(v1, "b");
    v2 = "";
    EXPECT_EQ(v2, "");
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Clear)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 0u);
    
    fwm.clear();
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 0u);
    EXPECT_TRUE(fwm.is_cleared());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}, {5, 6}};
    EXPECT_EQ(fwm.size(), 3u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    
    fwm.clear();
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    EXPECT_TRUE(fwm.is_cleared());
    fwm.clear();
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 4u);
    EXPECT_TRUE(fwm.is_cleared());
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    for (int i = 1; i <= 40; ++i)
      fwm[i] = i;
    EXPECT_EQ(fwm.size(), 40u);
    EXPECT_EQ(fwm.bucket_count(), 64u);

    fwm.clear();
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 64u);
    EXPECT_TRUE(fwm.is_cleared());

    for (int i = 1; i <= 40; ++i)
      fwm[i] = i;
    fwm.clear();
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 64u);
    EXPECT_TRUE(fwm.is_cleared());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Insert)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_FALSE(fwm.contains(1));
    auto it = fwm.insert({1, 2});
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fwm.at(1), 2);
    
    fwm.insert({3, 4});
    EXPECT_EQ(fwm.at(3), 4);
    
    it = fwm.insert({3, 5});
    EXPECT_EQ(fwm.at(3), 4);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 3);
    EXPECT_EQ(it.first->second, 4);
    EXPECT_FALSE(it.second);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::pair<DbgClass, DbgClass> v(10, 20);
    
    fwm.insert(v);
    EXPECT_EQ(fwm.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::pair<const DbgClass, const DbgClass> v(10, 20);
    
    fwm.insert(v);
    EXPECT_EQ(fwm.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::pair<DbgClass, DbgClass> v(10, 20);

    fwm.insert(std::move(v));
    EXPECT_EQ(fwm.at(10), 20);
    EXPECT_NE(v.first, 10);
    EXPECT_NE(v.second, 20);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::pair<const DbgClass, const DbgClass> v(10, 20);
    
    fwm.insert(std::move(v));
    EXPECT_EQ(fwm.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::pair<const DbgClass, DbgClass> v(10, 20);
    
    fwm.insert(std::move(v));
    EXPECT_EQ(fwm.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_NE(v.second, 20);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::pair<DbgClass, const DbgClass> v(10, 20);
    
    fwm.insert(std::move(v));
    EXPECT_EQ(fwm.at(10), 20);
    EXPECT_NE(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Insert2)
{
  {
    std::vector<std::pair<DbgClass, DbgClass>> vec{{1, 2}, {1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.insert(vec.begin(), vec.end());
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0].first, 1);
    EXPECT_EQ(vec[0].second, 2);
  }
  {
    std::vector<std::pair<DbgClass, DbgClass>> vec{{1, 2}, {1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm;
    fwm.insert(std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
    
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_NE(vec[0].first, 1);
    EXPECT_NE(vec[0].second, 2);
    EXPECT_EQ(vec[1].first, 1);
    EXPECT_EQ(vec[1].second, 2);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::initializer_list<std::pair<const DbgClass, DbgClass>> ilist{{1, 2}, {1, 2}};
    fwm.insert(ilist);
    
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(ilist.size(), 2u);
    EXPECT_EQ(ilist.begin()->first, 1);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    std::initializer_list<std::pair<DbgClass, DbgClass>> ilist{{1, 2}, {1, 2}};
    fwm.insert(ilist);
    
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(ilist.size(), 2u);
    EXPECT_EQ(ilist.begin()->first, 1);
  }
  {
    flat_wmap<int, int> fwm;
    fwm.insert({{1, 2}, {1, 2}});
    
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, InsertOrAssign)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    auto it = fwm.insert_or_assign(1, 2);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);

    it = fwm.insert_or_assign(1, 3);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 3);
    EXPECT_FALSE(it.second);
    EXPECT_EQ(fwm[1], 3);
    EXPECT_EQ(fwm.size(), 1u);

    it = fwm.insert_or_assign(2, 4);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fwm[2], 4);
    EXPECT_EQ(fwm.size(), 2u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    const DbgClass k(1);
    const DbgClass v(2);
    fwm.insert_or_assign(k, v);
    
    EXPECT_TRUE(fwm.contains(k));
    EXPECT_EQ(fwm[k], v);
    EXPECT_EQ(fwm.size(), 1u);
    
    const DbgClass v2(4);
    fwm.insert_or_assign(k, v2);
    EXPECT_EQ(fwm[k], v2);
    EXPECT_EQ(fwm.size(), 1u);
    
    const DbgClass k3(4);
    const DbgClass v3(4);
    fwm.insert_or_assign(k3, v3);
    EXPECT_EQ(fwm[k3], v3);
    EXPECT_EQ(fwm.size(), 2u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    fwm.insert_or_assign(k, v);
    
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    fwm.insert_or_assign(std::move(k), std::move(v));
    
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_NE(k, 1);
    EXPECT_NE(v, 2);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Emplace)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    auto it = fwm.emplace(1, 2);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    
    it = fwm.emplace(1, 3);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_FALSE(it.second);
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    
    it = fwm.emplace(2, 4);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fwm[2], 4);
    EXPECT_EQ(fwm.size(), 2u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fwm.emplace(k, v);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fwm.emplace(std::move(k), std::move(v));
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_NE(k, 1);
    EXPECT_NE(v, 2);
  }
  {
    flat_wmap<std::string, std::string> fwm;
    const char* k = "1";
    const char* v = "2";

    auto it = fwm.emplace(k, v);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, k);
    EXPECT_EQ(it.first->second, v);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(k));
    EXPECT_EQ(fwm[k], v);
    EXPECT_EQ(fwm.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, TryEmplace)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    auto it = fwm.try_emplace(1, 2);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    
    it = fwm.try_emplace(1, 3);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_FALSE(it.second);
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    
    it = fwm.try_emplace(2, 4);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fwm[2], 4);
    EXPECT_EQ(fwm.size(), 2u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fwm.try_emplace(k, v);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fwm.try_emplace(std::move(k), std::move(v));
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_NE(k, 1);
    EXPECT_NE(v, 2);
    
    DbgClass k2(1);
    DbgClass v2(2);
    it = fwm.try_emplace(std::move(k2), std::move(v2));
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_FALSE(it.second);
    EXPECT_TRUE(fwm.contains(1));
    EXPECT_EQ(fwm[1], 2);
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(k2, 1);
    EXPECT_EQ(v2, 2);
  }
  {
    flat_wmap<std::string, std::string> fwm;
    const char* k = "1";
    const char* v = "2";
    
    auto it = fwm.try_emplace(k, v);
    ASSERT_NE(it.first, fwm.end());
    EXPECT_EQ(it.first->first, k);
    EXPECT_EQ(it.first->second, v);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fwm.contains(k));
    EXPECT_EQ(fwm[k], v);
    EXPECT_EQ(fwm.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Erase)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}};
    EXPECT_EQ(fwm.erase(1), 1u);
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_TRUE(fwm.contains(3));
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    
    EXPECT_EQ(fwm.erase(1), 0u);
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_TRUE(fwm.contains(3));
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    
    EXPECT_EQ(fwm.erase(3), 1u);
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(3));
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    EXPECT_EQ(fwm.erase(1), 0u);
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 0u);
    
    fwm[2] = 2;
    EXPECT_EQ(fwm.erase(1), 0u);
    EXPECT_TRUE(fwm.contains(2));
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
    
    EXPECT_EQ(fwm.erase(2), 1u);
    EXPECT_EQ(fwm.erase(2), 0u);
    EXPECT_FALSE(fwm.contains(2));
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 2u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    for (int i = 1; i <= 15; ++i)
      fwm[i] = i;
    
    for (int i = 1; i <= 15; ++i)
      EXPECT_EQ(fwm.erase(i), 1u);
    
    EXPECT_EQ(fwm.size(), 0u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    
    for (int i = 1; i <= 15; ++i)
      fwm[i] = i;
    
    EXPECT_EQ(fwm.size(), 15u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    for (int i = 1; i <= 15; ++i)
      fwm[i] = i;
    
    for (int i = 2; i <= 15; ++i)
      EXPECT_EQ(fwm.erase(i), 1u);
    
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_EQ(fwm.bucket_count(), 16u);
    
    for (int i = 1; i <= 15; ++i)
      fwm[i] = i;
    
    EXPECT_EQ(fwm.size(), 15u);
    EXPECT_LE(fwm.bucket_count(), 32u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, EraseInSmall)
{
  {
    size_t size = 7u;
    size_t capa = 8u;
    flat_wmap<DbgClass, DbgClass> fwm;
    std::unordered_map<int, int> map;
    for (int i = 1; i <= (int)size; ++i) {
      fwm.emplace(i, i);
      map.emplace(i, i);
    }
    EXPECT_EQ(fwm.size(), size);
    EXPECT_EQ(fwm.bucket_count(), capa);
    
    for (int i = 0; i < 10000; ++i)
    {
      int key = i + 1;
      if (fwm.size() < size) {
        fwm.emplace(key, i);
        map.emplace(key, i);
      }
      else {
        auto it = map.begin();
        ASSERT_EQ(fwm.erase(it->first), 1u);
        map.erase(it);
      }
      ASSERT_EQ(fwm.bucket_count(), capa);
    }
  }
  {
    size_t size = 15u;
    size_t capa = 16u;
    flat_wmap<DbgClass, DbgClass> fwm;
    std::unordered_map<int, int> map;
    for (int i = 1; i <= (int)size; ++i) {
      fwm.emplace(i, i);
      map.emplace(i, i);
    }
    EXPECT_EQ(fwm.size(), size);
    EXPECT_EQ(fwm.bucket_count(), capa);

    for (int i = 0; i < 10000; ++i)
    {
      int key = i + 1;
      if (fwm.size() < size) {
        fwm.emplace(key, i);
        map.emplace(key, i);
      }
      else {
        auto it = map.begin();
        ASSERT_EQ(fwm.erase(it->first), 1u);
        map.erase(it);
      }
      ASSERT_EQ(fwm.bucket_count(), capa);
    }
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, EraseIter)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}};
    auto first = fwm.begin();
    bool is1 = first->first == 1;
    
    fwm.erase(first);
    EXPECT_EQ(fwm.contains(1), !is1);
    EXPECT_EQ(fwm.contains(3), is1);
    EXPECT_EQ(fwm.size(), 1u);
    
    first = fwm.begin();
    fwm.erase(first);
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(3));
    EXPECT_EQ(fwm.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 2}, {3, 4}};
    auto first = fwm.begin();
    bool is1 = first->first == 1;
    
    auto it = fwm.erase_(first);
    ASSERT_NE(it, fwm.end());
    EXPECT_EQ(it->first, is1 ? 3 : 1);
    EXPECT_EQ(it->second, is1 ? 4 : 2);
    EXPECT_EQ(fwm.contains(1), !is1);
    EXPECT_EQ(fwm.contains(3), is1);
    EXPECT_EQ(fwm.size(), 1u);
    
    first = fwm.begin();
    it = fwm.erase_(first);
    ASSERT_EQ(it, fwm.end());
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_FALSE(fwm.contains(3));
    EXPECT_EQ(fwm.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    for (int i = 1; i <= 100; ++i)
      fwm.emplace(i, 100 + i);
    EXPECT_EQ(fwm.size(), 100u);
    
    auto it = fwm.begin();
    while (!fwm.empty())
      it = fwm.erase_(it);
    EXPECT_EQ(fwm.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, EraseIf)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm;
    
    erase_if(fwm, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fwm.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 1}, {3, 3}};
    
    erase_if(fwm, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fwm.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{1, 1}, {2, 2}, {3, 3}};
    
    erase_if(fwm, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fwm.size(), 1u);
    EXPECT_FALSE(fwm.contains(1));
    EXPECT_TRUE(fwm.contains(2));
    EXPECT_FALSE(fwm.contains(3));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm{{2, 1}, {4, 3}};
    
    erase_if(fwm, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fwm.size(), 2u);
    EXPECT_TRUE(fwm.contains(2));
    EXPECT_TRUE(fwm.contains(4));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Swap)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2;
    
    fwm2.swap(fwm1);
    EXPECT_EQ(fwm1.size(), 0u);
    EXPECT_EQ(fwm2.size(), 0u);
    
    fwm2.swap(fwm1);
    EXPECT_EQ(fwm1.size(), 0u);
    EXPECT_EQ(fwm2.size(), 0u);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}, {3, 4}, {5, 6}};
    flat_wmap<DbgClass, DbgClass> fwm2;
    
    fwm2.swap(fwm1);
    EXPECT_EQ(fwm1.size(), 0u);
    EXPECT_EQ(fwm2.size(), 3u);
    EXPECT_FALSE(fwm1.contains(3));
    EXPECT_TRUE(fwm2.contains(3));
    
    fwm2.swap(fwm1);
    EXPECT_EQ(fwm1.size(), 3u);
    EXPECT_EQ(fwm2.size(), 0u);
    EXPECT_TRUE(fwm1.contains(3));
    EXPECT_FALSE(fwm2.contains(3));
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}, {3, 4}};
    flat_wmap<DbgClass, DbgClass> fwm2{{5, 6}};
    
    fwm2.swap(fwm1);
    EXPECT_EQ(fwm1.size(), 1u);
    EXPECT_EQ(fwm2.size(), 2u);
    EXPECT_TRUE(fwm1.contains(5));
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
    EXPECT_FALSE(fwm2.contains(5));
    
    fwm2.swap(fwm1);
    EXPECT_EQ(fwm1.size(), 2u);
    EXPECT_EQ(fwm2.size(), 1u);
    EXPECT_TRUE(fwm1.contains(3));
    EXPECT_FALSE(fwm1.contains(5));
    EXPECT_TRUE(fwm2.contains(5));
    EXPECT_FALSE(fwm2.contains(3));
  }
  {
    using std::swap;
    
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}, {3, 4}};
    flat_wmap<DbgClass, DbgClass> fwm2{{5, 6}};

    swap(fwm1, fwm2);
    EXPECT_EQ(fwm1.size(), 1u);
    EXPECT_EQ(fwm2.size(), 2u);
    EXPECT_TRUE(fwm1.contains(5));
    EXPECT_FALSE(fwm1.contains(1));
    EXPECT_TRUE(fwm2.contains(1));
    EXPECT_FALSE(fwm2.contains(5));

    swap(fwm1, fwm2);
    EXPECT_EQ(fwm1.size(), 2u);
    EXPECT_EQ(fwm2.size(), 1u);
    EXPECT_TRUE(fwm1.contains(3));
    EXPECT_FALSE(fwm1.contains(5));
    EXPECT_TRUE(fwm2.contains(5));
    EXPECT_FALSE(fwm2.contains(3));
  }
  {
    using std::swap;
    
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2{{5, 6}, {4, 3}};
    
    swap(fwm1, fwm1);
    EXPECT_EQ(fwm1.size(), 1u);
    EXPECT_TRUE(fwm1.contains(1));
    
    swap(fwm2, fwm1);
    fwm1.swap(fwm2);
    EXPECT_EQ(fwm1.size(), 1u);
    EXPECT_EQ(fwm2.size(), 2u);
    EXPECT_TRUE(fwm1.contains(1));
    EXPECT_FALSE(fwm1.contains(5));
    EXPECT_TRUE(fwm2.contains(5));
    EXPECT_FALSE(fwm2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, Equality)
{
  {
    flat_wmap<DbgClass, DbgClass> fwm1;
    flat_wmap<DbgClass, DbgClass> fwm2;
    EXPECT_EQ(fwm1, fwm2);
    EXPECT_EQ(fwm2, fwm1);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 2}};
    EXPECT_EQ(fwm1, fwm2);
    EXPECT_EQ(fwm2, fwm1);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2{{5, 6}};
    EXPECT_NE(fwm1, fwm2);
    EXPECT_NE(fwm2, fwm1);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 3}};
    EXPECT_NE(fwm1, fwm2);
    EXPECT_NE(fwm2, fwm1);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}};
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 2}, {3, 4}};
    EXPECT_NE(fwm1, fwm2);
    EXPECT_NE(fwm2, fwm1);
  }
  {
    flat_wmap<DbgClass, DbgClass> fwm1{{1, 2}, {3, 5}};
    flat_wmap<DbgClass, DbgClass> fwm2{{1, 2}, {3, 4}};
    EXPECT_NE(fwm1, fwm2);
    EXPECT_NE(fwm2, fwm1);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatWMapTest, BadHash)
{
  struct bad_hash {
    size_t operator()(const int& x) const { return x; }
  };
  
  flat_wmap<int, int, bad_hash> fwm;
  std::unordered_map<int, int, bad_hash> map;
  for (int i = 0; i < 100000; ++i)
  {
    fwm.emplace(i, i);
    map.emplace(i, i);
  }
  EXPECT_EQ(fwm.size(), map.size());
  
  for (const auto& item : map)
  {
    auto itF = fwm.find(item.first);
    ASSERT_NE(itF, fwm.end());
    EXPECT_EQ(itF->second, item.second);
  }
}

TEST(FlatWMapTest, Stress)
{
  {
    auto seed = time(NULL);
    auto get_rand = []() {
      int r = rand() + 1;
      return (RAND_MAX > 32768) ? r : r * rand() + 1;
    };
    
    std::cout << "Stress seed: " << seed << std::endl;
    srand((unsigned int)seed);
    
    flat_wmap<DbgClass, DbgClass> fwm;
    std::unordered_map<DbgClass, DbgClass> map;
    std::vector<DbgClass> vec;
    
    int addCount = 0;
    int updateCount = 0;
    int removeCount = 0;
    int rehashCount = 0;
    int iterCount = 0;

    for (int i = 0; i < 1000000; ++i)
    {
      int op = rand() % 9;
      switch(op)
      {
        case 0: // insert
        {
          int v = get_rand();
          int k = get_rand();
          fwm.insert({k, v});
          map.insert({k, v});
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fwm.size(), map.size()) << "insert";
          break;
        }
        case 1: // emplace
        {
          int v = get_rand();
          int k = get_rand();
          fwm.emplace(k, v);
          map.emplace(k, v);
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fwm.size(), map.size()) << "emplace";
          break;
        }
        case 2: // insert_or_assign
        {
          int v = get_rand();
          int k = get_rand();
          fwm.insert_or_assign(k, v);
          map[k] = v;
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fwm.size(), map.size()) << "insert_or_assign";
          break;
        }
        case 3: // update
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            int v = rand() + 1;
            fwm[k] = v;
            map[k] = v;
            ++updateCount;
          }
          EXPECT_EQ(fwm.size(), map.size()) << "update";
          break;
        }
        case 4: // erase
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto countF = fwm.erase(k);
            auto countM = map.erase(k);
            ++removeCount;
            EXPECT_EQ(countF, countM);
          }
          EXPECT_EQ(fwm.size(), map.size()) << "erase";
          break;
        }
        case 5: // erase first
        {
          if (!map.empty())
          {
            auto itM = map.begin();
            auto itF = fwm.find(itM->first);
            ASSERT_NE(itF, fwm.end());
            EXPECT_EQ(itF->second, itM->second);
            fwm.erase_(itF);
            map.erase(itM);
            ++removeCount;
          }
          EXPECT_EQ(fwm.size(), map.size()) << "erase first";
          break;
        }
        case 6: // find
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto itF = fwm.find(k);
            auto itM = map.find(k);
            if (itF != fwm.end())
            {
              EXPECT_NE(itM, map.end());
              EXPECT_EQ(itF->first, itM->first);
              EXPECT_EQ(itF->second, itM->second) << "find";
            }
            else
            {
              EXPECT_EQ(itM, map.end()) << "find none";
            }
          }
          break;
        }
        case 7: // rehash
        {
          if (rand() < RAND_MAX / 50)
          {
            auto j = vec.size() ? rand() % vec.size() : 0;
            fwm.rehash(j);
            ++rehashCount;
          }
          EXPECT_EQ(fwm.size(), map.size()) << "rehash";
          break;
        }
        case 8: // iter
        {
          if (!vec.empty() && rand() < RAND_MAX / 100)
          {
            auto k = vec[rand() % vec.size()];
            auto itF = fwm.find(k);
            if (itF != fwm.end())
            {
              auto it = fwm.begin();
              size_t count = 0u;
              while (it != itF) {
                ++it;
                ++count;
              }
              while (itF != fwm.end()) {
                ++itF;
                ++count;
              }
              ++iterCount;
              EXPECT_EQ(fwm.size(), count) << "iter";
            }
          }
          break;
        }
        default:
          assert(false && "unknown op");
      }
    }
    std::cout << "Stress final size: " << map.size()
              << ", add: " << addCount
              << ", update: " << updateCount
              << ", remove: " << removeCount
              << ", rehash: " << rehashCount
              << ", iter: " << iterCount << "\n";
    EXPECT_EQ(fwm.size(), map.size()) << "final";
    
    std::size_t idx = 0u;
    for (const auto& item : map)
    {
      auto itF = fwm.find(item.first);
      ASSERT_NE(itF, fwm.end()) << idx;
      EXPECT_EQ(itF->second, item.second) << idx;
      ++idx;
    }
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

#ifdef INDIVI_FLAT_W_STATS
TEST(FlatWMapTest, Stats)
{
  int count = 838860; // max load
  flat_wmap<int, int> fwm;
  std::unordered_map<int, int> map;
  for (int i = 1; i <= count; ++i) {
    fwm.emplace(i*83+i, i);
    map.emplace(i*83+i, i);
  }
  EXPECT_EQ(fwm.size(), (size_t)count);
  
  fwm.reset_find_stats();
  for (int i = 1; i <= count; ++i) {
    EXPECT_TRUE(fwm.contains(i*83+i));
    EXPECT_FALSE(fwm.contains(-i));
    EXPECT_TRUE(map.find(i*83+i) != map.end());
  }
  
  auto stats = fwm.get_find_stats();
  std::cout << "\n"
            << "load_factor: " << fwm.load_factor() << "\n"
            << "find_hit_count: " << stats.find_hit_count << "\n"
            << "find_miss_count: " << stats.find_miss_count << "\n"
            << "compare_hit_avg: " << stats.compare_hit_avg << "\n"
            << "compare_hit_max: " << stats.compare_hit_max << "\n"
            << "compare_miss_avg: " << stats.compare_miss_avg << "\n"
            << "compare_miss_max: " << stats.compare_miss_max << "\n"
            << "prob_len_hit_avg: " << stats.prob_len_hit_avg << "\n"
            << "prob_len_hit_max: " << stats.prob_len_hit_max << "\n"
            << "prob_len_miss_avg: " << stats.prob_len_miss_avg << "\n"
            << "prob_len_miss_max: " << stats.prob_len_miss_max << "\n"
            << std::endl;
  
  for (int i = 1; i <= count/2; ++i) {
    EXPECT_TRUE(fwm.erase(i*83+i));
    map.erase(i*83+i);
  }
  
  auto grpStats = fwm.get_group_stats();
  std::cout << "full_grp_avg: " << grpStats.full_grp_avg << "\n"
            << "tombstone_avg: " << grpStats.tombstone_avg << "\n"
            << std::endl;
  
  for (auto it : map)
    EXPECT_TRUE(fwm.contains(it.first));
  
  for (int i = count+1; i <= count*2; ++i) {
    EXPECT_EQ(fwm.contains(i*83+i), map.find(i*83+i) != map.end());
  }
}
#endif
