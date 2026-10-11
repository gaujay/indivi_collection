/**
 * Copyright 2025 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#ifndef INDIVI_FLAT_WTABLE_H
#define INDIVI_FLAT_WTABLE_H

#include "indivi/hash.h"
#include "indivi/detail/indivi_defines.h"
#include "indivi/detail/indivi_utils.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

#ifdef INDIVI_SIMD_SSE2
  #include <emmintrin.h>
#elif defined(INDIVI_SIMD_L_ENDIAN_NEON)
  #include <arm_neon.h>
#else
  #error indivi flat unordered containers need SSE2 or little endian NEON support
#endif

#ifdef INDIVI_FLAT_W_DEBUG
  #include <cassert>
  #define INDIVI_WTABLE_ASSERT(x) assert(x)
#else
  #define INDIVI_WTABLE_ASSERT(x)
#endif

// use faster quadratic probing
// else use better distributed double-hashing linear probing
#define INDIVI_FLAT_W_QUAD_PROB

namespace indivi
{
namespace detail
{
/*
 * Metadata functions for group of 16 buckets.
 * SIMD-accelerated, used by `flat_wtable`.
 * Each entry is an 1-Byte hash fragment, or empty/tombstone.
 */
namespace MetaWGroup
{
  static constexpr uint8_t EMPTY_FRAG{ 0x7F };     // 127
  static constexpr uint8_t TOMBSTONE_FRAG{ 0x7E }; // 126
  static constexpr uint8_t SETMAX_FRAG{ 0x7D };    // 125
  static constexpr uint8_t SENTINEL_FRAG{ 0x00 };  // arbitrary valid
  static constexpr int EMPTY_FRAGS{ 0x7F7F7F7F };
  static constexpr int TOMBSTONE_FRAGS{ 0x7E7E7E7E };
  static constexpr int SETMAX_FRAGS{ 0x7D7D7D7D };

  static inline int match_word(std::size_t hash) noexcept
  {
    // arbitrary remap reserved TOMBSTONE_FRAG and EMPTY_FRAG
    static constexpr uint32_t word[] = {
      0x00000000u,0x01010101u,0x02020202u,0x03030303u,0x04040404u,0x05050505u,
      0x06060606u,0x07070707u,0x08080808u,0x09090909u,0x0A0A0A0Au,0x0B0B0B0Bu,
      0x0C0C0C0Cu,0x0D0D0D0Du,0x0E0E0E0Eu,0x0F0F0F0Fu,0x10101010u,0x11111111u,
      0x12121212u,0x13131313u,0x14141414u,0x15151515u,0x16161616u,0x17171717u,
      0x18181818u,0x19191919u,0x1A1A1A1Au,0x1B1B1B1Bu,0x1C1C1C1Cu,0x1D1D1D1Du,
      0x1E1E1E1Eu,0x1F1F1F1Fu,0x20202020u,0x21212121u,0x22222222u,0x23232323u,
      0x24242424u,0x25252525u,0x26262626u,0x27272727u,0x28282828u,0x29292929u,
      0x2A2A2A2Au,0x2B2B2B2Bu,0x2C2C2C2Cu,0x2D2D2D2Du,0x2E2E2E2Eu,0x2F2F2F2Fu,
      0x30303030u,0x31313131u,0x32323232u,0x33333333u,0x34343434u,0x35353535u,
      0x36363636u,0x37373737u,0x38383838u,0x39393939u,0x3A3A3A3Au,0x3B3B3B3Bu,
      0x3C3C3C3Cu,0x3D3D3D3Du,0x3E3E3E3Eu,0x3F3F3F3Fu,0x40404040u,0x41414141u,
      0x42424242u,0x43434343u,0x44444444u,0x45454545u,0x46464646u,0x47474747u,
      0x48484848u,0x49494949u,0x4A4A4A4Au,0x4B4B4B4Bu,0x4C4C4C4Cu,0x4D4D4D4Du,
      0x4E4E4E4Eu,0x4F4F4F4Fu,0x50505050u,0x51515151u,0x52525252u,0x53535353u,
      0x54545454u,0x55555555u,0x56565656u,0x57575757u,0x58585858u,0x59595959u,
      0x5A5A5A5Au,0x5B5B5B5Bu,0x5C5C5C5Cu,0x5D5D5D5Du,0x5E5E5E5Eu,0x5F5F5F5Fu,
      0x60606060u,0x61616161u,0x62626262u,0x63636363u,0x64646464u,0x65656565u,
      0x66666666u,0x67676767u,0x68686868u,0x69696969u,0x6A6A6A6Au,0x6B6B6B6Bu,
      0x6C6C6C6Cu,0x6D6D6D6Du,0x6E6E6E6Eu,0x6F6F6F6Fu,0x70707070u,0x71717171u,
      0x72727272u,0x73737373u,0x74747474u,0x75757575u,0x76767676u,0x77777777u,
      0x78787878u,0x79797979u,0x7A7A7A7Au,0x7B7B7B7Bu,0x7C7C7C7Cu,0x7D7D7D7Du,
      0x29292929u,0x53535353u,0x80808080u,0x81818181u,0x82828282u,0x83838383u, // here
      0x84848484u,0x85858585u,0x86868686u,0x87878787u,0x88888888u,0x89898989u,
      0x8A8A8A8Au,0x8B8B8B8Bu,0x8C8C8C8Cu,0x8D8D8D8Du,0x8E8E8E8Eu,0x8F8F8F8Fu,
      0x90909090u,0x91919191u,0x92929292u,0x93939393u,0x94949494u,0x95959595u,
      0x96969696u,0x97979797u,0x98989898u,0x99999999u,0x9A9A9A9Au,0x9B9B9B9Bu,
      0x9C9C9C9Cu,0x9D9D9D9Du,0x9E9E9E9Eu,0x9F9F9F9Fu,0xA0A0A0A0u,0xA1A1A1A1u,
      0xA2A2A2A2u,0xA3A3A3A3u,0xA4A4A4A4u,0xA5A5A5A5u,0xA6A6A6A6u,0xA7A7A7A7u,
      0xA8A8A8A8u,0xA9A9A9A9u,0xAAAAAAAAu,0xABABABABu,0xACACACACu,0xADADADADu,
      0xAEAEAEAEu,0xAFAFAFAFu,0xB0B0B0B0u,0xB1B1B1B1u,0xB2B2B2B2u,0xB3B3B3B3u,
      0xB4B4B4B4u,0xB5B5B5B5u,0xB6B6B6B6u,0xB7B7B7B7u,0xB8B8B8B8u,0xB9B9B9B9u,
      0xBABABABAu,0xBBBBBBBBu,0xBCBCBCBCu,0xBDBDBDBDu,0xBEBEBEBEu,0xBFBFBFBFu,
      0xC0C0C0C0u,0xC1C1C1C1u,0xC2C2C2C2u,0xC3C3C3C3u,0xC4C4C4C4u,0xC5C5C5C5u,
      0xC6C6C6C6u,0xC7C7C7C7u,0xC8C8C8C8u,0xC9C9C9C9u,0xCACACACAu,0xCBCBCBCBu,
      0xCCCCCCCCu,0xCDCDCDCDu,0xCECECECEu,0xCFCFCFCFu,0xD0D0D0D0u,0xD1D1D1D1u,
      0xD2D2D2D2u,0xD3D3D3D3u,0xD4D4D4D4u,0xD5D5D5D5u,0xD6D6D6D6u,0xD7D7D7D7u,
      0xD8D8D8D8u,0xD9D9D9D9u,0xDADADADAu,0xDBDBDBDBu,0xDCDCDCDCu,0xDDDDDDDDu,
      0xDEDEDEDEu,0xDFDFDFDFu,0xE0E0E0E0u,0xE1E1E1E1u,0xE2E2E2E2u,0xE3E3E3E3u,
      0xE4E4E4E4u,0xE5E5E5E5u,0xE6E6E6E6u,0xE7E7E7E7u,0xE8E8E8E8u,0xE9E9E9E9u,
      0xEAEAEAEAu,0xEBEBEBEBu,0xECECECECu,0xEDEDEDEDu,0xEEEEEEEEu,0xEFEFEFEFu,
      0xF0F0F0F0u,0xF1F1F1F1u,0xF2F2F2F2u,0xF3F3F3F3u,0xF4F4F4F4u,0xF5F5F5F5u,
      0xF6F6F6F6u,0xF7F7F7F7u,0xF8F8F8F8u,0xF9F9F9F9u,0xFAFAFAFAu,0xFBFBFBFBu,
      0xFCFCFCFCu,0xFDFDFDFDu,0xFEFEFEFEu,0xFFFFFFFFu
    };
    return (int)word[(unsigned char)hash];
  }

