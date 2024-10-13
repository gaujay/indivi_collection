/**
 * Copyright 2024 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#include "gtest/gtest.h"

#define INDIVI_FLAT_U_DEBUG
#include "indivi/flat_umap.h"
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

TEST(FlatUMapTest, Constructor)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_FALSE(fum.contains(1));
  }
  {
    enum EN { AA, BB };
    
    flat_umap<EN, DbgClass> fum;
    EXPECT_FALSE(fum.contains(AA));
  }
  {
    flat_umap<std::shared_ptr<int>, DbgClass> fum;
    EXPECT_FALSE(fum.contains(nullptr));
  }
  {
    flat_umap<std::unique_ptr<int>, DbgClass> fum;
    EXPECT_FALSE(fum.contains(nullptr));
  }
  {
    flat_umap<int*, DbgClass> fum;
    EXPECT_FALSE(fum.contains(nullptr));
  }
  {
    flat_umap<int, double> fum;
    EXPECT_FALSE(fum.contains(0));
  }
  {
    flat_umap<std::string, std::string> fum;
    EXPECT_FALSE(fum.contains(""));
  }
  {
    flat_umap<const char*, char*> fum;
    EXPECT_FALSE(fum.contains(""));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Constructor2)
{
  {
    flat_umap<DbgClass, DbgClass> fum(10);
    EXPECT_GE(fum.bucket_count(), 10u);
    EXPECT_FALSE(fum.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass, std::hash<DbgClass>> fum(0, std::hash<DbgClass>());
    EXPECT_FALSE(fum.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass, std::hash<DbgClass>, std::equal_to<DbgClass>> fum(0, std::hash<DbgClass>(), std::equal_to<DbgClass>());
    EXPECT_FALSE(fum.contains(1));
  }
  {
    std::vector<std::pair<DbgClass, DbgClass>> vec{{1, 2}, {3, 4}};
    flat_umap<DbgClass, DbgClass> fum(vec.begin(), vec.end());
    EXPECT_EQ(fum.size(), 2u);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_TRUE(fum.contains(3));
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}};
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_FALSE(fum.contains(2));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Constructor3)
{
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2 = fum1;
    EXPECT_EQ(fum1.size(), 0u);
    EXPECT_EQ(fum2.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2 = fum1;
    EXPECT_TRUE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2 = std::move(fum1);
    EXPECT_EQ(fum1.size(), 0u);
    EXPECT_EQ(fum2.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2 = std::move(fum1);
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Assignment)
{
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2;
    fum2 = fum1;
    EXPECT_TRUE(fum1.empty());
    EXPECT_TRUE(fum2.empty());
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2;
    fum2 = fum1;
    EXPECT_TRUE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2{{1, 2}};
    fum2 = fum1;
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_FALSE(fum2.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2{{2, 4}};
    EXPECT_FALSE(fum2.contains(1));
    EXPECT_TRUE(fum2.contains(2));
    fum2 = fum1;
    EXPECT_TRUE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
    EXPECT_FALSE(fum2.contains(2));
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_TRUE(fum.empty());
    fum = {{1, 2}, {3, 2}};
    EXPECT_TRUE(fum.contains(1));
    EXPECT_TRUE(fum.contains(3));
    fum = {};
    EXPECT_FALSE(fum.contains(1));
    EXPECT_FALSE(fum.contains(3));
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 4}};
    fum = {{1, 2}, {3, 5}};
    EXPECT_EQ(fum.at(1), 2);
    EXPECT_EQ(fum.at(3), 5);
    fum = {};
    EXPECT_FALSE(fum.contains(1));
    EXPECT_FALSE(fum.contains(3));
    EXPECT_TRUE(fum.empty());
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Assignment2)
{
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2;
    fum2 = std::move(fum1);
    EXPECT_TRUE(fum1.empty());
    EXPECT_TRUE(fum2.empty());
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2;
    fum2 = std::move(fum1);
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2{{1, 2}};
    fum2 = std::move(fum1);
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_FALSE(fum2.contains(1));
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 3}};
    flat_umap<DbgClass, DbgClass> fum2{{1, 2}, {4, 5}};
    fum2 = std::move(fum1);
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_EQ(fum2.at(1), 3);
    EXPECT_FALSE(fum2.contains(4));
  }
  {
    flat_umap<int, int> fum1{{1, 3}, {6, 7}};
    flat_umap<int, int> fum2{{1, 2}, {4, 5}};
    fum2 = fum1;
    EXPECT_EQ(fum1.at(1), 3);
    EXPECT_EQ(fum2.at(1), 3);
    EXPECT_FALSE(fum2.contains(4));
    EXPECT_TRUE(fum2.contains(6));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Capacity)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_TRUE(fum.empty());
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 0u);
    EXPECT_EQ(fum.load_factor(), 0.f);
    
    auto max_size = (flat_umap<DbgClass, DbgClass>::size_type)(fum.max_bucket_count() * fum.max_load_factor());
    EXPECT_EQ(fum.max_load_factor(), 0.875f);
    EXPECT_EQ(fum.max_size(), max_size);
    EXPECT_GT(fum.max_bucket_count(), 0u);
    fum.max_load_factor(0.f); // no-op
    EXPECT_GT(fum.max_bucket_count(), 0u);
    
    fum = {{1, 2}, {3, 4}};
    EXPECT_EQ(fum.size(), 2u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    EXPECT_EQ(fum.load_factor(), 1.f);
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 1}};
    EXPECT_FALSE(fum.empty());
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    EXPECT_EQ(fum.load_factor(), 0.5f);
    
    fum.insert_or_assign(2, 1);
    EXPECT_EQ(fum.size(), 2u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    EXPECT_EQ(fum.load_factor(), 1.f);
    
    fum.insert_or_assign(3, 1);
    EXPECT_EQ(fum.size(), 3u);
    EXPECT_EQ(fum.bucket_count(), 4u);
    EXPECT_EQ(fum.load_factor(), 0.75f);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Capacity2)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    fum.reserve(0);
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 0u);
    EXPECT_EQ(fum.load_factor(), 0.f);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    fum.reserve(7);
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 8u);
    EXPECT_EQ(fum.load_factor(), 0.f);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    fum.reserve(31);
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 64u);
    EXPECT_EQ(fum.load_factor(), 0.f);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    fum.rehash(12);
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 16u);
    EXPECT_EQ(fum.load_factor(), 0.f);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    fum.rehash(31);
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 32u);
    EXPECT_EQ(fum.load_factor(), 0.f);
  }
  {
    flat_umap<DbgClass, DbgClass> fum(3);
    fum[1] = 2;
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 4u);
    
    fum.rehash(0);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    
    fum.clear();
    fum.rehash(0);
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 0u);
    
    fum[3] = 4;
    EXPECT_EQ(fum.at(3), 4);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    fum.rehash(4);
    EXPECT_EQ(fum.at(3), 4);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 4u);
    
    fum.reserve(4);
    EXPECT_EQ(fum.at(3), 4);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 4u);
    
    fum.reserve(5);
    EXPECT_EQ(fum.at(3), 4);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 8u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Observers)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    auto hash = fum.hash_function();
    auto keq = fum.key_eq();
    
    EXPECT_NE(hash(1), 1);
    EXPECT_TRUE(keq(1, 1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Iterator)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_EQ(fum.begin(), fum.end());
    EXPECT_EQ(fum.cbegin(), fum.cend());
    EXPECT_EQ(fum.cbegin(), fum.end());
    EXPECT_EQ(fum.begin(), fum.cend());
    EXPECT_EQ(fum.begin(), fum.cbegin());
    EXPECT_EQ(fum.end(), fum.cend());
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}};
    EXPECT_NE(fum.begin(), fum.end());
    EXPECT_NE(fum.cbegin(), fum.cend());
    EXPECT_EQ(++fum.begin(), fum.end());
    EXPECT_EQ(++fum.cbegin(), fum.cend());
    
    auto it = fum.begin();
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, 2);
    
    fum[3] = 4;
    it = fum.begin();
    EXPECT_TRUE(it->first == 3 || it->first == 1);
    EXPECT_EQ(it->second, (it->first == 3) ? 4 : 2);
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}};
    auto it1 = fum.find(1);
    EXPECT_NE(it1, fum.end());
    auto it2 = fum.find(3);
    EXPECT_NE(it2, fum.end());
    EXPECT_NE(it2, it1);
    auto it3 = fum.find(5);
    EXPECT_EQ(it3, fum.end());
    EXPECT_NE(it3, it1);
    EXPECT_NE(it3, it2);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    for (int i = 1; i <= 111; ++i)
      fum[i] = 111 + i;
    
    int count = 0;
    for (const auto& val : fum)
    {
      EXPECT_TRUE(fum.contains(val.first));
      ++count;
    }
    EXPECT_EQ(count, (int)fum.size());
  }
  {
    flat_umap<int, int> fum{{1,1}, {2,2}, {3,3}, {4,4}, {5,5}};
    for (auto& val : fum)
      val.second += 5;
    for (int i = 1; i <= 5; ++i)
      EXPECT_EQ(fum.at(i), i + 5) << i;
  }
  {
    flat_umap<int, int> fum{{1,1}, {2,2}, {3,3}};
    for (auto it = fum.begin(); it != fum.end(); ++it)
      it->second *= 2;
    for (int i = 1; i <= 3; ++i)
      EXPECT_EQ(fum.at(i), i * 2) << i;
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, ElementAccess)
{
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}, {5, 6}};
    EXPECT_TRUE(fum.contains(5));
    EXPECT_FALSE(fum.contains(6));
    EXPECT_EQ(fum.count(1), 1u);
    EXPECT_EQ(fum.count(2), 0u);
    
    auto it = fum.find(3);
    ASSERT_NE(it, fum.end());
    EXPECT_EQ(it->first, 3);
    EXPECT_EQ(it->second, 4);
    
    it = fum.find(2);
    EXPECT_EQ(it, fum.end());
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}, {5, 6}};
    EXPECT_NO_THROW(fum.at(3));
    auto val = fum.at(3);
    EXPECT_EQ(val, 4);
    EXPECT_THROW(fum.at(7), std::out_of_range);
  }
  {
    flat_umap<int, std::string> fum{{1, "2"}, {3, "4"}};
    EXPECT_EQ(fum[1], "2");
    EXPECT_EQ(fum[5], "");
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, ElementAccess2)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    
    fum[k] = v;
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
    EXPECT_EQ(fum.at(k), v);
    EXPECT_EQ(fum.size(), 1u);
    
    DbgClass v2(3);
    fum[std::move(k)] = std::move(v2);
    EXPECT_EQ(k, 1);
    EXPECT_NE(v2, 3);
    EXPECT_EQ(fum.at(k), 3);
    EXPECT_EQ(fum.size(), 1u);
  }
  {
    flat_umap<std::string, std::string> fum;
    auto& v1 = fum[""];
    EXPECT_EQ(v1, "");
    
    auto& v2 = fum["a"];
    EXPECT_EQ(v2, "");
    
    v1 = "b";
    EXPECT_EQ(v1, "b");
    v2 = "";
    EXPECT_EQ(v2, "");
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Clear)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 0u);
    
    fum.clear();
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}, {5, 6}};
    EXPECT_EQ(fum.size(), 3u);
    EXPECT_EQ(fum.bucket_count(), 4u);
    
    fum.clear();
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 4u);
    fum.clear();
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 4u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Insert)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_FALSE(fum.contains(1));
    auto it = fum.insert({1, 2});
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fum.at(1), 2);
    
    fum.insert({3, 4});
    EXPECT_EQ(fum.at(3), 4);
    
    it = fum.insert({3, 5});
    EXPECT_EQ(fum.at(3), 4);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 3);
    EXPECT_EQ(it.first->second, 4);
    EXPECT_FALSE(it.second);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::pair<DbgClass, DbgClass> v(10, 20);
    
    fum.insert(v);
    EXPECT_EQ(fum.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::pair<const DbgClass, const DbgClass> v(10, 20);
    
    fum.insert(v);
    EXPECT_EQ(fum.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::pair<DbgClass, DbgClass> v(10, 20);

    fum.insert(std::move(v));
    EXPECT_EQ(fum.at(10), 20);
    EXPECT_NE(v.first, 10);
    EXPECT_NE(v.second, 20);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::pair<const DbgClass, const DbgClass> v(10, 20);
    
    fum.insert(std::move(v));
    EXPECT_EQ(fum.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::pair<const DbgClass, DbgClass> v(10, 20);
    
    fum.insert(std::move(v));
    EXPECT_EQ(fum.at(10), 20);
    EXPECT_EQ(v.first, 10);
    EXPECT_NE(v.second, 20);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::pair<DbgClass, const DbgClass> v(10, 20);
    
    fum.insert(std::move(v));
    EXPECT_EQ(fum.at(10), 20);
    EXPECT_NE(v.first, 10);
    EXPECT_EQ(v.second, 20);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Insert2)
{
  {
    std::vector<std::pair<DbgClass, DbgClass>> vec{{1, 2}, {1, 2}};
    flat_umap<DbgClass, DbgClass> fum;
    fum.insert(vec.begin(), vec.end());
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0].first, 1);
    EXPECT_EQ(vec[0].second, 2);
  }
  {
    std::vector<std::pair<DbgClass, DbgClass>> vec{{1, 2}, {1, 2}};
    flat_umap<DbgClass, DbgClass> fum;
    fum.insert(std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
    
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(vec.size(), 2u);
    EXPECT_NE(vec[0].first, 1);
    EXPECT_NE(vec[0].second, 2);
    EXPECT_EQ(vec[1].first, 1);
    EXPECT_EQ(vec[1].second, 2);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::initializer_list<std::pair<const DbgClass, DbgClass>> ilist{{1, 2}, {1, 2}};
    fum.insert(ilist);
    
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(ilist.size(), 2u);
    EXPECT_EQ(ilist.begin()->first, 1);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    std::initializer_list<std::pair<DbgClass, DbgClass>> ilist{{1, 2}, {1, 2}};
    fum.insert(ilist);
    
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(ilist.size(), 2u);
    EXPECT_EQ(ilist.begin()->first, 1);
  }
  {
    flat_umap<int, int> fum;
    fum.insert({{1, 2}, {1, 2}});
    
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, InsertOrAssign)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    auto it = fum.insert_or_assign(1, 2);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);

    it = fum.insert_or_assign(1, 3);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 3);
    EXPECT_FALSE(it.second);
    EXPECT_EQ(fum[1], 3);
    EXPECT_EQ(fum.size(), 1u);

    it = fum.insert_or_assign(2, 4);
    ASSERT_NE(it.first, fum.end());
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fum[2], 4);
    EXPECT_EQ(fum.size(), 2u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    const DbgClass k(1);
    const DbgClass v(2);
    fum.insert_or_assign(k, v);
    
    EXPECT_TRUE(fum.contains(k));
    EXPECT_EQ(fum[k], v);
    EXPECT_EQ(fum.size(), 1u);
    
    const DbgClass v2(4);
    fum.insert_or_assign(k, v2);
    EXPECT_EQ(fum[k], v2);
    EXPECT_EQ(fum.size(), 1u);
    
    const DbgClass k3(4);
    const DbgClass v3(4);
    fum.insert_or_assign(k3, v3);
    EXPECT_EQ(fum[k3], v3);
    EXPECT_EQ(fum.size(), 2u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    fum.insert_or_assign(k, v);
    
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    fum.insert_or_assign(std::move(k), std::move(v));
    
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_NE(k, 1);
    EXPECT_NE(v, 2);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Emplace)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    auto it = fum.emplace(1, 2);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    
    it = fum.emplace(1, 3);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_FALSE(it.second);
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    
    it = fum.emplace(2, 4);
    ASSERT_NE(it.first, fum.end());
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fum[2], 4);
    EXPECT_EQ(fum.size(), 2u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fum.emplace(k, v);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fum.emplace(std::move(k), std::move(v));
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_NE(k, 1);
    EXPECT_NE(v, 2);
  }
  {
    flat_umap<std::string, std::string> fum;
    const char* k = "1";
    const char* v = "2";

    auto it = fum.emplace(k, v);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, k);
    EXPECT_EQ(it.first->second, v);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(k));
    EXPECT_EQ(fum[k], v);
    EXPECT_EQ(fum.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, TryEmplace)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    auto it = fum.try_emplace(1, 2);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    
    it = fum.try_emplace(1, 3);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_FALSE(it.second);
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    
    it = fum.try_emplace(2, 4);
    ASSERT_NE(it.first, fum.end());
    EXPECT_TRUE(it.second);
    EXPECT_EQ(fum[2], 4);
    EXPECT_EQ(fum.size(), 2u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fum.try_emplace(k, v);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(k, 1);
    EXPECT_EQ(v, 2);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    DbgClass k(1);
    DbgClass v(2);
    
    auto it = fum.try_emplace(std::move(k), std::move(v));
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_NE(k, 1);
    EXPECT_NE(v, 2);
    
    DbgClass k2(1);
    DbgClass v2(2);
    it = fum.try_emplace(std::move(k2), std::move(v2));
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, 1);
    EXPECT_EQ(it.first->second, 2);
    EXPECT_FALSE(it.second);
    EXPECT_TRUE(fum.contains(1));
    EXPECT_EQ(fum[1], 2);
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(k2, 1);
    EXPECT_EQ(v2, 2);
  }
  {
    flat_umap<std::string, std::string> fum;
    const char* k = "1";
    const char* v = "2";
    
    auto it = fum.try_emplace(k, v);
    ASSERT_NE(it.first, fum.end());
    EXPECT_EQ(it.first->first, k);
    EXPECT_EQ(it.first->second, v);
    EXPECT_TRUE(it.second);
    EXPECT_TRUE(fum.contains(k));
    EXPECT_EQ(fum[k], v);
    EXPECT_EQ(fum.size(), 1u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Erase)
{
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}};
    EXPECT_EQ(fum.erase(1), 1u);
    EXPECT_FALSE(fum.contains(1));
    EXPECT_TRUE(fum.contains(3));
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    
    EXPECT_EQ(fum.erase(1), 0u);
    EXPECT_FALSE(fum.contains(1));
    EXPECT_TRUE(fum.contains(3));
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 2u);
    
    EXPECT_EQ(fum.erase(3), 1u);
    EXPECT_FALSE(fum.contains(1));
    EXPECT_FALSE(fum.contains(3));
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 2u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    EXPECT_EQ(fum.erase(1), 0u);
    EXPECT_FALSE(fum.contains(1));
    EXPECT_EQ(fum.size(), 0u);
    EXPECT_EQ(fum.bucket_count(), 0u);
    
    fum[2] = 2;
    EXPECT_EQ(fum.erase(1), 0u);
    EXPECT_TRUE(fum.contains(2));
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_EQ(fum.bucket_count(), 2u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, EraseIter)
{
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}};
    auto first = fum.begin();
    bool is1 = first->first == 1;
    
    fum.erase(first);
    EXPECT_EQ(fum.contains(1), !is1);
    EXPECT_EQ(fum.contains(3), is1);
    EXPECT_EQ(fum.size(), 1u);
    
    first = fum.begin();
    fum.erase(first);
    EXPECT_FALSE(fum.contains(1));
    EXPECT_FALSE(fum.contains(3));
    EXPECT_EQ(fum.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 2}, {3, 4}};
    auto first = fum.begin();
    bool is1 = first->first == 1;
    
    auto it = fum.erase_(first);
    ASSERT_NE(it, fum.end());
    EXPECT_EQ(it->first, is1 ? 3 : 1);
    EXPECT_EQ(it->second, is1 ? 4 : 2);
    EXPECT_EQ(fum.contains(1), !is1);
    EXPECT_EQ(fum.contains(3), is1);
    EXPECT_EQ(fum.size(), 1u);
    
    first = fum.begin();
    it = fum.erase_(first);
    ASSERT_EQ(it, fum.end());
    EXPECT_FALSE(fum.contains(1));
    EXPECT_FALSE(fum.contains(3));
    EXPECT_EQ(fum.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum;
    for (int i = 1; i <= 100; ++i)
      fum.emplace(i, 100 + i);
    EXPECT_EQ(fum.size(), 100u);
    
    auto it = fum.begin();
    while (!fum.empty())
      it = fum.erase_(it);
    EXPECT_EQ(fum.size(), 0u);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, EraseIf)
{
  {
    flat_umap<DbgClass, DbgClass> fum;
    
    erase_if(fum, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fum.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 1}, {3, 3}};
    
    erase_if(fum, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fum.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{1, 1}, {2, 2}, {3, 3}};
    
    erase_if(fum, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fum.size(), 1u);
    EXPECT_FALSE(fum.contains(1));
    EXPECT_TRUE(fum.contains(2));
    EXPECT_FALSE(fum.contains(3));
  }
  {
    flat_umap<DbgClass, DbgClass> fum{{2, 1}, {4, 3}};
    
    erase_if(fum, [](const auto& item){ return (item.first.id % 2); });
    EXPECT_EQ(fum.size(), 2u);
    EXPECT_TRUE(fum.contains(2));
    EXPECT_TRUE(fum.contains(4));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Swap)
{
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2;
    
    fum2.swap(fum1);
    EXPECT_EQ(fum1.size(), 0u);
    EXPECT_EQ(fum2.size(), 0u);
    
    fum2.swap(fum1);
    EXPECT_EQ(fum1.size(), 0u);
    EXPECT_EQ(fum2.size(), 0u);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}, {3, 4}, {5, 6}};
    flat_umap<DbgClass, DbgClass> fum2;
    
    fum2.swap(fum1);
    EXPECT_EQ(fum1.size(), 0u);
    EXPECT_EQ(fum2.size(), 3u);
    EXPECT_FALSE(fum1.contains(3));
    EXPECT_TRUE(fum2.contains(3));
    
    fum2.swap(fum1);
    EXPECT_EQ(fum1.size(), 3u);
    EXPECT_EQ(fum2.size(), 0u);
    EXPECT_TRUE(fum1.contains(3));
    EXPECT_FALSE(fum2.contains(3));
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}, {3, 4}};
    flat_umap<DbgClass, DbgClass> fum2{{5, 6}};
    
    fum2.swap(fum1);
    EXPECT_EQ(fum1.size(), 1u);
    EXPECT_EQ(fum2.size(), 2u);
    EXPECT_TRUE(fum1.contains(5));
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
    EXPECT_FALSE(fum2.contains(5));
    
    fum2.swap(fum1);
    EXPECT_EQ(fum1.size(), 2u);
    EXPECT_EQ(fum2.size(), 1u);
    EXPECT_TRUE(fum1.contains(3));
    EXPECT_FALSE(fum1.contains(5));
    EXPECT_TRUE(fum2.contains(5));
    EXPECT_FALSE(fum2.contains(3));
  }
  {
    using std::swap;
    
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}, {3, 4}};
    flat_umap<DbgClass, DbgClass> fum2{{5, 6}};

    swap(fum1, fum2);
    EXPECT_EQ(fum1.size(), 1u);
    EXPECT_EQ(fum2.size(), 2u);
    EXPECT_TRUE(fum1.contains(5));
    EXPECT_FALSE(fum1.contains(1));
    EXPECT_TRUE(fum2.contains(1));
    EXPECT_FALSE(fum2.contains(5));

    swap(fum1, fum2);
    EXPECT_EQ(fum1.size(), 2u);
    EXPECT_EQ(fum2.size(), 1u);
    EXPECT_TRUE(fum1.contains(3));
    EXPECT_FALSE(fum1.contains(5));
    EXPECT_TRUE(fum2.contains(5));
    EXPECT_FALSE(fum2.contains(3));
  }
  {
    using std::swap;
    
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2{{5, 6}, {4, 3}};
    
    swap(fum1, fum1);
    EXPECT_EQ(fum1.size(), 1u);
    EXPECT_TRUE(fum1.contains(1));
    
    swap(fum2, fum1);
    fum1.swap(fum2);
    EXPECT_EQ(fum1.size(), 1u);
    EXPECT_EQ(fum2.size(), 2u);
    EXPECT_TRUE(fum1.contains(1));
    EXPECT_FALSE(fum1.contains(5));
    EXPECT_TRUE(fum2.contains(5));
    EXPECT_FALSE(fum2.contains(1));
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, Equality)
{
  {
    flat_umap<DbgClass, DbgClass> fum1;
    flat_umap<DbgClass, DbgClass> fum2;
    EXPECT_EQ(fum1, fum2);
    EXPECT_EQ(fum2, fum1);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2{{1, 2}};
    EXPECT_EQ(fum1, fum2);
    EXPECT_EQ(fum2, fum1);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2{{5, 6}};
    EXPECT_NE(fum1, fum2);
    EXPECT_NE(fum2, fum1);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2{{1, 3}};
    EXPECT_NE(fum1, fum2);
    EXPECT_NE(fum2, fum1);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}};
    flat_umap<DbgClass, DbgClass> fum2{{1, 2}, {3, 4}};
    EXPECT_NE(fum1, fum2);
    EXPECT_NE(fum2, fum1);
  }
  {
    flat_umap<DbgClass, DbgClass> fum1{{1, 2}, {3, 5}};
    flat_umap<DbgClass, DbgClass> fum2{{1, 2}, {3, 4}};
    EXPECT_NE(fum1, fum2);
    EXPECT_NE(fum2, fum1);
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}

TEST(FlatUMapTest, BadHash)
{
  struct bad_hash {
    size_t operator()(const int& x) const { return x; }
  };
  
  flat_umap<int, int, bad_hash> fum;
  std::unordered_map<int, int, bad_hash> map;
  for (int i = 0; i < 100000; ++i)
  {
    fum.emplace(i, i);
    map.emplace(i, i);
  }
  EXPECT_EQ(fum.size(), map.size());
  
  for (const auto& item : map)
  {
    auto itF = fum.find(item.first);
    ASSERT_NE(itF, fum.end());
    EXPECT_EQ(itF->second, item.second);
  }
}

TEST(FlatUMapTest, Stress)
{
  {
    auto seed = time(NULL);
    auto get_rand = []() {
      int r = rand() + 1;
      return (RAND_MAX > 32768) ? r : r * rand() + 1;
    };
    
    std::cout << "Stress seed: " << seed << "\n";
    srand((unsigned int)seed);
    
    flat_umap<DbgClass, DbgClass> fum;
    std::unordered_map<DbgClass, DbgClass> map;
    std::vector<DbgClass> vec;
    
    int addCount = 0;
    int updateCount = 0;
    int removeCount = 0;
    int rehashCount = 0;

    for (int i = 0; i < 1000000; ++i)
    {
      int op = rand() % 8;
      switch(op)
      {
        case 0: // insert
        {
          int v = get_rand();
          int k = get_rand();
          fum.insert({k, v});
          map.insert({k, v});
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fum.size(), map.size()) << "insert";
          break;
        }
        case 1: // emplace
        {
          int v = get_rand();
          int k = get_rand();
          fum.emplace(k, v);
          map.emplace(k, v);
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fum.size(), map.size()) << "emplace";
          break;
        }
        case 2: // insert_or_assign
        {
          int v = get_rand();
          int k = get_rand();
          fum.insert_or_assign(k, v);
          map[k] = v;
          vec.push_back(k);
          ++addCount;
          EXPECT_EQ(fum.size(), map.size()) << "insert_or_assign";
          break;
        }
        case 3: // update
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            int v = rand() + 1;
            fum[k] = v;
            map[k] = v;
            ++updateCount;
          }
          EXPECT_EQ(fum.size(), map.size()) << "update";
          break;
        }
        case 4: // erase
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto countF = fum.erase(k);
            auto countM = map.erase(k);
            ++removeCount;
            EXPECT_EQ(countF, countM);
          }
          EXPECT_EQ(fum.size(), map.size()) << "erase";
          break;
        }
        case 5: // erase first
        {
          if (!map.empty())
          {
            auto itM = map.begin();
            auto itF = fum.find(itM->first);
            ASSERT_NE(itF, fum.end());
            EXPECT_EQ(itF->second, itM->second);
            fum.erase_(itF);
            map.erase(itM);
            ++removeCount;
          }
          EXPECT_EQ(fum.size(), map.size()) << "erase first";
          break;
        }
        case 6: // find
        {
          if (!vec.empty())
          {
            auto k = vec[rand() % vec.size()];
            auto itF = fum.find(k);
            auto itM = map.find(k);
            if (itF != fum.end())
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
            fum.rehash(j);
            ++rehashCount;
          }
          EXPECT_EQ(fum.size(), map.size()) << "rehash";
          break;
        }
        default:
          assert(false);
      }
    }
    std::cout << "Stress final size: " << map.size()
              << ", add: " << addCount
              << ", update: " << updateCount
              << ", remove: " << removeCount
              << ", rehash: " << rehashCount << "\n";
    EXPECT_EQ(fum.size(), map.size()) << "final";

    for (const auto& item : map)
    {
      auto itF = fum.find(item.first);
      ASSERT_NE(itF, fum.end());
      EXPECT_EQ(itF->second, item.second);
    }
  }
  // No object leak
  EXPECT_EQ(DbgClass::count, 0);
}