  static inline uint8_t* empty_group() noexcept
  {
    // 16+1+1 as mShift is 'sizeof(size_type) - 1' when empty (so index is 0 or 1)
    // plus a fake sentinel value past end to force stop iteration
    static constexpr uint8_t sEmptyGroup[]{ // dummy extended group for empty tables
      EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,
      EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,EMPTY_FRAG,
      EMPTY_FRAG, SENTINEL_FRAG
    };
    return const_cast<uint8_t*>(sEmptyGroup);
  }

#ifdef INDIVI_SIMD_SSE2
  static inline __m128i load_hfrags(const uint8_t* hfrags) noexcept
  {
    return _mm_loadu_si128(reinterpret_cast<const __m128i*>(hfrags));
  }

  static inline int match_hfrag(__m128i hfrags, std::size_t hash) noexcept
  {
    return _mm_movemask_epi8(
        _mm_cmpeq_epi8(hfrags, _mm_set1_epi32(match_word(hash)))); // == hfrag
  }

  static inline int match_empty(__m128i hfrags) noexcept
  {
    return _mm_movemask_epi8(
        _mm_cmpeq_epi8(hfrags, _mm_set1_epi32(EMPTY_FRAGS))); // == empty
  }

  static inline int match_empty(const uint8_t* hfrags) noexcept
  {
    return _mm_movemask_epi8(
        _mm_cmpeq_epi8(load_hfrags(hfrags), _mm_set1_epi32(EMPTY_FRAGS))); // == empty
  }

  static inline int match_available(const uint8_t* hfrags) noexcept
  {
    return _mm_movemask_epi8(
        _mm_cmpgt_epi8(load_hfrags(hfrags), _mm_set1_epi32(SETMAX_FRAGS))); // > max set
  }
  static inline int match_set(const uint8_t* hfrags) noexcept
  {
    return _mm_movemask_epi8(
        _mm_cmplt_epi8(load_hfrags(hfrags), _mm_set1_epi32(TOMBSTONE_FRAGS))); // < tombstone
  }
#else // NEON
  static inline int mm_movemask_epi8(uint8x16_t v) noexcept
  {
    // mask with powers of 2, group pairs as u16 then accumulate
    static constexpr uint8_t mask[] = {
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128
    };

    uint8x16_t vmask = vandq_u8(vld1q_u8(mask), v);
    uint8x8x2_t vzip = vzip_u8(vget_low_u8(vmask), vget_high_u8(vmask));
    uint16x8_t vmix = vreinterpretq_u16_u8(vcombine_u8(vzip.val[0], vzip.val[1]));

  #ifdef INDIVI_ARCH_ARM64
    return vaddvq_u16(vmix);
  #else
    uint64x2_t vsum = vpaddlq_u32(vpaddlq_u16(vmix));
    return int(vgetq_lane_u64(vsum, 0)) + int(vgetq_lane_u64(vsum, 1));
  #endif
  }

  static inline int8x16_t load_hfrags(const uint8_t* hfrags) noexcept
  {
    return vld1q_s8(reinterpret_cast<const int8_t*>(hfrags));
  }

  static inline int match_hfrag(int8x16_t hfrags, std::size_t hash) noexcept
  {
    return mm_movemask_epi8(
        vceqq_s8(hfrags, vdupq_n_s8((int8_t)match_word(hash))));
  }

  static inline int match_empty(int8x16_t hfrags) noexcept
  {
    return mm_movemask_epi8(
        vceqq_s8(hfrags, vdupq_n_s8((int8_t)EMPTY_FRAG)));
  }

  static inline int match_empty(const uint8_t* hfrags) noexcept
  {
    return mm_movemask_epi8(
        vceqq_s8(load_hfrags(hfrags), vdupq_n_s8((int8_t)EMPTY_FRAG)));
  }

  static inline int match_available(const uint8_t* hfrags) noexcept
  {
    return mm_movemask_epi8(
        vcgtq_s8(load_hfrags(hfrags), vdupq_n_s8((int8_t)SETMAX_FRAG))); // > max set
  }
  static inline int match_set(const uint8_t* hfrags) noexcept
  {
    return mm_movemask_epi8(
        vcltq_s8(load_hfrags(hfrags), vdupq_n_s8((int8_t)TOMBSTONE_FRAG))); // < tombstone
  }
#endif

  static inline bool set_hfrag(uint8_t* groups, std::size_t hash, std::size_t index, std::size_t mask) noexcept
  {
    INDIVI_WTABLE_ASSERT(index <= mask);
    INDIVI_WTABLE_ASSERT((int8_t)groups[index] >= TOMBSTONE_FRAG);

    uint8_t hfrag = (uint8_t)match_word(hash);
    bool wasTombstone = groups[index] == TOMBSTONE_FRAG;
    groups[index] = hfrag;
    std::size_t extra = (index >= 15) ? 0u : mask + 1; // sync first group duplicate at end
    groups[index + extra] = hfrag;
    return wasTombstone;
  }

  static inline bool reset_hfrag(uint8_t* groups, std::size_t index, std::size_t mask) noexcept
  {
    // Check group before and after to see if self was part of a full group at some point
    // if not, then no need to use a tombstone (i.e. never caused probe hopping)
    INDIVI_WTABLE_ASSERT(index <= mask);
    INDIVI_WTABLE_ASSERT((int8_t)groups[index] < TOMBSTONE_FRAG);

    std::size_t leftIdx = (index - 16u) & mask;
    int leftEmpties = match_empty(groups + leftIdx); // not including self
    int rightEmpties = match_empty(groups + index);  // including self
    leftEmpties  |= 0x01;    // force first bit so not zero for clz
    rightEmpties |= 0x10000; // force 17th bit so not zero for ctz
    int leftNSize = last_bit_index(leftEmpties);   // before self (in [15,0])
    int rightSize = first_bit_index(rightEmpties); // after and including self (in [1,16])
    bool needTombstone = rightSize > leftNSize; // simplification of 'rightSize + (15 - leftNSize) >= 16'

    uint8_t val = needTombstone ? TOMBSTONE_FRAG : EMPTY_FRAG;
    groups[index] = val;
    std::size_t extra = (index >= 15) ? 0u : mask + 1; // sync first group duplicate at end
    groups[index + extra] = val;
    return needTombstone;
  }
};

/*
 * Underlying class of `flat_wmap` and `flat_wset`.
 */
template<
  class Key,
  class T,
  class value_type,
  class item_type,
  class size_type,
  class Hash,
  class KeyEqual,
  class Allocator >
class flat_wtable
{
public:
  using key_type = Key;
  using mapped_type = T;
  using difference_type = typename std::make_signed<size_type>::type;
  using hasher = Hash;
  using key_equal = KeyEqual;
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<Allocator>;
  using iter_reference = typename std::conditional<std::is_same<key_type, value_type>::value, const key_type&, value_type&>::type;
  using iter_const_reference = const value_type&; // for flat_wset, iter_reference is also const
  using iter_pointer = typename std::allocator_traits<Allocator>::pointer;
  using iter_const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

private:
  using init_type = item_type;
  using insert_type = std::pair<typename std::remove_const<Key>::type, const mapped_type>;
  using storage_type = typename std::aligned_storage<sizeof(item_type), alignof(item_type)>::type;
  using storage_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<storage_type>;
  using storage_alloc_traits = std::allocator_traits<storage_allocator_type>;
  using mixer = typename std::conditional<hash_is_avalanching<Hash>::value, no_mix, bit_mix>::type;

  static constexpr float MAX_LOAD_FACTOR{ 0.8f };
  static constexpr unsigned int MIN_CAPA{ 2u };
  static constexpr size_type EMPTY_SHIFT{ sizeof(size_type) * CHAR_BIT - 1u };

  static constexpr bool is_pow2(std::size_t v)  noexcept { return (v & (v - 1)) == 0u; }

  static_assert(MAX_LOAD_FACTOR > 0.f && MAX_LOAD_FACTOR < 1.f, "flat_wtable: MAX_LOAD_FACTOR must be > 0 and < 1");
  static_assert(MIN_CAPA >= 2u, "flat_wtable: MIN_CAPA must be >= 2");
  static_assert(is_pow2(MIN_CAPA), "flat_wtable: MIN_CAPA must be a power of 2");

#ifdef INDIVI_FLAT_W_STATS
  struct MFindStats
  {
    std::size_t find_hit_count = 0;
    std::size_t find_miss_count = 0;
    std::size_t prob_hit_len = 0;
    std::size_t prob_hit_max = 0;
    std::size_t prob_miss_len = 0;
    std::size_t prob_miss_max = 0;
    std::size_t cmp_hit = 0;
    std::size_t cmp_hit_max = 0;
    std::size_t cmp_miss = 0;
    std::size_t cmp_miss_max = 0;
  };
  mutable MFindStats mStats;
#endif

  struct Location
  {
    item_type* value;
    size_type index;
  };

  struct Groups : Hash // empty base optimization
  {
    uint8_t* data = MetaWGroup::empty_group(); // static const dummy

    Groups(const Hash& hash) : Hash(hash) {}
    Groups(const Groups& groups) : Hash(groups) {}
    Groups(Groups&& groups) noexcept(std::is_nothrow_move_constructible<Hash>::value)
      : Hash(std::move(groups.hash_move()))
      , data(groups.data)
    {}

    Hash& hash() noexcept { return *this; }
    const Hash& hash() const noexcept { return *this; }
    Hash&& hash_move() noexcept { return std::move(hash()); }
  };

  struct Values : EboWrapper<KeyEqual, 0>, EboWrapper<storage_allocator_type, 1> // empty base optimization
  {
    item_type* data = nullptr;

    Values(const KeyEqual& equal, const Allocator& alloc)
      : EboWrapper<KeyEqual, 0>(equal)
      , EboWrapper<storage_allocator_type, 1>(alloc)
    {}

    Values(KeyEqual&& equal, const Allocator& alloc)
      : EboWrapper<KeyEqual, 0>(std::move(equal))
      , EboWrapper<storage_allocator_type, 1>(alloc)
    {}

    Values(const Values& values)
      : EboWrapper<KeyEqual, 0>(values.equal())
      , EboWrapper<storage_allocator_type, 1>(values.alloc())
    {}

    Values(Values&& values)
        noexcept(std::is_nothrow_move_constructible<KeyEqual>::value && std::is_nothrow_move_constructible<storage_allocator_type>::value)
      : EboWrapper<KeyEqual, 0>(std::move(values.equal_move()))
      , EboWrapper<storage_allocator_type, 1>(std::move(values.alloc_move()))
      , data(values.data)
    {}

    KeyEqual& equal() noexcept { return EboWrapper<KeyEqual, 0>::get(); }
    const KeyEqual& equal() const noexcept { return EboWrapper<KeyEqual, 0>::get(); }
    KeyEqual&& equal_move() noexcept { return std::move(equal()); }

    storage_allocator_type& alloc() noexcept { return EboWrapper<storage_allocator_type, 1>::get(); }
    const storage_allocator_type& alloc() const noexcept { return EboWrapper<storage_allocator_type, 1>::get(); }
    storage_allocator_type&& alloc_move() noexcept { return std::move(alloc()); }

    value_type* cdata() const noexcept { return reinterpret_cast<value_type*>(data); }
  };

  // Members
  size_type mSize    = 0u;          // number of items
  size_type mShift   = EMPTY_SHIFT; // hash shift to keep highest bits (as location index)
  size_type mGMask   = 0u;          // group index mask (as power-of-2 capacity - 1)
  size_type mMaxSize = 0u;          // max size before rehash is needed
  Groups mGroups;                   // contains hash fragments (processed as 16-bytes groups)
  Values mValues;                   // contains key-mapped pairs (matching mGroups entries)

  size_type group_capa() const noexcept { return mValues.data ? mGMask + 1u : 0u; }
  Hash& hash() noexcept { return mGroups.hash(); }
  const Hash& hash() const noexcept { return mGroups.hash(); }
  KeyEqual& equal() noexcept { return mValues.equal(); }
  const KeyEqual& equal() const noexcept { return mValues.equal(); }
  storage_allocator_type& storage_alloc() noexcept { return mValues.alloc(); }
  const storage_allocator_type& storage_alloc() const noexcept { return mValues.alloc(); }

  template< typename P, class... Args >
  void construct_item(P* p, Args&&... args)
  {
    storage_alloc_traits::construct(storage_alloc(), p, std::forward<Args>(args)...);
  }

  template< typename P >
  void destroy_item(P* p)
  {
    storage_alloc_traits::destroy(storage_alloc(), p);
  }

public:
  template <typename Pointer, typename Reference>
  class Iterator
  {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = typename flat_wtable::difference_type;
    using pointer = Pointer;
    using reference = Reference;

  private:
    friend flat_wtable;

    int mSets = 0;
    const uint8_t* mGroup = nullptr;
    value_type* mValue = nullptr;
    const value_type* mValueLast = nullptr;

    Iterator(const uint8_t* group, item_type* value, item_type* valueLast)
      : mGroup(group), mValue(reinterpret_cast<value_type*>(value)), mValueLast(reinterpret_cast<value_type*>(valueLast)) // non-strict-aliasing of const
    {}

    static Iterator find_begin(const uint8_t* groups, item_type* values, size_type mask)
    {
      return ++Iterator(groups - 1, values - 1, values + mask);
    }

  public:
    Iterator() = default;
    Iterator(const Iterator& other) = default;

    template <typename P, typename R,
      typename = std::enable_if<std::is_convertible<P, pointer>::value>>
    Iterator(const Iterator<P, R>& other)
        : mSets(other.mSets), mGroup(other.mGroup), mValue(other.mValue), mValueLast(other.mValueLast)
    {}

    Iterator& operator=(const Iterator& other) = default;

    Iterator& operator++() noexcept
    {
      if (mSets)
      {
        int idx = first_bit_index(mSets) + 1;
        mGroup += idx;
        mValue += idx;
        mSets >>= idx; // remove match

        mValue = (mValue <= mValueLast) ? mValue : nullptr; // end
        return *this;
      }
      mGroup -= 15;
      mValue -= 15;
      do {
        mGroup += 16;
        mValue += 16;
        mSets = MetaWGroup::match_set(mGroup);
      }
      while (mSets == 0);

      INDIVI_WTABLE_ASSERT(mValue <= mValueLast + 16);
      INDIVI_PREFETCH(mValue);
      int idx = first_bit_index(mSets);
      mGroup += idx;
      mValue += idx;
      mSets >>= ++idx; // remove match

      mValue = (mValue <= mValueLast) ? mValue : nullptr; // end
      return *this;
    }

    Iterator operator++(int) noexcept
    {
      Iterator retval = *this;
      ++(*this);
      return retval;
    }

    bool operator==(const Iterator& other) const noexcept
    {
      return mValue == other.mValue;
    }

    bool operator!=(const Iterator& other) const noexcept
    {
      return mValue != other.mValue;
    }

    reference operator*() const noexcept
    {
      return *mValue;
    }

    pointer operator->() const noexcept
    {
      return mValue;
    }
  };

  using iterator = Iterator<iter_pointer, iter_reference>;
  using const_iterator = Iterator<iter_const_pointer, iter_const_reference>;

  // Ctr/Dtr
  flat_wtable() : flat_wtable(0)
  {}

  explicit flat_wtable(size_type bucket_count, const Hash& hash = Hash(), const key_equal& equal = key_equal(), const Allocator& alloc = Allocator())
    : mGroups(hash)
    , mValues(equal, alloc)
  {
    rehash(bucket_count);
  }

  template< class InputIt >
  flat_wtable(InputIt first, InputIt last, size_type bucket_count = 0, const Hash& hash = Hash(),
      const key_equal& equal = key_equal(), const Allocator& alloc = Allocator())
    : flat_wtable(bucket_count, hash, equal, alloc)
  {
    try {
      insert(first, last);
    }
    catch (...) {
      destroy();
      throw;
    }
  }

  flat_wtable(const std::initializer_list<value_type>& init, size_type bucket_count = 0, const Hash& hash = Hash(),
      const key_equal& equal = key_equal(), const Allocator& alloc = Allocator())
    : flat_wtable(init.begin(), init.end(), bucket_count, hash, equal, alloc)
  {}

  flat_wtable(const flat_wtable& other)
    : mGroups(other.mGroups)
    , mValues(other.equal(), alloc_traits::select_on_container_copy_construction(other.get_allocator()))
  {
    copy_content(other);
  }

  flat_wtable(const flat_wtable& other, const Allocator& alloc)
    : mGroups(other.mGroups)
    , mValues(other.equal(), alloc)
  {
    copy_content(other);
  }

  flat_wtable(flat_wtable&& other) noexcept(
      std::is_nothrow_move_constructible<Hash>::value &&
      std::is_nothrow_move_constructible<key_equal>::value &&
      std::is_nothrow_move_constructible<storage_allocator_type>::value)
    : mSize(other.mSize)
    , mShift(other.mShift)
    , mGMask(other.mGMask)
    , mMaxSize(other.mMaxSize)
    , mGroups(std::move(other.mGroups))
    , mValues(std::move(other.mValues))
  {
    other.mSize = 0u;
    other.mShift = EMPTY_SHIFT;
    other.mGMask = 0u;
    other.mMaxSize = 0u;
    other.mGroups.data = MetaWGroup::empty_group();
    other.mValues.data = nullptr;
  }

  flat_wtable(flat_wtable&& other, const Allocator& alloc)
    : mGroups(std::move(other.mGroups))
    , mValues(std::move(other.equal()), alloc)
  {
    mGroups.data = MetaWGroup::empty_group(); // allocators may differs: let move_construct_alloc decide
    move_construct_alloc(std::move(other));
  }

  ~flat_wtable()
  {
    destroy();
  }

  // Assignment
  void operator=(const flat_wtable& other)
  {
    if (this != std::addressof(other))
    {
      clear();

      using pocca = typename alloc_traits::propagate_on_container_copy_assignment;
      if (pocca::value)
      {
        if (storage_alloc() != other.storage_alloc())
        {
          destroy_empty();
          storage_alloc() = other.storage_alloc();
        }
      }

      hash()  = other.hash();
      equal() = other.equal();

      copy_content(other);
    }
  }

  void operator=(flat_wtable&& other) noexcept(
      storage_alloc_traits::propagate_on_container_move_assignment::value &&
      std::is_nothrow_move_assignable<Hash>::value &&
      std::is_nothrow_move_assignable<key_equal>::value)
  {
    if (this != std::addressof(other))
    {
      using pocma = typename storage_alloc_traits::propagate_on_container_move_assignment;

      if (pocma::value || storage_alloc() == other.storage_alloc())
      {
        destroy();
        mSize = 0u;
        mShift = EMPTY_SHIFT;
        mGMask = 0u;
        mMaxSize = 0u;

        if (pocma::value)
          storage_alloc() = std::move(other.storage_alloc());

        hash()  = std::move(other.hash());
        equal() = std::move(other.equal());

        mSize = other.mSize;
        mShift = other.mShift;
        mGMask = other.mGMask;
        mMaxSize = other.mMaxSize;
        mGroups.data = other.mGroups.data;
        mValues.data = other.mValues.data;

        other.mSize = 0u;
        other.mShift = EMPTY_SHIFT;
        other.mGMask = 0u;
        other.mMaxSize = 0u;
        other.mGroups.data = MetaWGroup::empty_group();
        other.mValues.data = nullptr;
      }
      else
      {
        clear();
        hash()  = std::move(other.hash());
        equal() = std::move(other.equal());

        if (!other.empty())
        {
          reserve(other.mSize);
          other.uc_for_each([&](item_type* pValue) {
            insert_unique(std::move(*pValue));
            ++mSize;
          });
          other.clear();
        }
      }
    }
  }

  // Iterators
  iterator begin() noexcept { return iterator::find_begin(mGroups.data, mValues.data, mGMask); }
  const_iterator begin() const noexcept { return const_iterator::find_begin(mGroups.data, mValues.data, mGMask); }
  const_iterator cbegin() const noexcept { return const_iterator::find_begin(mGroups.data, mValues.data, mGMask); }

  static iterator end() noexcept { return iterator(); }
  static const_iterator cend() noexcept { return const_iterator(); }

  // Capacity
  bool empty() const noexcept { return mSize == 0u; }
  size_type size() const noexcept { return mSize; }
  size_type max_size() const noexcept { return (size_type)(max_bucket_count() * max_load_factor()); }

  // Bucket interface
  size_type bucket_count() const noexcept { return mGMask ? (mGMask + 1u) : 0u; }
  size_type max_bucket_count() const noexcept { return (size_type)std::numeric_limits<difference_type>::max(); }

  // Hash policy
  float load_factor() const noexcept { return mSize ? (float)mSize / bucket_count() : mSize; }
  float max_load_factor() const noexcept { return MAX_LOAD_FACTOR; }
  void max_load_factor(float) noexcept { /*for compatibility*/ }

  void rehash(size_type count)
  {
    INDIVI_WTABLE_ASSERT(count <= max_size());
    size_type minCapa = (size_type)std::ceil((float)size() / max_load_factor());
    count = std::max(count, minCapa);
    if (count)
    {
      count = std::max(count, (size_type)MIN_CAPA);
      count = std::min(count, max_bucket_count());
      count += (count == 8 || count == 16); // avoid full single group issue
      count = round_up_pow2(count);
      if (count != bucket_count())
        rehash_impl(count);
    }
    else
    {
      destroy_empty();
      mSize = 0u;
      mShift = EMPTY_SHIFT;
      mGMask = 0u;
      mMaxSize = 0u;
      mGroups.data = MetaWGroup::empty_group();
      mValues.data = nullptr;
    }
  }

  void reserve(size_type count)
  {
    INDIVI_WTABLE_ASSERT(count <= max_size());
    count = (count >= 16) ? (size_type)std::ceil((float)count / max_load_factor()) : count;
    rehash(count);
  }

  // Obervers
  hasher hash_function() const { return hash(); }
  key_equal key_eq() const { return equal(); }
  allocator_type get_allocator() const noexcept { return allocator_type(storage_alloc()); }

  // Lookup
  mapped_type& at(const Key& key)
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    if (!loc.value)
      throw std::out_of_range("flat_wtable::at");

    return loc.value->second;
  }
  const mapped_type& at(const Key& key) const
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    if (!loc.value)
      throw std::out_of_range("flat_wtable::at");

    return loc.value->second;
  }

  mapped_type& operator[](const Key& key)
  {
    return try_emplace_key_impl(key);
  }
  mapped_type& operator[](Key&& key)
  {
    return try_emplace_key_impl(std::move(key));
  }

  size_type count(const Key& key) const
  {
    return contains(key);
  }

  bool contains(const Key& key) const
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    return loc.value != nullptr;
  }

  iterator find(const Key& key)
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    return as_iter(loc);
  }
  const_iterator find(const Key& key) const
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    return as_const_iter(loc);
  }

  // Modifiers
  void clear() noexcept
  {
    INDIVI_WTABLE_ASSERT(mValues.data || mMaxSize == 0u);
    if (mValues.data)
    {
      uc_for_each([&](item_type* pValue) {
        destroy_item(pValue);
      });

      size_type capa = mGMask + 1u;
      std::memset((void*)mGroups.data, MetaWGroup::EMPTY_FRAG, sizeof(uint8_t) * (capa + 15u)); // for extra group, not including sentinel
      INDIVI_WTABLE_ASSERT(mGroups.data[sizeof(uint8_t) * (capa + 15u)] == MetaWGroup::SENTINEL_FRAG);
      mSize = 0u;
      mMaxSize = capa_to_maxsize(capa);
    }
  }

  template< class P >
  std::pair<iterator, bool> insert(P&& value)
  {
    return insert_impl(std::forward<P>(value));
  }

  std::pair<iterator, bool> insert(init_type&& value)
  {
    return insert_impl(std::move(value));
  }

  template< class InputIt >
  void insert(InputIt first, InputIt last)
  {
    for (; first != last; ++first)
      emplace(*first);
  }

  template <typename U>
  typename std::enable_if<
    std::is_same<U, value_type>::value || std::is_same<U, item_type>::value,
    void >::type
  insert_list(const std::initializer_list<U>& ilist)
  {
    insert(ilist.begin(), ilist.end());
  }

  template< class M >
  std::pair<iterator, bool> insert_or_assign(const Key& key, M&& obj)
  {
    return insert_or_assign_impl(key, std::forward<M>(obj));
  }

  template< class M >
  std::pair<iterator, bool> insert_or_assign(Key&& key, M&& obj)
  {
    return insert_or_assign_impl(std::move(key), std::forward<M>(obj));
  }

  template< class... Args >
  std::pair<iterator, bool> emplace(Args&&... args)
  {
    return try_insert_impl(item_type(std::forward<Args>(args)...));
  }

  template <typename P>
  typename std::enable_if<
    traits::is_similar<P, value_type>::value || traits::is_similar<P, init_type>::value || traits::is_similar<P, insert_type>::value,
    std::pair<iterator, bool> >::type
  emplace(P&& value)
  {
    return try_insert_impl(std::forward<P>(value));
  }

  template< class... Args >
  std::pair<iterator, bool> try_emplace(const Key& key, Args&&... args)
  {
    return try_emplace_impl(key, std::forward<Args>(args)...);
  }

  template< class... Args >
  std::pair<iterator, bool> try_emplace(Key&& key, Args&&... args)
  {
    return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
  }

  iterator erase_(iterator pos)
  {
    INDIVI_WTABLE_ASSERT(is_dereferenceable(pos));
    iterator res = pos;
    ++res;

    erase_impl(Location{ reinterpret_cast<item_type*>(pos.mValue), (size_type)(reinterpret_cast<item_type*>(pos.mValue) - mValues.data) });
    return res;
  }
  iterator erase_(const_iterator pos)
  {
    INDIVI_WTABLE_ASSERT(is_dereferenceable(pos));
    iterator res = pos;
    ++res;

    erase_impl(Location{ const_cast<item_type*>(reinterpret_cast<item_type*>(pos.mValue)),
                        (size_type)(reinterpret_cast<const item_type*>(pos.mValue) - mValues.data) });
    return res;
  }
  // non-standard, see `erase_()`
  void erase(iterator pos)
  {
    INDIVI_WTABLE_ASSERT(is_dereferenceable(pos));
    erase_impl(Location{ reinterpret_cast<item_type*>(pos.mValue), (size_type)(reinterpret_cast<item_type*>(pos.mValue) - mValues.data) });
  }
  void erase(const_iterator pos)
  {
    INDIVI_WTABLE_ASSERT(is_dereferenceable(pos));
    erase_impl(Location{ const_cast<item_type*>(reinterpret_cast<item_type*>(pos.mValue)),
                        (size_type)(reinterpret_cast<const item_type*>(pos.mValue) - mValues.data) });
  }

  size_type erase(const Key& key)
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    if (loc.value)
    {
      erase_impl(loc);
      return 1u;
    }
    return 0u;
  }

  void swap(flat_wtable& other)
    noexcept(traits::is_nothrow_swappable<Hash>::value &&
             traits::is_nothrow_swappable<key_equal>::value &&
            (!alloc_traits::propagate_on_container_swap::value || traits::is_nothrow_swappable<storage_allocator_type>::value))
  {
    using pocs = typename alloc_traits::propagate_on_container_swap;
    if (pocs::value)
    {
      using std::swap;
      swap(storage_alloc(), other.storage_alloc());
    }
    else
    {
      INDIVI_WTABLE_ASSERT(get_allocator() == other.get_allocator());
    }

    using std::swap;
    swap(hash(),  other.hash());
    swap(equal(), other.equal());

    swap(mSize,        other.mSize);
    swap(mShift,       other.mShift);
    swap(mGMask,       other.mGMask);
    swap(mMaxSize,     other.mMaxSize);
    swap(mGroups.data, other.mGroups.data);
    swap(mValues.data, other.mValues.data);
  }

  template <typename U>
  typename std::enable_if<
    !std::is_same<typename U::key_type, typename U::init_type>::value,
    bool >::type
  equal(const U& other) const
  {
    if (mSize != other.mSize)
      return false;

    auto itEnd = cend();
    auto otEnd = other.cend();
    for (auto it = cbegin(); it != itEnd; ++it)
    {
      auto ot = other.find(get_key(*it));
      if (ot == otEnd || it->second != ot->second)
        return false;
    }
    return true;
  }

  template <typename U>
  typename std::enable_if<
    std::is_same<typename U::key_type, typename U::init_type>::value,
    bool >::type
  equal(const U& other) const
  {
    if (mSize != other.mSize)
      return false;

    auto end_ = cend();
    for (auto it = cbegin(); it != end_; ++it)
    {
      if (!other.contains(*it))
        return false;
    }
    return true;
  }

  template< class Pred >
  size_type erase_if(Pred& pred)
  {
    size_type oldSize = mSize;
    iterator end_ = end();
    for (iterator it = begin(); it != end_; ++it)
    {
      if (pred(*it))
        erase(it);
    }
    return oldSize - mSize;
  }

#ifdef INDIVI_FLAT_W_DEBUG
  bool is_cleared() const noexcept
  {
    if (mValues.data)
    {
      auto capa = group_capa();
      for (size_type i = 0; i < capa + 15u; ++i) // with extra group
      {
        if (mGroups.data[i] != MetaWGroup::EMPTY_FRAG)
          return false;
      }
      if (mGroups.data[capa + 15u] != MetaWGroup::SENTINEL_FRAG)
        return false;
    }
    return true;
  }
#endif
#ifdef INDIVI_FLAT_W_STATS
  struct GroupStats
  {
    float full_grp_avg = 0.f;
    float tombstone_avg = 0.f;
  };

  GroupStats get_group_stats() const noexcept
  {
    GroupStats stats;
    if (empty())
      return stats;

    std::size_t full_count    = 0u;
    std::size_t tomb_count    = 0u;

    std::size_t grp_count = mGMask + 1;
    uint8_t* pGroup = mGroups.data;
    for (std::size_t idx = 0; idx < grp_count; ++idx)
    {
      full_count += !MetaWGroup::match_empty(pGroup + idx);
      tomb_count += pGroup[idx] == MetaWGroup::TOMBSTONE_FRAG;
    }

    // compute
    stats.full_grp_avg = (float)full_count / grp_count;
    stats.tombstone_avg = (float)tomb_count / grp_count;

    return stats;
  }

  struct FindStats
  {
    std::size_t find_hit_count = 0;
    std::size_t find_miss_count = 0;
    float prob_len_hit_avg = 0.f;
    std::size_t prob_len_hit_max = 0;
    float prob_len_miss_avg = 0.f;
    std::size_t prob_len_miss_max = 0;
    float compare_hit_avg = 0.f;
    std::size_t compare_hit_max = 0;
    float compare_miss_avg = 0.f;
    std::size_t compare_miss_max = 0;
  };

  FindStats get_find_stats() const noexcept
  {
    FindStats stats;

    stats.find_hit_count = mStats.find_hit_count;
    stats.find_miss_count = mStats.find_miss_count;

    stats.prob_len_hit_avg = mStats.find_hit_count ? (float)mStats.prob_hit_len / mStats.find_hit_count : 0.f;
    stats.prob_len_hit_max = mStats.prob_hit_max;
    stats.prob_len_miss_avg = mStats.find_miss_count ? (float)mStats.prob_miss_len / mStats.find_miss_count : 0.f;
    stats.prob_len_miss_max = mStats.prob_miss_max;

    stats.compare_hit_avg = mStats.find_hit_count ? (float)mStats.cmp_hit / mStats.find_hit_count : 0.f;
    stats.compare_hit_max = mStats.cmp_hit_max;
    stats.compare_miss_avg = mStats.find_miss_count ? (float)mStats.cmp_miss / mStats.find_miss_count : 0.f;
    stats.compare_miss_max = mStats.cmp_miss_max;

    return stats;
  }

  void reset_find_stats() noexcept
  {
    mStats.find_hit_count = 0;
    mStats.find_miss_count = 0;

    mStats.prob_hit_len = 0;
    mStats.prob_hit_max = 0;
    mStats.prob_miss_len = 0;
    mStats.prob_miss_max = 0;

    mStats.cmp_hit = 0;
    mStats.cmp_hit_max = 0;
    mStats.cmp_miss = 0;
    mStats.cmp_miss_max = 0;
  }
#endif // INDIVI_FLAT_W_STATS

private:
  // static functions
  static inline constexpr size_type hash_shift(size_type gCapa) noexcept
  {
    INDIVI_WTABLE_ASSERT(is_pow2(gCapa));
    return (gCapa <= 2u) ? EMPTY_SHIFT : sizeof(size_type) * CHAR_BIT - (size_type)(first_bit_index(gCapa));
  }

  static inline constexpr size_type hash_position(std::size_t hash, size_type shift) noexcept
  {
    return hash >> shift; // keep (64 - n) highest bits
  }

  static inline constexpr size_type capa_to_maxsize(size_type capa) noexcept
  {
    return (capa > 16) ? (size_type)(capa * MAX_LOAD_FACTOR)
               : (capa < 8) ? capa : capa - 1u; // force at least one empty in single group
  }

  static inline unsigned int gmask_to_setsmask(size_type gMask) noexcept
  {
    unsigned int maskShift = std::min((unsigned int)(gMask + 1u), 16u);
    return ~(unsigned int)(0xFFFFFFFFu << maskShift); // use mask to ignore extra group content
  }
#ifndef INDIVI_FLAT_W_QUAD_PROB
  static constexpr size_type prob_delta(std::size_t hash)
  {
    // use double-hashing based on hfrag
    // capacity is a power of 2 so all odd numbers are ok (coprime)
    return (size_type)((hash & 0xFF) | 0x01);
  }
#endif
  template< typename P >
  const Key& get_key(const P& value) const noexcept
  {
    return value.first;
  }

  const Key& get_key(const Key& value) const noexcept
  {
    return value;
  }

  // member functions
#if defined(INDIVI_FLAT_W_DEBUG) && !defined(NDEBUG)
  bool is_dereferenceable(iterator pos) const noexcept
  {
    INDIVI_WTABLE_ASSERT(mSize != 0u);
    INDIVI_WTABLE_ASSERT(mValues.data);
    INDIVI_WTABLE_ASSERT(pos.mGroup);
    INDIVI_WTABLE_ASSERT(pos.mValue);
    INDIVI_WTABLE_ASSERT(pos.mSets >= 0 && pos.mSets <= 0xFFFF);
    INDIVI_WTABLE_ASSERT(pos.mValueLast == reinterpret_cast<const value_type*>(mValues.cdata() + mGMask));
    INDIVI_WTABLE_ASSERT(pos.mValue >= reinterpret_cast<const value_type*>(mValues.cdata())
                      && pos.mValue <  reinterpret_cast<const value_type*>(mValues.cdata() + bucket_count()));
    return true;
  }
#else
  bool is_dereferenceable(iterator) const noexcept
  {
    return true;
  }
#endif

  std::size_t get_hash(const Key& key) const
  {
    return mixer::mix(hash(), key);
  }

  template< typename F >
  void uc_for_each(F fct) const
  {
    INDIVI_WTABLE_ASSERT(mValues.data);
    item_type* pValue = mValues.data;

    uint8_t* pGroup = mGroups.data;
    uint8_t* end = pGroup + mGMask + 1;
    unsigned int setsMask = gmask_to_setsmask(mGMask);
    do {
      int idx = 0;
      int sets = MetaWGroup::match_set(pGroup) & setsMask;
      while (sets)
      {
        if (sets & 0x01)
          fct(&pValue[idx]);

        sets >>= 1;
        ++idx;
      }
      pGroup += 16;
      pValue += 16;
    }
    while (pGroup < end);
  }

  template< typename F >
  void uc_each_while(F fct) const
  {
    INDIVI_WTABLE_ASSERT(mValues.data);
    item_type* pValue = mValues.data;

    uint8_t* pGroup = mGroups.data;
    uint8_t* end = pGroup + mGMask + 1;
    unsigned int setsMask = gmask_to_setsmask(mGMask);
    do {
      int idx = 0;
      int sets = MetaWGroup::match_set(pGroup) & setsMask;
      while (sets)
      {
        if (sets & 0x01)
        {
          if (!fct(&pValue[idx]))
            return;
        }
        sets >>= 1;
        ++idx;
      }
      pGroup += 16;
      pValue += 16;
    }
    while (pGroup < end);
  }

  void destroy()
  {
    if (mValues.data)
    {
      uc_for_each([&](item_type* pValue) {
        destroy_item(pValue);
      });
      storage_alloc_traits::deallocate(storage_alloc(), reinterpret_cast<storage_type*>(mValues.data),
                                       NewStorage::storageCapa(mGMask + 1u, mGMask + 1u + 16u));

      // reset state to prevent double-destruction during stack unwinding
      mValues.data = nullptr;
      mGroups.data = MetaWGroup::empty_group();
      mSize = 0u;
      mGMask = 0u;
      mShift = EMPTY_SHIFT;
      mMaxSize = 0u;
    }
  }

  void destroy_empty()
  {
    INDIVI_WTABLE_ASSERT(empty());
    if (mValues.data)
    {
      storage_alloc_traits::deallocate(storage_alloc(), reinterpret_cast<storage_type*>(mValues.data),
                                       NewStorage::storageCapa(mGMask + 1u, mGMask + 1u + 16u));
      mValues.data = nullptr;
      mGroups.data = MetaWGroup::empty_group();
      mGMask = 0u;
      mShift = EMPTY_SHIFT;
      mMaxSize = 0u;
    }
  }

  iterator as_iter(const Location& loc) const noexcept
  {
    return iterator(mGroups.data + loc.index, loc.value, mValues.data + mGMask);
  }

  const_iterator as_const_iter(const Location& loc) const noexcept
  {
    return const_iterator(mGroups.data + loc.index, loc.value, mValues.data + mGMask);
  }

  Location find_impl(std::size_t hash, size_type index, const Key& key) const
  {
  #ifdef INDIVI_FLAT_W_STATS
    std::size_t probLen = 1;
    std::size_t cmpCount = 0;
  #endif
  #ifdef INDIVI_FLAT_W_QUAD_PROB
    size_type delta = 0u;
  #endif
    do {
      const uint8_t* group = &mGroups.data[index];
      auto hfrags = MetaWGroup::load_hfrags(group);
      int matchs = MetaWGroup::match_hfrag(hfrags, hash);
      if (matchs)
      {
        item_type* pValue = &mValues.data[index];
        INDIVI_PREFETCH(pValue);
        do {
        #ifdef INDIVI_FLAT_W_STATS
          ++cmpCount;
        #endif
          int idx = first_bit_index(matchs);
          size_type valIdx = (index + idx) & mGMask;
          if (equal()(key, get_key(mValues.data[valIdx]))) // found
          {
          #ifdef INDIVI_FLAT_W_STATS
            mStats.prob_hit_len += probLen;
            mStats.prob_hit_max = (mStats.prob_hit_max >= probLen) ? mStats.prob_hit_max : probLen;
            mStats.cmp_hit += cmpCount;
            mStats.cmp_hit_max = (mStats.cmp_hit_max >= cmpCount) ? mStats.cmp_hit_max : cmpCount;
            ++mStats.find_hit_count;
          #endif
            return { mValues.data + valIdx, valIdx };
          }
          matchs &= matchs - 1; // remove match
        }
        while (matchs);
      }
      // not found
      if (MetaWGroup::match_empty(hfrags))
      {
      #ifdef INDIVI_FLAT_W_STATS
        mStats.prob_miss_len += probLen;
        mStats.prob_miss_max = (mStats.prob_miss_max >= probLen) ? mStats.prob_miss_max : probLen;
        mStats.cmp_miss += cmpCount;
        mStats.cmp_miss_max = (mStats.cmp_miss_max >= cmpCount) ? mStats.cmp_miss_max : cmpCount;
        ++mStats.find_miss_count;
      #endif
        return { nullptr, 0 };
      }
    #ifdef INDIVI_FLAT_W_STATS
      ++probLen;
    #endif
    #ifdef INDIVI_FLAT_W_QUAD_PROB
      index = (index + (++delta)*16) & mGMask;
    #else
      index += prob_delta(hash);
      index &= mGMask;
    #endif
    }
    while (index <= mGMask); // non-infinite loop helps optimization

    return { nullptr, 0 };
  }

  template< typename U >
  Location unchecked_insert(std::size_t hash, size_type index, U&& value)
  {
  #ifdef INDIVI_FLAT_W_QUAD_PROB
    size_type delta = 0u;
  #endif
    do {
      const uint8_t* group = &mGroups.data[index];
      int avails = MetaWGroup::match_available(group);
      if (avails) // not full
      {
        int idx = first_bit_index(avails);
        size_type valIdx = (index + idx) & mGMask;
        item_type* pValue = &mValues.data[valIdx];
        construct_item(pValue, std::forward<U>(value));
        ++mSize;

        bool wasTombstone = MetaWGroup::set_hfrag(mGroups.data, hash, valIdx, mGMask);
        mMaxSize += wasTombstone;
        return { pValue, valIdx };
      }
    #ifdef INDIVI_FLAT_W_QUAD_PROB
      index = (index + (++delta)*16) & mGMask;
    #else
      index += prob_delta(hash);
      index &= mGMask;
    #endif
    }
    while (true);
  }

  template< typename U, class... Args >
  Location unchecked_emplace(std::size_t hash, size_type index, U&& key, Args&&... args)
  {
  #ifdef INDIVI_FLAT_W_QUAD_PROB
    size_type delta = 0u;
  #endif
    do {
      uint8_t* group = &mGroups.data[index];
      int avails = MetaWGroup::match_available(group);
      if (avails) // not full
      {
        int idx = first_bit_index(avails);
        size_type valIdx = (index + idx) & mGMask;
        item_type* pValue = &mValues.data[valIdx];
        construct_item(pValue, std::piecewise_construct,
                               std::forward_as_tuple(std::forward<U>(key)),
                               std::forward_as_tuple(std::forward<Args>(args)...));
        ++mSize;

        bool wasTombstone = MetaWGroup::set_hfrag(mGroups.data, hash, valIdx, mGMask);
        mMaxSize += wasTombstone;
        return { pValue, valIdx };
      }
    #ifdef INDIVI_FLAT_W_QUAD_PROB
      index = (index + (++delta)*16) & mGMask;
    #else
      index += prob_delta(hash);
      index &= mGMask;
    #endif
    }
    while (true);
  }

  std::pair<iterator, bool> insert_impl(const init_type& value) { return try_insert_impl(value); }

  std::pair<iterator, bool> insert_impl(init_type&& value) { return try_insert_impl(std::move(value)); }

  template< typename = void > // resolve ambiguities with init_type
  std::pair<iterator, bool> insert_impl(const value_type& value) { return try_insert_impl(value); }

  template< typename = void >
  std::pair<iterator, bool> insert_impl(value_type&& value) { return try_insert_impl(std::move(value)); }

  template< typename U >
  std::pair<iterator, bool> try_insert_impl(U&& value)
  {
    std::size_t hash = get_hash(get_key(value));
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, get_key(value));
    if (loc.value != nullptr) // already exist
      return {  as_iter(loc), false };

    if (mSize < mMaxSize)
    {
      loc = unchecked_insert(hash, gIndex, std::forward<U>(value));
      return { as_iter(loc), true };
    }
    else // need rehash
    {
      if (mMaxSize > 0u || mGMask == 0u)
      {
        loc = grow_with_insert(hash, std::forward<U>(value));
        return { as_iter(loc), true };
      }
      else // only tombstones
      {
        INDIVI_WTABLE_ASSERT(mSize == 0u);
        clear();
        loc = unchecked_insert(hash, gIndex, std::forward<U>(value));
        return { as_iter(loc), true };
      }
    }
  }

  template< typename U, class... Args >
  std::pair<iterator, bool> try_emplace_impl(U&& key, Args&&... args)
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    if (loc.value != nullptr) // already exist
      return { as_iter(loc), false };

    if (mSize < mMaxSize)
    {
      loc = unchecked_emplace(hash, gIndex, std::forward<U>(key), std::forward<Args>(args)...);
      return { as_iter(loc), true };
    }
    else // need rehash
    {
      if (mMaxSize > 0u || mGMask == 0u)
      {
        loc = grow_with_emplace(hash, std::forward<U>(key), std::forward<Args>(args)...);
        return { as_iter(loc), true };
      }
      else // only tombstones
      {
        INDIVI_WTABLE_ASSERT(mSize == 0u);
        clear();
        loc = unchecked_emplace(hash, gIndex, std::forward<U>(key), std::forward<Args>(args)...);
        return { as_iter(loc), true };
      }
    }
  }

  template< typename U >
  mapped_type& try_emplace_key_impl(U&& key)
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    if (loc.value != nullptr) // already exist
      return loc.value->second;

    if (mSize < mMaxSize)
    {
      loc = unchecked_emplace(hash, gIndex, std::forward<U>(key));
      return loc.value->second;
    }
    else // need rehash
    {
      if (mMaxSize > 0u || mGMask == 0u)
      {
        loc = grow_with_emplace(hash, std::forward<U>(key));
        return loc.value->second;
      }
      else // only tombstones
      {
        INDIVI_WTABLE_ASSERT(mSize == 0u);
        clear();
        loc = unchecked_emplace(hash, gIndex, std::forward<U>(key));
        return loc.value->second;
      }
    }
  }

  template< typename U, class M >
  std::pair<iterator, bool> insert_or_assign_impl(U&& key, M&& obj)
  {
    std::size_t hash = get_hash(key);
    size_type gIndex = hash_position(hash, mShift);

    Location loc = find_impl(hash, gIndex, key);
    if (loc.value != nullptr) // already exist
    {
      loc.value->second = std::forward<M>(obj);
      return { as_iter(loc), false };
    }

    if (mSize < mMaxSize)
    {
      loc = unchecked_emplace(hash, gIndex, std::forward<U>(key), std::forward<M>(obj));
      return { as_iter(loc), true };
    }
    else // need rehash
    {
      if (mMaxSize > 0u || mGMask == 0u)
      {
        loc = grow_with_emplace(hash, std::forward<U>(key), std::forward<M>(obj));
        return { as_iter(loc), true };
      }
      else // only tombstones
      {
        INDIVI_WTABLE_ASSERT(mSize == 0u);
        clear();
        loc = unchecked_emplace(hash, gIndex, std::forward<U>(key), std::forward<M>(obj));
        return { as_iter(loc), true };
      }
    }
  }

  template< typename U >
  void insert_unique(uint8_t* groups, item_type* values, size_type shift, size_type gMask, U&& value)
  {
    std::size_t hash = get_hash(get_key(value));
    size_type index = hash_position(hash, shift);

  #ifdef INDIVI_FLAT_W_QUAD_PROB
    size_type delta = 0u;
  #endif
    do {
      uint8_t* group = &groups[index];
      int avails = MetaWGroup::match_available(group);
      if (avails) // not full
      {
        int idx = first_bit_index(avails);
        size_type realIdx = (index + idx) & gMask;
        item_type* pValue = &values[realIdx];
        construct_item(pValue, std::forward<U>(value));

        MetaWGroup::set_hfrag(groups, hash, realIdx, gMask);
        return;
      }
    #ifdef INDIVI_FLAT_W_QUAD_PROB
      index = (index + (++delta)*16) & gMask;
    #else
      index += prob_delta(hash);
      index &= gMask;
    #endif
    }
    while (true);
  }

  template< typename U >
  void insert_unique(U&& value)
  {
    insert_unique(mGroups.data, mValues.data, mShift, mGMask, std::forward<U>(value));
  }

  template< typename U >
  Location insert_first(uint8_t* groups, item_type* values, size_type shift, size_type gMask, std::size_t hash, U&& value)
  {
    size_type index = hash_position(hash, shift);

    item_type* pValue = &values[index];
    construct_item(pValue, std::forward<U>(value));

    MetaWGroup::set_hfrag(groups, hash, index, gMask);
    return { pValue, index };
  }

  template< typename U, class... Args >
  Location emplace_first(uint8_t* groups, item_type* values, size_type shift, size_type gMask, std::size_t hash, U&& key, Args&&... args)
  {
    size_type index = hash_position(hash, shift);

    item_type* pValue = &values[index];
    construct_item(pValue, std::piecewise_construct,
                           std::forward_as_tuple(std::forward<U>(key)),
                           std::forward_as_tuple(std::forward<Args>(args)...));

    MetaWGroup::set_hfrag(groups, hash, index, gMask);
    return { pValue, index };
  }

  void erase_impl(const Location& loc)
  {
    bool addedTombstone = MetaWGroup::reset_hfrag(mGroups.data, loc.index, mGMask);
    destroy_item(loc.value);
    INDIVI_WTABLE_ASSERT(mSize);
    INDIVI_WTABLE_ASSERT(mMaxSize);
    --mSize;
    mMaxSize -= addedTombstone; // anti-drift: reduce max size for each tombstone
  }

  void fast_copy(const flat_wtable& other)
  {
    INDIVI_WTABLE_ASSERT(empty());
    INDIVI_WTABLE_ASSERT(!other.empty());
    INDIVI_WTABLE_ASSERT(mValues.data);
    INDIVI_WTABLE_ASSERT(mShift == other.mShift);
    INDIVI_WTABLE_ASSERT(mGMask == other.mGMask);
    INDIVI_WTABLE_ASSERT(bucket_count() == other.bucket_count());
    size_type bucketCount = bucket_count();

    if (std::is_trivially_copy_constructible<item_type>::value)
    {
      std::memcpy((void*)mValues.data, other.mValues.data, sizeof(item_type) * bucketCount);
      std::memcpy((void*)mGroups.data, other.mGroups.data, sizeof(uint8_t) * (mGMask + 1u + 16u)); // extra group
      INDIVI_WTABLE_ASSERT(mGroups.data[mGMask + 1u + 15u] == 0); // sentinel
      mSize = other.mSize;
    }
    else
    {
      size_type ctr_count = 0u;
      try
      {
        other.uc_for_each([&](const item_type* otValue) {
          construct_item(mValues.data + (otValue - other.mValues.data), *otValue);
          ++ctr_count;
        });
      }
      catch (...)
      {
        if (ctr_count)
        {
          other.uc_each_while([&](const item_type* otValue) {
            item_type* pValue = mValues.data + (otValue - other.mValues.data);
            destroy_item(pValue);
            return --ctr_count;
          });
        }
        throw;
      }
      INDIVI_WTABLE_ASSERT(ctr_count == other.mSize);

      std::memcpy(mGroups.data, other.mGroups.data, sizeof(uint8_t) * (mGMask + 1u + 16u)); // extra group
      INDIVI_WTABLE_ASSERT(mGroups.data[mGMask + 1u + 15u] == 0); // sentinel
      mSize = other.mSize;
    }
  }

  void copy_content(const flat_wtable& other)
  {
    INDIVI_WTABLE_ASSERT(empty());
    if (other.empty())
      return;

    reserve(other.mSize);
    try
    {
      if (mMaxSize == other.mMaxSize) // same bucket count
      {
        fast_copy(other);
      }
      else
      {
        other.uc_for_each([&](const item_type* pValue) {
          insert_unique(*pValue);
          ++mSize;
        });
      }
    }
    catch (...)
    {
      destroy();
      throw;
    }
  }

  void move_construct_alloc(flat_wtable&& other)
  {
    if (get_allocator() == other.get_allocator())
    {
      mSize = other.mSize;
      mShift = other.mShift;
      mGMask = other.mGMask;
      mMaxSize = other.mMaxSize;
      mGroups.data = other.mGroups.data;
      mValues.data = other.mValues.data;

      other.mSize = 0u;
      other.mShift = EMPTY_SHIFT;
      other.mGMask = 0u;
      other.mMaxSize = 0u;
      other.mGroups.data = MetaWGroup::empty_group();
      other.mValues.data = nullptr;
    }
    else
    {
      if (!other.empty())
      {
        reserve(other.mSize);
        try
        {
          other.uc_for_each([&](item_type* pValue) {
            insert_unique(std::move(*pValue));
            ++mSize;
          });
          other.clear();
        }
        catch (...)
        {
          destroy();
          throw;
        }
      }
    }
  }

  void move_to(uint8_t* newGroups, item_type* newValues, size_type newShift, size_type newGMask)
  {
    item_type* last_destroyed = nullptr;
    try
    {
      uc_for_each([&](item_type* pValue) {
        insert_unique(newGroups, newValues, newShift, newGMask, std::move(*pValue));
        last_destroyed = pValue;
        destroy_item(pValue);
      });
    }
    catch (...)
    {
      // clear old table metadata for items that were already successfully moved and destroyed
      if (last_destroyed)
      {
        uc_each_while([&](item_type* pValue) {
          if (pValue <= last_destroyed)
          {
            size_type realIdx = static_cast<size_type>(pValue - mValues.data);
            MetaWGroup::reset_hfrag(mGroups.data, realIdx, mGMask);
            return true;
          }
          return false;
        });
      }

      // destroy elements successfully constructed in the new storage block
      item_type* pValue = newValues;
      uint8_t* pGroup = newGroups;
      uint8_t* last = pGroup + newGMask + 1;
      unsigned int setsMask = gmask_to_setsmask(newGMask);
      for (; pGroup != last; pGroup += 16, pValue += 16)
      {
        int idx = 0;
        int sets = MetaWGroup::match_set(pGroup) & setsMask;
        while (sets)
        {
          if (sets & 0x01)
            destroy_item(&pValue[idx]);

          sets >>= 1;
          ++idx;
        }
      }
      throw;
    }
  }

  struct NewStorage // exception-safe helper
  {
    size_type itemsCapa;
    size_type totalStorageCapa;
    storage_type* data;
    storage_allocator_type& alloc;

    NewStorage(size_type itemsCapa_, size_type groupsCapa_, storage_allocator_type& alloc_)
      : itemsCapa(itemsCapa_)
      , totalStorageCapa(storageCapa(itemsCapa_, groupsCapa_))
      , data(storage_alloc_traits::allocate(alloc_, totalStorageCapa))
      , alloc(alloc_)
    {
      std::size_t padding = (groupsCapa_ > 32u) ? 15u : 0u; // padding for iteration stop on sentinel group
      void* ptr = (void*)(data + itemsCapa_);
      std::memset(ptr, MetaWGroup::EMPTY_FRAG, sizeof(uint8_t) * groupsCapa_ + padding); // init MetaWGroups manually
      ((uint8_t*)ptr)[sizeof(uint8_t) * (groupsCapa_ - 1)] = MetaWGroup::SENTINEL_FRAG; // fake entry to force stop iteration
    }

    ~NewStorage()
    {
      if (data)
        storage_alloc_traits::deallocate(alloc, data, totalStorageCapa);
    }

    static size_type storageCapa(size_type itemsCapa, size_type groupsCapa)
    {
      std::size_t padding = (groupsCapa > 32u) ? 15u : 0u;
      size_type grpsAsItemCapa = sizeof(uint8_t) * groupsCapa + padding;
      grpsAsItemCapa = (grpsAsItemCapa + sizeof(item_type) - 1) / sizeof(item_type); // round-up
      return itemsCapa + grpsAsItemCapa; // storage uses item_type element size
    }

    void release() noexcept { data = nullptr; }

    item_type* values() const noexcept { return reinterpret_cast<item_type*>(data); }
    uint8_t* groups() const noexcept { return reinterpret_cast<uint8_t*>(data + itemsCapa); }
  };

  void rehash_impl(size_type newCapa)
  {
    INDIVI_WTABLE_ASSERT(newCapa >= MIN_CAPA);
    INDIVI_WTABLE_ASSERT(is_pow2(newCapa));

    size_type newGCapa = newCapa + 16u; // with extra group (first group duplicate)
    size_type newShift = hash_shift(newCapa);
    size_type newGMask = newCapa - 1u;

    if (mValues.data)
    {
      NewStorage newStorage(newCapa, newGCapa, storage_alloc());
      uint8_t* newGroups = newStorage.groups();
      item_type* newValues = newStorage.values();

      // move existing
      move_to(newGroups, newValues, newShift, newGMask);

      // delete old and update
      storage_alloc_traits::deallocate(storage_alloc(), reinterpret_cast<storage_type*>(mValues.data),
                                       NewStorage::storageCapa(mGMask + 1u, mGMask + 1u + 16u));
      mGroups.data = newGroups;
      mValues.data = newValues;
      newStorage.release();
    }
    else // first time
    {
      INDIVI_WTABLE_ASSERT(mSize == 0u);
      INDIVI_WTABLE_ASSERT(mShift == EMPTY_SHIFT);
      INDIVI_WTABLE_ASSERT(mGMask == 0u);

      NewStorage newStorage(newCapa, newGCapa, storage_alloc());
      mGroups.data = newStorage.groups();
      mValues.data = newStorage.values();
      newStorage.release();
    }

    mShift = newShift;
    mGMask = newGMask;
    mMaxSize = capa_to_maxsize(newCapa);
  }

  template< typename U >
  Location grow_with_insert(std::size_t hash, U&& value)
  {
    size_type newCapa = bucket_count() * 2;
    newCapa = std::max(newCapa, (size_type)MIN_CAPA);
    INDIVI_WTABLE_ASSERT(is_pow2(newCapa));

    size_type newGCapa = newCapa + 16u; // with extra group (first group duplicate)
    size_type newShift = hash_shift(newCapa);
    size_type newGMask = newCapa - 1u;

    NewStorage newStorage(newCapa, newGCapa, storage_alloc());
    uint8_t* newGroups = newStorage.groups();
    item_type* newValues = newStorage.values();

    // insert new value first (strong exception guarantee)
    Location loc = insert_first(newGroups, newValues, newShift, newGMask, hash, std::forward<U>(value));

    if (mValues.data)
    {
      // move existing
      move_to(newGroups, newValues, newShift, newGMask);

      // delete old
      storage_alloc_traits::deallocate(storage_alloc(), reinterpret_cast<storage_type*>(mValues.data),
                                       NewStorage::storageCapa(mGMask + 1u, mGMask + 1u + 16u));
    }
    // update
    mGroups.data = newGroups;
    mValues.data = newValues;
    newStorage.release();

    mShift = newShift;
    mGMask = newGMask;
    mMaxSize = capa_to_maxsize(newCapa);
    ++mSize;

    return loc;
  }

  template< typename U, class... Args >
  Location grow_with_emplace(std::size_t hash, U&& key, Args&&... args)
  {
    size_type newCapa = bucket_count() * 2;
    newCapa = std::max(newCapa, (size_type)MIN_CAPA);
    INDIVI_WTABLE_ASSERT(is_pow2(newCapa));

    size_type newGCapa = newCapa + 16u; // with extra group (first group duplicate)
    size_type newShift = hash_shift(newCapa);
    size_type newGMask = newCapa - 1u;

    NewStorage newStorage(newCapa, newGCapa, storage_alloc());
    uint8_t* newGroups = newStorage.groups();
    item_type* newValues = newStorage.values();

    // insert new value first (strong exception guarantee)
    Location loc = emplace_first(newGroups, newValues, newShift, newGMask, hash, std::forward<U>(key), std::forward<Args>(args)...);

    if (mValues.data)
    {
      // move existing
      move_to(newGroups, newValues, newShift, newGMask);

      // delete old
      storage_alloc_traits::deallocate(storage_alloc(), reinterpret_cast<storage_type*>(mValues.data),
                                       NewStorage::storageCapa(mGMask + 1u, mGMask + 1u + 16u));
    }
    // update
    mGroups.data = newGroups;
    mValues.data = newValues;
    newStorage.release();

    mShift = newShift;
    mGMask = newGMask;
    mMaxSize = capa_to_maxsize(newCapa);
    ++mSize;

    return loc;
  }
};

} // namespace detail
} // namespace indivi

#endif // INDIVI_FLAT_WTABLE_H
