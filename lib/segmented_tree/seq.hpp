// (C) Copyright Chris Clearwater 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SEGMENTED_TREE_SEQ
#define BOOST_SEGMENTED_TREE_SEQ

#include "seq_fwd.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

#ifdef BOOST_SEGMENTED_TREE_DEBUG
#include <iostream>
#endif

namespace boost {
namespace segmented_tree {

#ifndef BOOST_SEGMENTED_TREE_DOXYGEN_INVOKED
namespace detail {
namespace is_nothrow_swappable_impl {
using std::swap;

template <typename T>
struct test {
  static bool const value =
      noexcept(swap(std::declval<T&>(), std::declval<T&>()));
};
}

template <typename T>
struct is_nothrow_swappable
    : std::integral_constant<bool, is_nothrow_swappable_impl::test<T>::value> {
};

template <typename T, typename Alloc>
struct is_alloc_move_construct_default {
 private:
  template <typename U>
  static auto test(U alloc)
      -> decltype(alloc.construct(std::declval<T*>(), std::declval<T&&>()),
                  std::false_type{});
  template <typename>
  static std::true_type test(...);

 public:
  static bool constexpr value =
      decltype(test<Alloc>(std::declval<Alloc>()))::value;
};

template <typename T, typename VoidPointer, typename SizeType,
          std::size_t segment_target, std::size_t base_target>
struct static_traits_t {
  // forward declarations
  struct node_base;
  struct node_data;
  struct node;
  struct segment_entry;
  struct leaf_entry;
  struct iterator_entry;
  struct iterator_data;

  using value_type = T;
  using void_pointer = VoidPointer;
  using size_type = SizeType;
  using node_pointer =
      typename std::pointer_traits<VoidPointer>::template rebind<node>;
  using element_pointer =
      typename std::pointer_traits<VoidPointer>::template rebind<value_type>;
  using difference_type =
      typename std::pointer_traits<VoidPointer>::difference_type;

  // sized types
  struct node_base {
    node_pointer parent_pointer;
    std::uint16_t parent_index;
    std::uint16_t length;
  };

  struct node_data {
    void_pointer pointer;
    size_type sz;
  };

  // constexpr
  static constexpr std::size_t segment_free() { return segment_target; }

  static constexpr std::size_t node_size() { return sizeof(node_base); }

  static constexpr std::size_t base_free() {
    return node_size() > base_target ? 0 : base_target - node_size();
  }

  static constexpr std::size_t segment_fit() {
    return segment_free() / sizeof(T);
  }

  static constexpr std::size_t base_fit() {
    return base_free() / sizeof(node_data);
  }

  static constexpr std::size_t segment_max() {
    return segment_fit() > 1 ? segment_fit() : 1;
  }

  static constexpr std::size_t base_max() {
    return base_fit() > 3 ? base_fit() : 3;
  }

  static constexpr std::size_t segment_min() { return (segment_max() + 1) / 2; }

  static constexpr std::size_t base_min() { return (base_max() + 1) / 2; }

  // types
  struct node {
    node_pointer parent_pointer;
    std::uint16_t parent_index_;
    std::uint16_t length_;
    std::array<size_type, base_max()> sizes;
    std::array<void_pointer, base_max()> pointers;

    size_type parent_index() { return parent_index_; }

    void parent_index(size_type index) {
      parent_index_ = static_cast<std::uint16_t>(index);
    }

    size_type length() { return length_; }

    void length(size_type length) {
      length_ = static_cast<std::uint16_t>(length);
    }
  };

  struct segment_entry {
    element_pointer pointer;
    size_type index;
    size_type length;
  };

  struct leaf_entry {
    node_pointer pointer;
    size_type index;
  };

  struct iterator_entry {
    segment_entry segment;
    leaf_entry leaf;
  };

  struct iterator_data {
    iterator_entry entry;
    size_type pos;
  };

  // cast
  static element_pointer cast_segment(void_pointer pointer) {
    return static_cast<element_pointer>(pointer);
  }

  static node_pointer cast_node(void_pointer pointer) {
    return static_cast<node_pointer>(pointer);
  }

  // find_index
  static iterator_data find_index_root(void_pointer pointer, size_type sz,
                                       size_type ht, size_type pos) {
    iterator_data it;
    it.pos = pos;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_index_segment(cast_segment(pointer), sz, pos);
    } else
      it.entry = find_index_node(cast_node(pointer), ht, pos);

    return it;
  }

  static iterator_entry find_index_node(node_pointer pointer, size_type ht,
                                        size_type pos) {
    if (ht == 2) return find_index_leaf(pointer, pos);
    return find_index_branch(pointer, ht, pos);
  }

  static iterator_entry find_index_branch(node_pointer pointer, size_type ht,
                                          size_type pos) {
    while (true) {
      size_type index = 0;
      auto size = pointer->sizes[0];
      while (pos >= size) {
        pos -= size;
        ++index;
        size = pointer->sizes[index];
      }

      auto child = cast_node(pointer->pointers[index]);
      --ht;
      if (ht == 2)
        return find_index_leaf(child, pos);

      pointer = child;
    }
  }

  static iterator_entry find_index_leaf(node_pointer pointer, size_type pos) {
    size_type index = 0;
    auto size = pointer->sizes[0];
    while (pos >= size) {
      pos -= size;
      ++index;
      size = pointer->sizes[index];
    }

    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = index;
    entry.segment = find_index_segment(cast_segment(pointer->pointers[index]),
                                       pointer->sizes[index], pos);
    return entry;
  }

  static segment_entry find_index_segment(element_pointer pointer, size_type sz,
                                          size_type pos) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = pos;
    entry.length = sz;
    return entry;
  }

  // find_first
  static iterator_data find_first_root(void_pointer pointer, size_type sz,
                                       size_type ht) {
    iterator_data it;
    it.pos = 0;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_first_segment(cast_segment(pointer), sz);
    } else
      it.entry = find_first_node(cast_node(pointer), ht);

    return it;
  }

  static iterator_entry find_first_node(node_pointer pointer, size_type ht) {
    if (ht == 2) return find_first_leaf(pointer);
    return find_first_branch(pointer, ht);
  }

  static iterator_entry find_first_branch(node_pointer pointer, size_type ht) {
    while (true) {
      auto child = cast_node(pointer->pointers[0]);
      --ht;
      if (ht == 2) return find_first_leaf(child);

      pointer = child;
    }
  }

  static iterator_entry find_first_leaf(node_pointer pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = 0;
    entry.segment = find_first_segment(cast_segment(pointer->pointers[0]),
                                       pointer->sizes[0]);
    return entry;
  }

  static segment_entry find_first_segment(element_pointer pointer,
                                          size_type sz) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = 0;
    entry.length = sz;
    return entry;
  }

  // find_last
  static iterator_data find_last_root(void_pointer pointer, size_type sz,
                                      size_type ht) {
    iterator_data it;
    it.pos = sz - 1;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_last_segment(cast_segment(pointer), sz);
    } else
      it.entry = find_last_node(cast_node(pointer), ht);

    return it;
  }

  static iterator_entry find_last_node(node_pointer pointer, size_type ht) {
    if (ht == 2) return find_last_leaf(pointer);
    return find_last_branch(pointer, ht);
  }

  static iterator_entry find_last_branch(node_pointer pointer, size_type ht) {
    while (true) {
      auto index = pointer->length() - 1;
      auto child = cast_node(pointer->pointers[index]);
      --ht;
      if (ht == 2) return find_last_leaf(child);

      pointer = child;
    }
  }

  static iterator_entry find_last_leaf(node_pointer pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = pointer->length() - 1;
    entry.segment =
        find_last_segment(cast_segment(pointer->pointers[entry.leaf.index]),
                          pointer->sizes[entry.leaf.index]);
    return entry;
  }

  static segment_entry find_last_segment(element_pointer pointer,
                                         size_type sz) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = sz - 1;
    entry.length = sz;
    return entry;
  }

  // find_end
  static iterator_data find_end_root(void_pointer pointer, size_type sz,
                                     size_type ht) {
    iterator_data it;
    it.pos = sz;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_end_segment(cast_segment(pointer), sz);
    } else
      it.entry = find_end_node(cast_node(pointer), ht);

    return it;
  }

  static iterator_entry find_end_node(node_pointer pointer, size_type ht) {
    if (ht == 2) return find_end_leaf(pointer);
    return find_end_branch(pointer, ht);
  }

  static iterator_entry find_end_branch(node_pointer pointer, size_type ht) {
    while (true) {
      auto index = pointer->length() - 1;
      auto child = cast_node(pointer->pointers[index]);

      --ht;
      if (ht == 2) return find_end_leaf(child);

      pointer = child;
    }
  }

  static iterator_entry find_end_leaf(node_pointer pointer) {
    iterator_entry entry;
    auto index = pointer->length() - 1;
    entry.leaf.pointer = pointer;
    entry.leaf.index = index;
    entry.segment = find_end_segment(cast_segment(pointer->pointers[index]),
                                     pointer->sizes[index]);
    return entry;
  }

  static segment_entry find_end_segment(element_pointer pointer, size_type sz) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = sz;
    entry.length = sz;
    return entry;
  }

  // move_next
  static void move_next_iterator(iterator_data& it) {
    ++it.pos;
    move_next_segment(it.entry);
  }

  static void move_next_segment(iterator_entry& entry) {
    auto index = entry.segment.index;
    auto length = entry.segment.length;

    ++index;
    if (index != length) {
      entry.segment.index = index;
      return;
    }

    move_next_leaf(entry);
  }

  static void move_next_leaf(iterator_entry& entry) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    // Special case for end iterator.
    if (pointer == nullptr) {
      entry.segment.index = entry.segment.length;
      return;
    }

    ++index;
    if (index != pointer->length()) {
      entry.leaf.index = index;
      entry.segment = find_first_segment(cast_segment(pointer->pointers[index]),
                                         pointer->sizes[index]);
      return;
    }

    move_next_branch(entry, pointer->parent_pointer, pointer->parent_index());
  }

  static void move_next_branch(iterator_entry& entry, node_pointer pointer,
                               size_type index) {
    size_type child_ht = 2;

    while (true) {
      // Special case for end iterator.
      if (pointer == nullptr) {
        entry.segment.index = entry.segment.length;
        return;
      }

      ++index;
      if (index != pointer->length()) {
        entry = find_first_node(cast_node(pointer->pointers[index]), child_ht);
        return;
      }

      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  // move_prev
  static void move_prev_iterator(iterator_data& it) {
    --it.pos;
    move_prev_segment(it.entry);
  }

  static void move_prev_segment(iterator_entry& entry) {
    auto index = entry.segment.index;

    if (index != 0) {
      --index;
      entry.segment.index = index;
      return;
    }

    move_prev_leaf(entry);
  }

  static void move_prev_leaf(iterator_entry& entry) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    if (index != 0) {
      --index;
      entry.leaf.index = index;
      entry.segment = find_last_segment(cast_segment(pointer->pointers[index]),
                                        pointer->sizes[index]);
      return;
    }

    move_prev_branch(entry, pointer->parent_pointer, pointer->parent_index());
  }

  static void move_prev_branch(iterator_entry& entry, node_pointer pointer,
                               size_type index) {
    size_type child_ht = 2;

    while (true) {
      if (index != 0) {
        entry =
            find_last_node(cast_node(pointer->pointers[index - 1]), child_ht);
        return;
      }

      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  // move_count
  static void move_iterator_count(iterator_data& it, difference_type diff) {
    auto size = static_cast<size_type>(diff);
    it.pos += size;
    if (diff > 0)
      move_next_segment_count(it.entry, size);
    else if (diff < 0)
      move_prev_segment_count(it.entry, ~size + 1);
  }

  // move_next_count
  static void move_next_iterator_count(iterator_data& it, size_type count) {
    it.pos += count;
    move_next_segment_count(it.entry, count);
  }

  static void move_next_segment_count(iterator_entry& entry, size_type count) {
    auto index = entry.segment.index;
    auto length = entry.segment.length;

    index += count;
    if (index < length) {
      entry.segment.index = index;
      return;
    }

    move_next_leaf_count(entry, index - length);
  }

  static void move_next_leaf_count(iterator_entry& entry, size_type count) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    // Special case for end iterator.
    if (pointer == nullptr) {
      entry.segment.index = entry.segment.length;
      return;
    }

    while (true) {
      ++index;
      if (index == pointer->length()) break;

      auto size = pointer->sizes[index];
      if (size > count) {
        entry.leaf.index = index;
        entry.segment = find_index_segment(
            cast_segment(pointer->pointers[index]), size, count);
        return;
      }
      count -= size;
    }

    move_next_branch_count(entry, pointer, pointer->parent_pointer,
                           pointer->parent_index(), count);
  }

  static void move_next_branch_count(iterator_entry& entry, node_pointer base,
                                     node_pointer pointer, size_type index,
                                     size_type count) {
    size_type child_ht = 2;

    while (true) {
      // Special case for end iterator.
      if (pointer == nullptr) {
        entry = find_end_node(base, child_ht);
        return;
      }

      while (true) {
        ++index;
        if (index == pointer->length()) break;

        auto size = pointer->sizes[index];
        if (size > count) {
          entry = find_index_node(cast_node(pointer->pointers[index]), child_ht,
                                  count);
          return;
        }
        count -= size;
      }

      base = pointer;
      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  // move_prev_count
  static void move_prev_iterator_count(iterator_data& it, size_type count) {
    it.pos -= count;
    move_prev_segment_count(it.entry, count);
  }

  static void move_prev_segment_count(iterator_entry& entry, size_type count) {
    auto index = entry.segment.index;

    if (index >= count) {
      index -= count;
      entry.segment.index = index;
      return;
    }

    move_prev_leaf_count(entry, count - index);
  }

  static void move_prev_leaf_count(iterator_entry& entry, size_type count) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    while (true) {
      if (index == 0) break;
      --index;

      auto size = pointer->sizes[index];
      if (size >= count) {
        entry.leaf.index = index;
        entry.segment = find_index_segment(
            cast_segment(pointer->pointers[index]), size, size - count);
        return;
      }
      count -= size;
    }

    move_prev_branch_count(entry, pointer->parent_pointer,
                           pointer->parent_index(), count);
  }

  static void move_prev_branch_count(iterator_entry& entry,
                                     node_pointer pointer, size_type index,
                                     size_type count) {
    size_type child_ht = 2;

    while (true) {
      while (true) {
        if (index == 0) break;
        --index;

        auto size = pointer->sizes[index];
        if (size >= count) {
          entry = find_index_node(cast_node(pointer->pointers[index]), child_ht,
                                  size - count);
          return;
        }
        count -= size;
      }

      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  static element_pointer first_element(iterator_data it) {
    return it.entry.segment.pointer;
  }

  static element_pointer current_element(iterator_data it) {
    return it.entry.segment.pointer + it.entry.segment.index;
  }

  static element_pointer last_element(iterator_data it) {
    return it.entry.segment.pointer + it.entry.segment.length;
  }

  static value_type& dereference(iterator_data it) {
    return it.entry.segment.pointer[it.entry.segment.index];
  }

  static value_type& dereference_count(iterator_data it, difference_type diff) {
    move_iterator_count(it, diff);
    return it.entry.segment.pointer[it.entry.segment.index];
  }

  static difference_type difference(iterator_data a, iterator_data b) {
    return a.pos > b.pos ? static_cast<difference_type>(a.pos - b.pos)
                         : -static_cast<difference_type>(b.pos - a.pos);
  }

  static void move_after_segment(iterator_data& it) {
    it.pos += it.entry.segment.length - it.entry.segment.index;
    move_next_leaf(it.entry);
  }

  static void move_after_segment_count(iterator_data& it, size_type count) {
    it.pos += it.entry.segment.length - it.entry.segment.index + count;
    move_next_leaf_count(it.entry, count);
  }

  static void move_before_segment(iterator_data& it) {
    it.pos -= it.entry.segment.index + 1;
    move_prev_leaf(it.entry);
  }

  static void move_before_segment_count(iterator_data& it, size_type count) {
    it.pos -= it.entry.segment.index + 1 - count;
    move_prev_leaf_count(it.entry, count);
  }
};
}
#endif  // #ifndef BOOST_SEGMENTED_TREE_DOXYGEN_INVOKED

/// A template class used for const and non-const iterators for
/// seq.
///
/// \tparam Pointer A const or non-const pointer.
/// \tparam Reference A const or non-const reference.
template <typename StaticTraits, typename Pointer, typename Reference>
class iterator_t {
  template <typename, typename, std::size_t, std::size_t>
  friend class boost::segmented_tree::seq;

 private:
  template <typename, typename, typename>
  friend class iterator_t;
  using static_traits = StaticTraits;
  using iterator_data = typename static_traits::iterator_data;
  iterator_data it_;
  iterator_t(iterator_data it) : it_(it) {}

 public:
  /// The iterator category type.
  using iterator_category = std::random_access_iterator_tag;
  /// The value type.
  using value_type = typename static_traits::value_type;
  /// The size type.
  using size_type = typename static_traits::size_type;
  /// The difference type.
  using difference_type = typename static_traits::difference_type;
  /// The pointer type.
  using pointer = Pointer;
  /// The iterator reference type.
  using reference = Reference;

  /// \par Effects
  ///   Default constructs an iterator from another.
  ///
  /// \par Complexity
  ///   Constant.
  iterator_t() = default;

  /// \par Effects
  ///   Copy constructs an iterator from other.
  ///
  /// \par Complexity
  ///   Constant.
  iterator_t(iterator_t const& other) = default;

  /// \par Effects
  ///   Copy assigns from other.
  ///
  /// \par Complexity
  ///   Constant.
  iterator_t& operator=(iterator_t const& other) = default;

  /// \par Effects
  ///   Copy constructs a const iterator from non-const iterator other.
  ///
  /// \par Complexity
  ///   Constant.
  template <typename P, typename R,
            typename = typename std::enable_if<
                std::is_convertible<P, pointer>::value>::type>
  iterator_t(iterator_t<static_traits, P, R> const& other) : it_(other.it_) {}

  /// \par Returns
  ///   A pointer to the current element of the current segment of the iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Note
  ///   Non-standard extension.
  pointer current() const { return static_traits::current_element(it_); }

  /// \par Returns
  ///   A pointer to the first element of the current segment of the iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Note
  ///   Non-standard extension.
  pointer begin() const { return static_traits::first_element(it_); }

  /// \par Returns
  ///  A pointer 1 past the last element of the current segment of the iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Note
  ///   Non-standard extension.
  pointer end() const { return static_traits::last_element(it_); }

  /// \par Returns
  ///   A pointer to the current element.
  ///
  /// \par Complexity
  ///   Constant.
  pointer operator->() const { return static_traits::current_element(it_); }

  /// \par Returns
  ///   A reference to the current element.
  ///
  /// \par Complexity
  ///   Constant.
  reference operator*() const { return static_traits::dereference(it_); }

  /// \par Returns
  ///   A reference to the element diff positions away from the current
  ///   position.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in the absolute value of diff.
  reference operator[](difference_type diff) const {
    return static_traits::dereference_count(it_, diff);
  }

  /// \par Effects
  ///   Move the iterator forward 1 element.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Constant amortized.
  iterator_t& operator++() {
    static_traits::move_next_iterator(it_);
    return *this;
  }

  /// \par Effects
  ///   Move the iterator backward 1 element.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Constant amortized.
  iterator_t& operator--() {
    static_traits::move_prev_iterator(it_);
    return *this;
  }

  /// \par Returns
  ///   A copy of the iterator moved forward 1 element.
  ///
  /// \par Complexity
  ///   Constant amortized.
  iterator_t operator++(int) {
    auto copy = it_;
    static_traits::move_next_iterator(it_);
    return copy;
  }

  /// \par Returns
  ///   A copy of the iterator moved backward 1 element.
  ///
  /// \par Complexity
  ///   Constant amortized.
  iterator_t operator--(int) {
    auto copy = it_;
    static_traits::move_prev_iterator(it_);
    return copy;
  }

  /// \par Effects
  ///   Move the iterator forward diff elements.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in the absolute value of diff.
  iterator_t& operator+=(difference_type diff) {
    static_traits::move_iterator_count(it_, diff);
    return *this;
  }

  /// \par Effects
  ///   Move the iterator backward diff elements.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in the absolute value of diff.
  iterator_t& operator-=(difference_type diff) {
    static_traits::move_iterator_count(it_, -diff);
    return *this;
  }

  /// \par Returns
  ///   A copy of the iterator moved forward diff elements.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in the absolute value of diff.
  iterator_t operator+(difference_type diff) const {
    auto copy = it_;
    static_traits::move_iterator_count(copy, diff);
    return copy;
  }

  /// \par Returns
  ///   A copy of the iterator moved backward diff elements.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in the absolute value of diff.
  iterator_t operator-(difference_type diff) const {
    auto copy = it_;
    static_traits::move_iterator_count(copy, -diff);
    return copy;
  }

  /// \par Returns
  ///   A distance between the specified iterator.
  ///
  /// \par Complexity
  ///   Constant.
  difference_type operator-(iterator_t const& other) const {
    return static_traits::difference(it_, other.it_);
  }

  /// \par Effects
  ///   Move the iterator to the last element of the previous segment.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Constant amortized.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t& move_before_segment() {
    static_traits::move_before_segment(it_);
    return *this;
  }

  /// \par Effects
  ///   Move the iterator to the last element of the previous segment and then
  ///   move backward count elements.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in count.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t& move_before_segment(size_type count) {
    static_traits::move_before_segment_count(it_, count);
    return *this;
  }

  /// \par Effects
  ///   Move the iterator to the first element of the next segment.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Constant amortized.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t& move_after_segment() {
    static_traits::move_after_segment(it_);
    return *this;
  }

  /// \par Effects
  ///   Move the iterator to the first element of the next segment and then move
  ///   forward count elements.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in count.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t& move_after_segment(size_type count) {
    static_traits::move_after_segment_count(it_, count);
    return *this;
  }

  /// \par Returns
  ///   A copy of the iterator moved to the last element of the previous
  ///   segment.
  ///
  /// \par Complexity
  ///   Constant amortized.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t before_segment() const {
    auto copy = it_;
    static_traits::move_before_segment(copy);
    return copy;
  }

  /// \par Returns
  ///   A copy of the iterator moved to the last element of the previous segment
  ///   and then move backward count elements.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in count.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t after_segment() const {
    auto copy = it_;
    static_traits::move_after_segment(copy);
    return copy;
  }

  /// \par Returns
  ///   A copy of the iterator moved to the last element of the previous
  ///   segment.
  ///
  /// \par Complexity
  ///   Constant amortized.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t before_segment(size_type count) const {
    auto copy = it_;
    static_traits::move_before_segment_count(copy, count);
    return copy;
  }

  /// \par Returns
  ///   A copy of the iterator to the first element of the next segment and then
  ///   move forward count elements.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in count.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator_t after_segment(size_type count) const {
    auto copy = it_;
    static_traits::move_after_segment_count(copy, count);
    return copy;
  }

  /// \par Returns
  ///   False if both iterators point to the same element. True otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  bool operator!=(iterator_t const& other) const {
    return it_.pos != other.it_.pos;
  }

  /// \par Returns
  ///   True if both iterators point to the same element. False otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  bool operator==(iterator_t const& other) const {
    return it_.pos == other.it_.pos;
  }

  /// \par Returns
  ///   True if *this points to an element before other. False otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  bool operator<(iterator_t const& other) const {
    return it_.pos < other.it_.pos;
  }

  /// \par Returns
  ///   True if *this points to an element after other. False otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  bool operator>(iterator_t const& other) const {
    return it_.pos > other.it_.pos;
  }

  /// \par Returns
  ///   True if *this points to the same element or before other. False
  ///   otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  bool operator<=(iterator_t const& other) const {
    return it_.pos <= other.it_.pos;
  }

  /// \par Returns
  ///   True if *this points to the same element or after other. False
  ///   otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  bool operator>=(iterator_t const& other) const {
    return it_.pos >= other.it_.pos;
  }

  /// \par Returns
  ///   A copy of the iterator it moved forward diff elements.
  ///
  /// \par Complexity
  ///   Logarithmic amortized in the absolute value of diff.
  friend iterator_t operator+(difference_type diff, iterator_t it) {
    return it + diff;
  }
};

/// A seq is a sequence container that provides efficient random
/// access insert and erase.
///
/// \tparam T The type of the element to be stored
/// \tparam Allocator The type of the allocator used for all memory management
/// \tparam segment_target The size in bytes to try to use for element nodes
/// \tparam base_target The size in bytes to try to use for index nodes
template <typename T, typename Allocator, std::size_t segment_target,
          std::size_t base_target>
class seq {
 private:
  // alias
  using element_traits = typename std::allocator_traits<Allocator>;
  using static_traits =
      detail::static_traits_t<T, typename element_traits::void_pointer,
                              typename element_traits::size_type,
                              segment_target, base_target>;
  using element_pointer = typename static_traits::element_pointer;
  using void_pointer = typename static_traits::void_pointer;
  using node_pointer = typename static_traits::node_pointer;
  using node_type = typename static_traits::node;
  using iterator_data = typename static_traits::iterator_data;
  using iterator_entry = typename static_traits::iterator_entry;
  using leaf_entry = typename static_traits::leaf_entry;
  using node_allocator =
      typename element_traits::template rebind_alloc<node_type>;
  using node_traits =
      typename element_traits::template rebind_traits<node_type>;

 public:
  /// The type of elements stored in the container.
  using value_type = typename element_traits::value_type;
  /// The allocator type used by the container.
  using allocator_type = Allocator;
  /// The unsigned integral type used by the container.
  using size_type = typename element_traits::size_type;
  /// The pointers difference type.
  using difference_type = typename element_traits::difference_type;
  /// The value reference type.
  using reference = value_type&;
  /// The value const reference type.
  using const_reference = value_type const&;
  /// The pointer type.
  using pointer = typename element_traits::pointer;
  /// The const pointer type.
  using const_pointer = typename element_traits::const_pointer;
  /// The iterator type.
  using iterator = iterator_t<static_traits, pointer, T&>;
  /// The const iterator type.
  using const_iterator = iterator_t<static_traits, const_pointer, T const&>;
  /// The reverse iterator type.
  using reverse_iterator = std::reverse_iterator<iterator>;
  /// The const reverse iterator.
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  /// The stored allocator type.
  using stored_allocator_type = allocator_type;

//uint64_t iii1 = 0u;//TEMP
//uint64_t iii2 = 0u;//TEMP
//uint64_t iii3 = 0u;//TEMP
//uint64_t iii4 = 0u;//TEMP
//uint64_t iii5 = 0u;//TEMP
//uint64_t iii6 = 0u;//TEMP

 private:
#ifndef BOOST_SEGMENTED_TREE_DOXYGEN_INVOKED
  // private data
  void_pointer root_{nullptr};

  struct size_pair : allocator_type {
    size_type sz;
    size_pair() = default;
    size_pair(allocator_type const& alloc) : allocator_type{alloc}, sz{} {}
  } size_pair_{};

  struct height_pair : node_allocator {
    size_type ht;
    height_pair() = default;
    height_pair(allocator_type const& alloc) : node_allocator{alloc}, ht{} {}
  } height_pair_{};

  // getters
  void_pointer& get_root() { return root_; }
  void_pointer const& get_root() const { return root_; }
  size_type& get_size() { return size_pair_.sz; }
  size_type const& get_size() const { return size_pair_.sz; }
  size_type& get_height() { return height_pair_.ht; }
  size_type const& get_height() const { return height_pair_.ht; }
  allocator_type& get_element_allocator() { return size_pair_; }
  allocator_type const& get_element_allocator() const { return size_pair_; }
  node_allocator& get_node_allocator() { return height_pair_; }
  node_allocator const& get_node_allocator() const { return height_pair_; }

  // allocate
  element_pointer allocate_segment() {
    return element_traits::allocate(get_element_allocator(),
                                    static_traits::segment_max());
  }

  node_pointer allocate_node() {
    return node_traits::allocate(get_node_allocator(), 1);
  }

  // destroy
  void destroy_segment(element_pointer pointer, size_type index) {
    element_traits::destroy(get_element_allocator(),
                            std::addressof(pointer[index]));
  }

  void destroy_node(node_pointer pointer, size_type index) {
    pointer->pointers[index].~void_pointer();
  }

  // deallocate
  void deallocate_segment(element_pointer pointer) {
    element_traits::deallocate(get_element_allocator(), pointer,
                               static_traits::segment_max());
  }

  void deallocate_node(node_pointer pointer) {
    node_traits::deallocate(get_node_allocator(), pointer, 1);
  }

  // purge
  void purge() { purge_root(get_root(), get_size(), get_height()); }

  void purge_segment(element_pointer, size_type, std::true_type) {}

  void purge_segment(element_pointer pointer, size_type sz, std::false_type) {
    for (size_type i = 0, e = sz; i != e; ++i) destroy_segment(pointer, i);
  }

  void purge_segment(element_pointer pointer, size_type sz) {
    purge_segment(
        pointer, sz,
        std::integral_constant<bool,
                               std::is_trivially_destructible<T>::value>{});
    deallocate_segment(pointer);
  }

  void purge_node(node_pointer pointer, size_type ht) {
    if (ht == 2) {
      for (size_type i = 0, e = pointer->length(); i != e; ++i) {
        purge_segment(static_traits::cast_segment(pointer->pointers[i]),
                      pointer->sizes[i]);
        destroy_node(pointer, i);
      }
    } else {
      for (size_type i = 0, e = pointer->length(); i != e; ++i) {
        purge_node(static_traits::cast_node(pointer->pointers[i]), ht - 1);
        destroy_node(pointer, i);
      }
    }
    deallocate_node(pointer);
  }

  void purge_root(void_pointer pointer, size_type sz, size_type ht) {
    if (ht < 2)
      purge_segment(static_traits::cast_segment(pointer), sz);
    else
      purge_node(static_traits::cast_node(pointer), ht);
  }

  // construct
  size_type construct_segment(element_pointer pointer, size_type index,
                              value_type&& value) {
    element_traits::construct(get_element_allocator(),
                              std::addressof(pointer[index]), std::move(value));
    return 1;
  }

  size_type construct_leaf(node_pointer pointer, size_type index,
                           std::size_t child_sz, void_pointer child_pointer) {
    pointer->sizes[index] = child_sz;
    ::new (static_cast<void*>(std::addressof(pointer->pointers[index]))) auto(
        child_pointer);
    return child_sz;
  }

  size_type construct_branch(node_pointer pointer, size_type index,
                             std::size_t child_sz, void_pointer child_pointer) {
    pointer->sizes[index] = child_sz;
    auto child = static_traits::cast_node(child_pointer);
    child->parent_pointer = pointer;
    child->parent_index(index);
    ::new (static_cast<void*>(std::addressof(pointer->pointers[index]))) auto(
        child_pointer);
    return child_sz;
  }

  // assign
  size_type assign_segment(element_pointer pointer, size_type index,
                           value_type&& value) {
    pointer[index] = std::move(value);
    return 1;
  }

  size_type assign_leaf(node_pointer pointer, size_type index,
                        std::size_t child_sz, void_pointer child_pointer) {
    pointer->sizes[index] = child_sz;
    pointer->pointers[index] = child_pointer;
    return child_sz;
  }

  size_type assign_branch(node_pointer pointer, size_type index,
                          std::size_t child_sz, void_pointer child_pointer) {
    pointer->sizes[index] = child_sz;
    auto child = static_traits::cast_node(child_pointer);
    child->parent_pointer = pointer;
    child->parent_index(index);
    pointer->pointers[index] = child_pointer;
    return child_sz;
  }

  // move_assign
  size_type move_assign_segment(element_pointer source, size_type source_index,
                                element_pointer dest, size_type dest_index) {
    assign_segment(dest, dest_index, std::move(source[source_index]));
    return 1;
  }

  size_type move_assign_leaf(node_pointer source, size_type source_index,
                             node_pointer dest, size_type dest_index) {
    return assign_leaf(dest, dest_index, source->sizes[source_index],
                       source->pointers[source_index]);
  }

  size_type move_assign_branch(node_pointer source, size_type source_index,
                               node_pointer dest, size_type dest_index) {
    return assign_branch(dest, dest_index, source->sizes[source_index],
                         source->pointers[source_index]);
  }

  // move
  size_type move_segment(element_pointer source, size_type source_index,
                         element_pointer dest, size_type dest_index) {
    construct_segment(dest, dest_index, std::move(source[source_index]));
    return 1;
  }

  size_type move_leaf(node_pointer source, size_type source_index,
                      node_pointer dest, size_type dest_index) {
    auto child_sz = source->sizes[source_index];
    auto child_pointer = source->pointers[source_index];
    construct_leaf(dest, dest_index, child_sz, child_pointer);
    return child_sz;
  }

  size_type move_branch(node_pointer source, size_type source_index,
                        node_pointer dest, size_type dest_index) {
    auto child_sz = source->sizes[source_index];
    auto child_pointer = source->pointers[source_index];
    construct_branch(dest, dest_index, child_sz, child_pointer);
    return child_sz;
  }

  // construct_range
  void construct_range_segment(element_pointer source, size_type source_index,
                               element_pointer dest, size_type dest_index,
                               size_type count, std::true_type) {
    std::memcpy(std::addressof(dest[dest_index]),
                std::addressof(source[source_index]), count * sizeof(T));
  }

  void construct_range_segment(element_pointer source, size_type source_index,
                               element_pointer dest, size_type dest_index,
                               size_type count, std::false_type) {
    auto from = source_index;
    auto last = source_index + count;
    auto to = dest_index;

    while (from != last) {
      construct_segment(dest, to, std::move(source[from]));
      destroy_segment(source, from);
      ++from;
      ++to;
    }
  }

  size_type construct_range_segment(element_pointer source,
                                    size_type source_index,
                                    element_pointer dest, size_type dest_index,
                                    size_type count) {
    construct_range_segment(
        source, source_index, dest, dest_index, count,
        std::integral_constant < bool,
        std::is_trivially_copyable<T>::value &&
            (std::is_same<allocator_type, std::allocator<value_type>>::value ||
             detail::is_alloc_move_construct_default<value_type,
                                                     allocator_type>::value) >
                {});
    return count;
  }

  size_type construct_range_leaf(node_pointer source, size_type source_index,
                                 node_pointer dest, size_type dest_index,
                                 size_type count) {
    size_type copy_size = 0;
    auto from = source_index;
    auto last = source_index + count;
    auto to = dest_index;

    while (from != last) {
      copy_size += move_assign_leaf(source, from, dest, to);
      ++from;
      ++to;
    }

    return copy_size;
  }

  size_type construct_range_branch(node_pointer source, size_type source_index,
                                   node_pointer dest, size_type dest_index,
                                   size_type count) {
    size_type copy_size = 0;
    auto from = source_index;
    auto last = source_index + count;
    auto to = dest_index;

    while (from != last) {
      copy_size += move_assign_branch(source, from, dest, to);
      ++from;
      ++to;
    }

    return copy_size;
  }

  // assign_forward
  void assign_forward_segment(element_pointer pointer, size_type length,
                              size_type index, size_type distance,
                              std::true_type) {
    std::memmove(std::addressof(pointer[index + distance]),
                 std::addressof(pointer[index]), (length - index) * sizeof(T));
  }

  void assign_forward_segment(element_pointer pointer, size_type length,
                              size_type index, size_type distance,
                              std::false_type) {
    auto first = index;
    auto from = length;
    auto to = length + distance;

    while (first != from) {
      --from;
      --to;
      move_assign_segment(pointer, from, pointer, to);
    }
  }

  void assign_forward_segment(element_pointer pointer, size_type length,
                              size_type index, size_type distance) {
    assign_forward_segment(
        pointer, length, index, distance,
        std::integral_constant<bool, std::is_trivially_copyable<T>::value>{});
  }

  void assign_forward_leaf(node_pointer pointer, size_type length,
                           size_type index, size_type distance) {
    auto first = index;
    auto from = length;
    auto to = length + distance;

    while (first != from) {
      --from;
      --to;
      move_assign_leaf(pointer, from, pointer, to);
    }
  }

  void assign_forward_branch(node_pointer pointer, size_type length,
                             size_type index, size_type distance) {
    auto first = index;
    auto from = length;
    auto to = length + distance;

    while (first != from) {
      --from;
      --to;
      move_assign_branch(pointer, from, pointer, to);
    }
  }

  // assign_backward
  void assign_backward_segment(element_pointer pointer, size_type length,
                               size_type index, size_type distance,
                               std::true_type) {
    std::memmove(std::addressof(pointer[index]),
                 std::addressof(pointer[index + distance]),
                 (length - index) * sizeof(T));
  }

  void assign_backward_segment(element_pointer pointer, size_type length,
                               size_type index, size_type distance,
                               std::false_type) {
    auto from = index + distance;
    auto to = index;
    auto last = length;

    while (to != last) {
      move_assign_segment(pointer, from, pointer, to);
      ++from;
      ++to;
    }
  }

  void assign_backward_segment(element_pointer pointer, size_type length,
                               size_type index, size_type distance) {
    assign_backward_segment(
        pointer, length, index, distance,
        std::integral_constant<bool, std::is_trivially_copyable<T>::value>{});
  }

  void assign_backward_leaf(node_pointer pointer, size_type length,
                            size_type index, size_type distance) {
    auto from = index + distance;
    auto to = index;
    auto last = length;

    while (to != last) {
      move_assign_leaf(pointer, from, pointer, to);
      ++from;
      ++to;
    }
  }

  void assign_backward_branch(node_pointer pointer, size_type length,
                              size_type index, size_type distance) {
    auto from = index + distance;
    auto to = index;
    auto last = length;

    while (to != last) {
      move_assign_branch(pointer, from, pointer, to);
      ++from;
      ++to;
    }
  }

  // update_sizes
  void update_sizes(node_pointer pointer, size_type index, size_type sz) {
    while (pointer != nullptr) {
      pointer->sizes[index] += sz;
      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
    }

    get_size() += sz;
  }

  void increment_sizes(node_pointer pointer, size_type index,
                       std::size_t by = 1) {
    update_sizes(pointer, index, by);
  }

  void decrement_sizes(node_pointer pointer, size_type index,
                       std::size_t by = 1) {
    update_sizes(pointer, index, ~by + 1);
  }

  // alloc_nodes_single
  node_pointer alloc_nodes_single(node_pointer pointer,
                                  element_pointer segment_alloc) {
    node_pointer alloc = nullptr;
    try {
      while (true) {
        if (pointer == nullptr) {
          auto temp = allocate_node();
          temp->parent_pointer = alloc;
          alloc = temp;
          return alloc;
        }

        if (pointer->length() != static_traits::base_max()) return alloc;

        auto temp = allocate_node();
        temp->parent_pointer = alloc;
        alloc = temp;

        pointer = pointer->parent_pointer;
      }
    } catch (...) {
      deallocate_segment(segment_alloc);

      while (alloc != nullptr) {
        auto temp = alloc->parent_pointer;
        deallocate_node(alloc);
        alloc = temp;
      }
      throw;
    }
  }

  //   insert_single
  void insert_single_iterator(iterator_data& it, value_type value) {
    insert_single_segment(it.entry, std::move(value));
  }

  void insert_single_segment(iterator_entry& entry, value_type value) {
    auto pointer = entry.segment.pointer;
    auto index = entry.segment.index;
    auto length = entry.segment.length;
    auto parent_pointer = entry.leaf.pointer;
    auto parent_index = entry.leaf.index;

    if (index != length && length != static_traits::segment_max()) {
      move_segment(pointer, length - 1, pointer, length);
      assign_forward_segment(pointer, length - 1, index, 1);
      assign_segment(pointer, index, std::move(value));
      ++entry.segment.length;
      increment_sizes(parent_pointer, parent_index);
      return;
    }

    if (pointer == nullptr) {
      auto alloc = allocate_segment();
      get_root() = alloc;
      get_size() = 1;
      get_height() = 1;
      construct_segment(alloc, 0, std::move(value));
      entry.segment.pointer = alloc;
      entry.segment.length = 1;
      return;
    }

    if (length != static_traits::segment_max()) {
      construct_segment(pointer, index, std::move(value));
      ++entry.segment.length;
      increment_sizes(parent_pointer, parent_index);
      return;
    }

    auto alloc = allocate_segment();
    auto leaf_alloc = alloc_nodes_single(parent_pointer, alloc);

    constexpr auto sum = static_traits::segment_max() + 1;
    constexpr auto pointer_length = sum / 2;
    constexpr auto alloc_length = sum - pointer_length;

    if (index < pointer_length) {
      auto left_index = pointer_length - 1;
      move_segment(pointer, left_index, alloc, 0);
      construct_range_segment(pointer, left_index + 1, alloc, 1,
                              alloc_length - 1);
      assign_forward_segment(pointer, left_index, index, 1);
      assign_segment(pointer, index, std::move(value));

      entry.segment.length = pointer_length;
    } else {
      auto new_index = index - pointer_length;
      auto move_length = length - index;
      construct_range_segment(pointer, pointer_length, alloc, 0, new_index);
      construct_range_segment(pointer, index, alloc, new_index + 1,
                              move_length);
      construct_segment(alloc, new_index, std::move(value));
      entry.segment.length = alloc_length;
      entry.segment.pointer = alloc;
      entry.segment.index = new_index;
      ++entry.leaf.index;
    }

    insert_single_leaf(entry, pointer, parent_pointer, parent_index + 1,
                       leaf_alloc, alloc, alloc_length);
  }

  void insert_single_leaf(iterator_entry& entry, element_pointer base,
                          node_pointer pointer, size_type index,
                          node_pointer alloc, element_pointer child_pointer,
                          size_type child_size) {
    if (pointer == nullptr) {
      alloc->parent_pointer = nullptr;
      alloc->parent_index(0);
      alloc->length(2);
      construct_leaf(alloc, 0, get_size() - child_size + 1, base);
      construct_leaf(alloc, 1, child_size, child_pointer);
      get_root() = alloc;
      get_height() = 2;
      ++get_size();
      entry.leaf.pointer = alloc;
      return;
    }

    auto length = pointer->length();
    pointer->sizes[index - 1] -= child_size - 1;

    if (length != static_traits::base_max()) {
      if (index != length) {
        move_leaf(pointer, length - 1, pointer, length);
        assign_forward_leaf(pointer, length - 1, index, 1);
        assign_leaf(pointer, index, child_size, child_pointer);
      } else
        construct_leaf(pointer, index, child_size, child_pointer);
      pointer->length(length + 1);
      increment_sizes(pointer->parent_pointer, pointer->parent_index());
      return;
    }

    auto next_alloc = alloc->parent_pointer;
    constexpr auto sum = static_traits::base_max() + 1;
    constexpr auto pointer_length = sum / 2;
    constexpr auto alloc_length = sum - pointer_length;

    size_type alloc_size = 0;
    if (index < pointer_length) {
      auto left_index = pointer_length - 1;
      alloc_size += move_leaf(pointer, left_index, alloc, 0);
      alloc_size += construct_range_leaf(pointer, left_index + 1, alloc, 1,
                                         alloc_length - 1);
      assign_forward_leaf(pointer, left_index, index, 1);
      assign_leaf(pointer, index, child_size, child_pointer);
    } else {
      auto new_index = index - pointer_length;
      auto move_length = length - index;
      alloc_size +=
          construct_range_leaf(pointer, pointer_length, alloc, 0, new_index);
      alloc_size += construct_range_leaf(pointer, index, alloc, new_index + 1,
                                         move_length);
      alloc_size += construct_leaf(alloc, new_index, child_size, child_pointer);
    }

    pointer->length(pointer_length);
    alloc->length(alloc_length);

    if (entry.leaf.index >= pointer_length) {
      entry.leaf.pointer = alloc;
      entry.leaf.index -= pointer_length;
    }

    insert_single_branch(pointer, pointer->parent_pointer,
                         pointer->parent_index() + 1, next_alloc, alloc,
                         alloc_size);
  }

  void insert_single_branch(node_pointer base, node_pointer pointer,
                            size_type index, node_pointer alloc,
                            node_pointer child_pointer, size_type child_size) {
    while (true) {
      if (pointer == nullptr) {
        alloc->parent_pointer = nullptr;
        alloc->parent_index(0);
        alloc->length(2);
        construct_branch(alloc, 0, get_size() - child_size + 1, base);
        construct_branch(alloc, 1, child_size, child_pointer);
        get_root() = alloc;
        ++get_height();
        ++get_size();
        return;
      }

      auto length = pointer->length();
      pointer->sizes[index - 1] -= child_size - 1;

      if (length != static_traits::base_max()) {
        if (index != length) {
          move_branch(pointer, length - 1, pointer, length);
          assign_forward_branch(pointer, length - 1, index, 1);
          assign_branch(pointer, index, child_size, child_pointer);
        } else
          construct_branch(pointer, index, child_size, child_pointer);
        pointer->length(length + 1);
        increment_sizes(pointer->parent_pointer, pointer->parent_index());
        return;
      }

      auto next_alloc = alloc->parent_pointer;
      constexpr auto sum = static_traits::base_max() + 1;
      constexpr auto pointer_length = sum / 2;
      constexpr auto alloc_length = sum - pointer_length;

      size_type alloc_size = 0;
      if (index < pointer_length) {
        auto left_index = pointer_length - 1;
        alloc_size += move_branch(pointer, left_index, alloc, 0);
        alloc_size += construct_range_branch(pointer, left_index + 1, alloc, 1,
                                             alloc_length - 1);
        assign_forward_branch(pointer, left_index, index, 1);
        assign_branch(pointer, index, child_size, child_pointer);
      } else {
        auto new_index = index - pointer_length;
        auto move_length = length - index;
        alloc_size += construct_range_branch(pointer, pointer_length, alloc, 0,
                                             new_index);
        alloc_size += construct_range_branch(pointer, index, alloc,
                                             new_index + 1, move_length);
        alloc_size +=
            construct_branch(alloc, new_index, child_size, child_pointer);
      }

      pointer->length(pointer_length);
      alloc->length(alloc_length);

      child_pointer = alloc;
      child_size = alloc_size;
      base = pointer;
      index = pointer->parent_index() + 1;
      pointer = pointer->parent_pointer;
      alloc = next_alloc;
    }
  }

  // erase_single
  void erase_single_iterator(iterator_data& it) {
    erase_single_segment(it.entry);
  }
  
//  __attribute__ ((noinline))
  void erase_single_segment(iterator_entry& entry) {
    auto pointer = entry.segment.pointer;
    auto index = entry.segment.index;
    auto length = entry.segment.length;
    auto parent_pointer = entry.leaf.pointer;
    auto parent_index = entry.leaf.index;
    
    // last value
    if (length == 1 &&
        (static_traits::segment_min() != 1 || parent_pointer == nullptr)) {
      destroy_segment(pointer, 0);
      deallocate_segment(pointer);
      get_root() = nullptr;
      get_size() = 0;
      get_height() = 0;
      entry.segment.pointer = nullptr;
      entry.segment.index = 0;
      entry.segment.length = 0;
      return;
    }
    
    // erase value in segment
    if (length-- != static_traits::segment_min() || parent_pointer == nullptr) {
      assign_backward_segment(pointer, length, index, 1);
      destroy_segment(pointer, length);
      entry.segment.length = length;
      decrement_sizes(parent_pointer, parent_index);
//++iii1;
      return;
    }

    constexpr auto merge_size = static_traits::segment_min() * 2 - 1;
    auto pointers = &parent_pointer->pointers[0];
    auto sizes = &parent_pointer->sizes[0];
    
    // check previous segment
    size_type erase_index;
    if (parent_index != 0) {
      auto prev_index = parent_index - 1;
      auto prev_pointer = static_traits::cast_segment(pointers[prev_index]);
      auto prev_length = sizes[prev_index];
      
      // steal value from previous segment
      if (prev_length != static_traits::segment_min()) {
        --prev_length;
        assign_forward_segment(pointer, index, 0, 1);
        move_assign_segment(prev_pointer, prev_length, pointer, 0);
        destroy_segment(prev_pointer, prev_length);
        sizes[prev_index] = prev_length;
        ++entry.segment.index;
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());
//++iii2;
        return;
      }
      
      // merge in previous segment
      construct_range_segment(pointer, 0, prev_pointer, prev_length, index);
      construct_range_segment(pointer, index + 1, prev_pointer,
                              prev_length + index, length - index);
      destroy_segment(pointer, index);
      deallocate_segment(pointer);
      sizes[prev_index] = merge_size;
      erase_index = parent_index;
      entry.segment.pointer = prev_pointer;
      entry.segment.length = merge_size;
      entry.segment.index += static_traits::segment_min();
      --entry.leaf.index;
//++iii3;
    }
    // check next segment
    else {
      auto next_index = parent_index + 1;
      auto next_pointer = static_traits::cast_segment(pointers[next_index]);
      auto next_length = sizes[next_index];
      
      // steal value from next segment
      if (next_length != static_traits::segment_min()) {
        --next_length;
        assign_backward_segment(pointer, length, index, 1);
        move_assign_segment(next_pointer, 0, pointer, length);
        assign_backward_segment(next_pointer, next_length, 0, 1);
        destroy_segment(next_pointer, next_length);
        sizes[next_index] = next_length;
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());
//++iii4;
        return;
      }
      // merge next to current segment
      assign_backward_segment(pointer, length, index, 1);
      move_assign_segment(next_pointer, 0, pointer, length);
      destroy_segment(next_pointer, 0);
      construct_range_segment(next_pointer, 1, pointer, length + 1,
                              next_length - 1);
      deallocate_segment(next_pointer);

      sizes[parent_index] = merge_size;
      erase_index = next_index;
      entry.segment.length = merge_size;
//++iii5;
    }

    erase_single_leaf(entry.leaf, parent_pointer, erase_index);
//++iii6;
  }

  void erase_single_leaf(leaf_entry& entry, node_pointer pointer,
                         size_type index) {
    auto parent_pointer = pointer->parent_pointer;
    auto parent_index = pointer->parent_index();
    auto length = pointer->length();

    if (length == 2 &&
        (static_traits::base_min() != 2 || parent_pointer == nullptr)) {
      auto other = pointer->pointers[index ^ 1];
      destroy_node(pointer, 0);
      destroy_node(pointer, 1);
      deallocate_node(pointer);
      get_root() = other;
      --get_size();
      get_height() = 1;
      entry.pointer = nullptr;
      entry.index = 0;
      return;
    }

    if (length-- != static_traits::base_min() || parent_pointer == nullptr) {
      assign_backward_leaf(pointer, length, index, 1);
      destroy_node(pointer, length);
      pointer->length(length);
      decrement_sizes(parent_pointer, parent_index);
      return;
    }

    auto pointers = &parent_pointer->pointers[0];
    auto sizes = &parent_pointer->sizes[0];

    size_type erase_index;
    if (parent_index != 0) {
      auto prev_index = parent_index - 1;
      auto prev_pointer = static_traits::cast_node(pointers[prev_index]);
      auto prev_length = prev_pointer->length();

      if (prev_length != static_traits::base_min()) {
        --prev_length;
        assign_forward_leaf(pointer, index, 0, 1);
        auto sz = move_assign_leaf(prev_pointer, prev_length, pointer, 0);
        destroy_node(prev_pointer, prev_length);
        sizes[prev_index] -= sz;
        sizes[parent_index] += sz - 1;
        prev_pointer->length(prev_length);
        ++entry.index;
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());
        return;
      }

      auto sz =
          construct_range_leaf(pointer, 0, prev_pointer, prev_length, index);
      sz += construct_range_leaf(pointer, index + 1, prev_pointer,
                                 prev_length + index, length - index);
      destroy_node(pointer, index);
      deallocate_node(pointer);
      prev_pointer->length(prev_length + length);
      sizes[prev_index] += sz;
      erase_index = parent_index;
      entry.pointer = prev_pointer;
      entry.index += prev_length;
    }

    else {
      auto next_index = parent_index + 1;
      auto next_pointer = static_traits::cast_node(pointers[next_index]);
      auto next_length = next_pointer->length();

      if (next_length != static_traits::base_min()) {
        --next_length;
        assign_backward_leaf(pointer, length, index, 1);
        auto sz = move_assign_leaf(next_pointer, 0, pointer, length);
        assign_backward_leaf(next_pointer, next_length, 0, 1);
        destroy_node(next_pointer, next_length);
        sizes[next_index] -= sz;
        sizes[parent_index] += sz - 1;
        next_pointer->length(next_length);
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());
        return;
      }

      assign_backward_leaf(pointer, length, index, 1);
      auto sz = move_assign_leaf(next_pointer, 0, pointer, length);
      destroy_node(next_pointer, 0);
      sz += construct_range_leaf(next_pointer, 1, pointer, length + 1,
                                 next_length - 1);
      deallocate_node(next_pointer);
      pointer->length(length + next_length);
      sizes[parent_index] += sz - 1;
      erase_index = next_index;
    }

    erase_single_branch(parent_pointer, erase_index);
  }

  void erase_single_branch(node_pointer pointer, size_type index) {
    while (true) {
      auto parent_pointer = pointer->parent_pointer;
      auto parent_index = pointer->parent_index();
      auto length = pointer->length();

      if (length == 2 &&
          (static_traits::base_min() != 2 || parent_pointer == nullptr)) {
        auto other = static_traits::cast_node(pointer->pointers[index ^ 1]);
        destroy_node(pointer, 0);
        destroy_node(pointer, 1);
        deallocate_node(pointer);
        get_root() = other;
        other->parent_pointer = nullptr;
        other->parent_index(0);
        --get_size();
        --get_height();
        return;
      }

      if (length-- != static_traits::base_min() || parent_pointer == nullptr) {
        assign_backward_branch(pointer, length, index, 1);
        destroy_node(pointer, length);
        pointer->length(length);
        decrement_sizes(parent_pointer, parent_index);
        return;
      }

      auto pointers = &parent_pointer->pointers[0];
      auto sizes = &parent_pointer->sizes[0];

      size_type erase_index;
      if (parent_index != 0) {
        auto prev_index = parent_index - 1;
        auto prev_pointer = static_traits::cast_node(pointers[prev_index]);
        auto prev_length = prev_pointer->length();

        if (prev_length != static_traits::base_min()) {
          --prev_length;
          assign_forward_branch(pointer, index, 0, 1);
          auto sz = move_assign_branch(prev_pointer, prev_length, pointer, 0);
          destroy_node(prev_pointer, prev_length);
          sizes[prev_index] -= sz;
          sizes[parent_index] += sz - 1;
          prev_pointer->length(prev_length);
          decrement_sizes(parent_pointer->parent_pointer,
                          parent_pointer->parent_index());
          return;
        }

        auto sz = construct_range_branch(pointer, 0, prev_pointer, prev_length,
                                         index);
        sz += construct_range_branch(pointer, index + 1, prev_pointer,
                                     prev_length + index, length - index);
        destroy_node(pointer, index);
        deallocate_node(pointer);
        prev_pointer->length(prev_length + length);
        sizes[prev_index] += sz;
        erase_index = parent_index;
      }

      else {
        auto next_index = parent_index + 1;
        auto next_pointer = static_traits::cast_node(pointers[next_index]);
        auto next_length = next_pointer->length();

        if (next_length != static_traits::base_min()) {
          --next_length;
          assign_backward_branch(pointer, length, index, 1);
          auto sz = move_assign_branch(next_pointer, 0, pointer, length);
          assign_backward_branch(next_pointer, next_length, 0, 1);
          destroy_node(next_pointer, next_length);
          sizes[next_index] -= sz;
          sizes[parent_index] += sz - 1;
          next_pointer->length(next_length);
          decrement_sizes(parent_pointer->parent_pointer,
                          parent_pointer->parent_index());
          return;
        }

        assign_backward_branch(pointer, length, index, 1);
        auto sz = move_assign_branch(next_pointer, 0, pointer, length);
        destroy_node(next_pointer, 0);
        sz += construct_range_branch(next_pointer, 1, pointer, length + 1,
                                     next_length - 1);
        deallocate_node(next_pointer);
        pointer->length(length + next_length);
        sizes[parent_index] += sz - 1;
        erase_index = next_index;
      }

      pointer = parent_pointer;
      index = erase_index;
    }
  }

  // helpers
  iterator_data find_index(size_type pos) const {
    return static_traits::find_index_root(get_root(), get_size(), get_height(),
                                          pos);
  }

  iterator_data find_first() const {
    return static_traits::find_first_root(get_root(), get_size(), get_height());
  }

  iterator_data find_last() const {
    return static_traits::find_last_root(get_root(), get_size(), get_height());
  }

  iterator_data find_end() const {
    return static_traits::find_end_root(get_root(), get_size(), get_height());
  }

  void steal(seq& other) {
    get_root() = other.get_root();
    get_height() = other.get_height();
    get_size() = other.get_size();
    other.get_root() = nullptr;
    other.get_height() = 0;
    other.get_size() = 0;
  }

  template <class... Args>
  iterator_data emplace_single(iterator_data it, Args&&... args) {
    insert_single_iterator(it, value_type(std::forward<Args>(args)...));
    return it;
  }

  template <typename... Args>
  iterator_data emplace_count(iterator_data it, size_type count,
                              Args&&... args) {
    for (size_type i = 0; i != count; ++i) {
      it = emplace_single(it, std::forward<Args>(args)...);
      static_traits::move_next_iterator(it);
    }
    static_traits::move_prev_iterator_count(it, count);
    return it;
  }

  template <class InputIt>
  iterator_data emplace_range(iterator_data it, InputIt first, InputIt last) {
    size_type count = 0;
    while (first != last) {
      it = emplace_single(it, *first);
      ++first;
      static_traits::move_next_iterator(it);
      ++count;
    }
    static_traits::move_prev_iterator_count(it, count);
    return it;
  }
  
//  __attribute__ ((noinline))
  //__declspec(noinline)
  iterator_data erase_single(iterator_data it) {
    erase_single_iterator(it);
    if (it.entry.segment.index == it.entry.segment.length)
      static_traits::move_next_leaf(it.entry);
    return it;
  }

  iterator_data erase_range(iterator_data first, iterator_data last) {
    while (last.pos - first.pos) {
      static_traits::move_prev_iterator(last);
      last = erase_single(last);
    }
    return last;
  }

  void assign_count(size_type count, T const& value) {
    auto first = find_first();
    auto last = find_end();
    while (true) {
      if (count == 0) {
        erase_range(first, last);
        return;
      }

      if (first.pos == last.pos) {
        emplace_count(last, count, value);
        return;
      }

      first.entry.segment.pointer[first.entry.segment.index] = value;
      static_traits::move_next_iterator(first);
      --count;
    }
  }

  template <class InputIt>
  void assign_range(InputIt source_first, InputIt source_last) {
    auto first = find_first();
    auto last = find_end();
    while (true) {
      if (source_first == source_last) {
        erase_range(first, last);
        return;
      }

      if (first.pos == last.pos) {
        emplace_range(last, source_first, source_last);
        return;
      }

      first.entry.segment.pointer[first.entry.segment.index] = *source_first;
      static_traits::move_next_iterator(first);
      ++source_first;
    }
  }

  template <typename... Args>
  void resize_count(size_type count, Args&&... args) {
    auto sz = size();
    if (sz == count) return;

    auto last = find_end();
    if (count < sz)
      erase_range(find_index(count), last);
    else
      emplace_count(last, count - sz, std::forward<Args>(args)...);
  }

  void copy_assign_alloc(seq const& other)
  {
    copy_assign_alloc(
        other,
        std::integral_constant<
            bool,
            element_traits::propagate_on_container_copy_assignment::value>{});
  }

  template <typename = void>
  void copy_assign_alloc(seq const& other, std::true_type)
  {
    if (get_element_allocator() != other.get_element_allocator()) clear();
    get_element_allocator() = other.get_element_allocator();
    get_node_allocator() = other.get_node_allocator();
  }

  template <typename = void>
  void copy_assign_alloc(seq const&, std::false_type) {}

  void move_assign_alloc(seq& other) noexcept(
      !element_traits::propagate_on_container_move_assignment::value ||
      std::is_nothrow_move_assignable<allocator_type>::value)
  {
    move_assign_alloc(
        other,
        std::integral_constant<
            bool,
            element_traits::propagate_on_container_move_assignment::value>{});
  }

  template <typename = void>
  void move_assign_alloc(seq& other, std::true_type) noexcept(
      std::is_nothrow_move_assignable<allocator_type>::value)
  {
    get_element_allocator() = std::move(other.get_element_allocator());
    get_node_allocator() = std::move(other.get_node_allocator());
  }

  template <typename = void>
  void move_assign_alloc(seq const&, std::false_type) noexcept {}

  void move_assign(seq& other) noexcept(
      element_traits::propagate_on_container_move_assignment::value &&
          std::is_nothrow_move_assignable<allocator_type>::value)
  {
    move_assign(
        other,
        std::integral_constant<
            bool,
            element_traits::propagate_on_container_move_assignment::value>{});
  }

  template <typename = void>
  void move_assign(seq& other, std::false_type)
  {
    if (get_element_allocator() != other.get_element_allocator()) {
      assign(std::make_move_iterator(other.begin()),
             std::make_move_iterator(other.end()));
    } else
      move_assign(other, std::true_type{});
  }

  template <typename = void>
  void move_assign(seq& other, std::true_type)
  {
    purge();
    move_assign_alloc(other);
    steal(other);
  }

  void swap_allocator(seq& other) {
    swap_allocator(
        other, std::integral_constant<
                   bool, element_traits::propagate_on_container_swap::value>{});
  }

  template <typename = void>
  void swap_allocator(seq& other, std::true_type) noexcept(
      detail::is_nothrow_swappable<allocator_type>::value) {
    using std::swap;
    swap(get_element_allocator(), other.get_element_allocator());
    swap(get_node_allocator(), other.get_node_allocator());
  }

  template <typename = void>
  void swap_allocator(seq&, std::false_type) noexcept {}
#endif  // #ifndef BOOST_SEGMENTED_TREE_DOXYGEN_INVOKED

 public:
  // public interface
  /// \par Effects
  ///   Default constructs an empty sequence.
  ///
  /// \par Complexity
  ///   Constant.
  explicit seq() noexcept(
      std::is_nothrow_default_constructible<allocator_type>::value) {}

  /// \par Effects
  ///   Constructs an empty sequence using the specified allocator.
  ///
  /// \par Complexity
  ///   Constant.
  explicit seq(Allocator const& alloc)
      : size_pair_{alloc}, height_pair_{alloc} {}

  /// \par Effects
  ///   Constructs a count size sequence using the specified allocator, each
  ///   element copy constructed from value.
  ///
  /// \par Complexity
  ///   NlogN, where N is count.
  seq(size_type count, T const& value, Allocator const& alloc = Allocator())
      : seq{alloc} {
    emplace_count(find_end(), count, value);
  }

  /// \par Effects
  ///   Constructs a count size sequence using the specified allocator, each
  ///   element default constructed.
  ///
  /// \par Complexity
  ///   NlogN, where N is count.
  explicit seq(size_type count, Allocator const& alloc = Allocator())
      : seq{alloc} {
    emplace_count(find_end(), count);
  }

  /// \par Effects
  ///   Constructs an empty sequence using the specified allocator, and inserts
  ///   elements from the range [first, last).
  ///
  /// \par Complexity
  ///   NlogN, where N is the size of the range.
  template <class InputIt,
            typename = typename std::iterator_traits<InputIt>::pointer>
  seq(InputIt first, InputIt last, Allocator const& alloc = Allocator())
      : seq{alloc} {
    emplace_range(find_end(), first, last);
  }

  /// \par Effects
  ///   Copy constructs a sequence.
  ///
  /// \par Complexity
  ///   NlogN, where N is other.size().
  seq(seq const& other)
      : seq{element_traits::select_on_container_copy_construction(
            other.get_element_allocator())} {
    emplace_range(find_end(), other.begin(), other.end());
  }

  /// \par Effects
  ///   Copy constructs a sequence using the specified allocator.
  ///
  /// \par Complexity
  ///   NlogN, where N is other.size().
  seq(seq const& other, Allocator const& alloc) : seq{alloc} {
    emplace_range(find_end(), other.begin(), other.end());
  }

  /// \par Effects
  ///   Move constructs a sequence.
  ///
  /// \par Complexity
  ///   Constant.
  seq(seq&& other) noexcept(
      std::is_nothrow_move_constructible<allocator_type>::value)
      : root_{other.root_},
        size_pair_{std::move(other.size_pair_)},
        height_pair_{std::move(other.height_pair_)} {
    other.get_root() = nullptr;
    other.get_height() = 0;
    other.get_size() = 0;
  }

  /// \par Effects
  ///   Move constructs a sequence using the specified allocator.
  ///
  /// \par Complexity
  ///   Constant if alloc compares equal to other's allocator. NlogN, where N is
  ///   other.size() otherwise.
  seq(seq&& other, Allocator const& alloc) : seq{alloc} {
    if (get_element_allocator() == other.get_element_allocator())
      steal(other);
    else
      emplace_range(find_end(), std::make_move_iterator(other.begin()),
                    std::make_move_iterator(other.end()));
  }

  /// \par Effects
  ///   Constructs an empty sequence using the specified allocator, and inserts
  ///   elements from init.
  ///
  /// \par Complexity
  ///   NlogN, where N is init.size().
  seq(std::initializer_list<T> init, Allocator const& alloc = Allocator())
      : seq{alloc} {
    emplace_range(find_end(), init.begin(), init.end());
  }

  /// \par Effects
  ///   Destructs the sequence releasing all memory.
  ///
  /// \par Complexity
  ///   Linear in size().
  ~seq() { purge(); }

  /// \par Effects
  ///   Copy assigns a sequence.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and other.size(), plus MlogN, where M is
  ///   the difference of size() and other.size(), and N is the maximum of
  ///   size() and other.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  seq& operator=(seq const& other) {
    if (this != &other) {
      copy_assign_alloc(other);
      assign_range(other.begin(), other.end());
    }
    return *this;
  }

  /// \par Effects
  ///   Move assigns a sequence.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Linear in size() if the allocator propagates on move assignment or the
  ///   sequence's allocator compares equal to other's allocator. Linear in the
  ///   minimum of size() and other.size(), plus MlogN, where M is the
  ///   difference of size() and other.size(), and N is the maximum of size()
  ///   and other.size() otherwise.
  ///
  /// \par Iterator invalidation
  ///   No iterators are invalidated if the allocator propagates on move
  ///   assignment or the sequence's allocator compares equal to other's
  ///   allocator. Invalidates all iterators otherwise.
  ///
  /// \par Exception safety
  ///   No-throw if the allocator propagates on move assignment or the
  ///   sequence's allocator compares equal to other's allocator. Basic
  ///   otherwise.
  seq& operator=(seq&& other) noexcept(
      element_traits::propagate_on_container_move_assignment::value&&
          std::is_nothrow_move_assignable<allocator_type>::value) {
    move_assign(other);
    return *this;
  }

  /// \par Effects
  ///   Assigns the sequence to count elements copy constructed from value.
  ///
  /// \par Returns
  ///   A reference to *this.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and ilist.size(), plus MlogN, where M is
  ///   the difference of size() and ilist.size(), and N is the maximum of
  ///   size() and ilist.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  seq& operator=(std::initializer_list<T> ilist) {
    assign_range(ilist.begin(), ilist.end());
    return *this;
  }

  /// \par Effects
  ///   Assigns the sequence to count elements copy constructed from value.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and count, plus MlogN, where M is the
  ///   difference of size() and count, and N is the maximum of size() and
  ///   count.
  ///
  /// \par Iterator invalidation
  ///  Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void assign(size_type count, T const& value) { assign_count(count, value); }

  /// \par Effects
  ///   Assigns the sequence to the elements copy constructed from the range
  ///   [first, last).
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and the size of the range, plus MlogN,
  ///   where M is the difference of size() and the size of the range, and N is
  ///   the maximum of size() and the size of the range.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class InputIt,
            typename = typename std::iterator_traits<InputIt>::pointer>
  void assign(InputIt first, InputIt last) {
    assign_range(first, last);
  }

  /// \par Effects
  ///   Assigns the sequence to the elements of the specified initializer_list.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and ilist.size(), plus MlogN, where M is
  ///   the difference of size() and ilist.size(), and N is the maximum of
  ///   size() and ilist.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void assign(std::initializer_list<T> ilist) {
    assign_range(ilist.begin(), ilist.end());
  }

  /// \par Returns
  ///   A copy of the allocator for the sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  allocator_type get_allocator() const noexcept {
    return get_element_allocator();
  }

  /// \par Returns
  ///   A reference to the allocator for the sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  stored_allocator_type const& get_stored_allocator() const noexcept {
    return get_element_allocator();
  }

  /// \par Returns
  ///   A reference for the element located at the index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference at(size_type pos) {
    if (pos >= get_size()) throw std::out_of_range{"seq at() out of bounds"};
    auto it = find_index(pos);
    return static_traits::dereference(it);
  }

  /// \par Returns
  ///   A const_reference for the element located at the specified index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference at(size_type pos) const {
    if (pos >= get_size()) throw std::out_of_range{"seq at() out of bounds"};
    auto it = find_index(pos);
    return static_traits::dereference(it);
  }

  /// \par Returns
  ///   A reference for the element located at the specified index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference operator[](size_type pos) {
    auto it = find_index(pos);
    return static_traits::dereference(it);
  }

  /// \par Returns
  ///   A const_reference for the element located at the specified index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference operator[](size_type pos) const {
    auto it = find_index(pos);
    return static_traits::dereference(it);
  }

  /// \par Returns
  ///   A reference for the element located at the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference front() { return *begin(); }

  /// \par Returns
  ///   A const_reference for the element located at the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference front() const { return *begin(); }

  /// \par Returns
  ///   A reference for the element located at the index size() - 1.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference back() { return *penultimate(); }

  /// \par Returns
  ///   A const_reference for the element located at the index size() - 1.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference back() const { return *penultimate(); }

  /// \par Returns
  ///   An iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  iterator begin() noexcept { return find_first(); }

  /// \par Returns
  ///   A const_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator begin() const noexcept { return find_first(); }

  /// \par Returns
  ///   A const_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator cbegin() const noexcept { return find_first(); }

  /// \par Returns
  ///   An iterator to the index size() - 1
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  iterator penultimate() noexcept { return find_last(); }

  /// \par Returns
  ///   A const_iterator to the index size() - 1
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  const_iterator penultimate() const noexcept { return find_last(); }

  /// \par Returns
  ///   A const_iterator to the index size() - 1
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  const_iterator cpenultimate() const noexcept { return find_last(); }

  /// \par Returns
  ///   An iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  iterator end() noexcept { return find_end(); }

  /// \par Returns
  ///   A const_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator end() const noexcept { return find_end(); }

  /// \par Returns
  ///   A const_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator cend() const noexcept { return find_end(); }

  /// \par Returns
  ///   A reverse_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }

  /// \par Returns
  ///   A const_reverse_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  /// \par Returns
  ///   A const_reverse_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  /// \par Returns
  ///   A reverse_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }

  /// \par Returns
  ///   A const_reverse_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  /// \par Returns
  ///   A const_reverse_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  /// \par Returns
  ///   An iterator for the index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  ///
  /// \par Note
  ///   This member function is more efficient than calling begin() and
  ///   advancing.
  iterator nth(size_type pos) noexcept {
    if (pos >= size()) return find_end();
    return find_index(pos);
  }

  /// \par Returns
  ///   A const_iterator for the index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  ///
  /// \par Note
  ///   This member function is more efficient than calling begin() and
  ///   advancing.
  const_iterator nth(size_type pos) const noexcept {
    if (pos >= size()) return find_end();
    return find_index(pos);
  }

  /// \par Returns
  ///   The index of the specified iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  size_type index_of(iterator pos) noexcept { return pos.it_.pos; }

  /// \par Returns
  ///   The index of the specified const_iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  size_type index_of(const_iterator pos) const noexcept { return pos.it_.pos; }

  /// \par Returns
  ///   True if the sequence is empty. False otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  bool empty() const noexcept { return get_size() == 0; }

  /// \par Returns
  ///   The count of elements stored in the sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  size_type size() const noexcept { return get_size(); }

  /// \par Returns
  ///   The height of the tree.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \par Note
  ///   Non-standard extension.
  size_type height() const noexcept { return get_height(); }

  /// \par Returns
  ///   The maximum count of elements able to be stored in the sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  size_type max_size() const noexcept {
    return (std::numeric_limits<size_type>::max)();
  }

  /// \par Effects
  ///   Removes all elements from the sequence.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   No-throw.
  void clear() noexcept {
    purge();
    get_root() = nullptr;
    get_height() = 0;
    get_size() = 0;
  }

  /// \par Effects
  ///   Copy constructs an element at the specified position.
  ///
  /// \par Returns
  ///   An iterator to the inserted element.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  iterator insert(const_iterator pos, T const& value) {
    return emplace_single(pos.it_, value);
  }

  /// \par Effects
  ///   Move constructs an element at the specified position.
  ///
  /// \par Returns
  ///   An iterator to the inserted element.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  iterator insert(const_iterator pos, T&& value) {
    return emplace_single(pos.it_, std::move(value));
  }

  /// \par Effects
  ///   Copy constructs count elements at the specified position.
  ///
  /// \par Returns
  ///   An iterator to the first inserted element if count != 0. A copy of pos
  ///   otherwise.
  ///
  /// \par Complexity
  ///   MlogN, where M is count, and N is the maximum of size() and count.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  iterator insert(const_iterator pos, size_type count, T const& value) {
    return emplace_count(pos.it_, count, value);
  }

  /// \par Effects
  ///   Copy constructs all elements in the range [first, last) at the specified
  ///   position.
  ///
  /// \par Returns
  ///   An iterator to the first inserted element if first != last. A copy of
  ///   pos otherwise.
  ///
  /// \par Complexity
  ///   MlogN, where M is the size of the range, and N is the maximum of size()
  ///   and the size of the range.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class InputIt,
            typename = typename std::iterator_traits<InputIt>::pointer>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    return emplace_range(pos.it_, first, last);
  }

  /// \par Effects
  ///   Copy constructs all elements in the specified initializer_list at the
  ///   specified position.
  ///
  /// \par Returns
  ///   An iterator to the first inserted element if ilist.size() != 0. A copy
  ///   of pos otherwise.
  ///
  /// \par Complexity
  ///   MlogN, where M is ilist.size(), and N is the maximum of size() and
  ///   ilist.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
    return emplace_range(pos.it_, ilist.begin(), ilist.end());
  }

  /// \par Effects
  ///   Forward constructs an element at the specified position.
  ///
  /// \par Returns
  ///   An iterator to the inserted element.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return emplace_single(pos.it_, std::forward<Args>(args)...);
  }

  /// \par Effects
  ///   Remove the element at the specified position from the sequence.
  ///
  /// \par Returns
  ///   An iterator to the element following the removed element.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  iterator erase(const_iterator pos) { return erase_single(pos.it_); }

  /// \par Effects
  ///   Remove all elements in the range [first, last) from the sequence.
  ///
  /// \par Returns
  ///   An iterator to the element following the last removed element.
  ///
  /// \par Complexity
  ///   MlogN, where M is the size of the range, and N is size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  iterator erase(const_iterator first, const_iterator last) {
    return erase_range(first.it_, last.it_);
  }

  /// \par Effects
  ///   Copy constructs an element at end().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_back(T const& value) { emplace_back(value); }

  /// \par Effects
  ///   Move constructs an element at end().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_back(T&& value) { emplace_back(std::move(value)); }

  /// \par Effects
  ///   Forward constructs an element at end().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  template <class... Args>
  void emplace_back(Args&&... args) {
    emplace_single(find_end(), std::forward<Args>(args)...);
  }

  /// \par Effects
  ///   Removes the element at end() - 1.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void pop_back() { erase_single(find_last()); }

  /// \par Effects
  ///   Copy constructs an element at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_front(const T& value) { emplace_front(value); }

  /// \par Effects
  ///   Move constructs an element at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_front(T&& value) { emplace_front(std::move(value)); }

  /// \par Effects
  ///   Forward constructs an element at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  template <class... Args>
  void emplace_front(Args&&... args) {
    emplace_single(find_first(), std::forward<Args>(args)...);
  }

  /// \par Effects
  ///   Removes the element at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void pop_front() { erase_single(find_first()); }

  /// \par Effects
  ///   Resizes the seqeuence to the specified size, default constructing any
  ///   elements above the current size.
  ///
  /// \par Complexity
  ///   MlogN, where M is the difference of size() and count, and N is the max
  ///   of size() and count.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void resize(size_type count) { resize_count(count); }

  /// \par Effects
  ///   Resizes the seqeuence to the specified size, copy constructing any
  ///   elements from value above the current size.
  ///
  /// \par Complexity
  ///   MlogN, where M is the difference of size() and count, and N is the max
  ///   of size() and count.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void resize(size_type count, value_type const& value) {
    resize_count(count, value);
  }

  /// \par Effects
  ///   Swaps the contents *this with the specified sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Every iterator referring to an element in one container before the
  ///   swap shall refer to the same element in the other container after the
  ///   swap.
  ///
  /// \par Exception safety
  ///   No-throw if the allocator propagates on swap or the allocator doesn't
  ///   throw on swap. Strong otherwise.
  void swap(seq& other) noexcept(
      !element_traits::propagate_on_container_swap::value ||
      detail::is_nothrow_swappable<allocator_type>::value) {
    using std::swap;
    swap(get_root(), other.get_root());
    swap(get_height(), other.get_height());
    swap(get_size(), other.get_size());
    swap_allocator(other);
  }

  /// \par Effects
  ///   Transfers all elements in the sorted other into the sorted *this so that
  ///   all elements are in stable sorted order.
  ///
  /// \par Complexity
  ///   Linear in size() + other.size() if enough temporary memory is available.
  ///   NlogN, where N is the maximum of size() and other.size() otherwise.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void merge(seq& other) { merge(other, std::less<value_type>{}); }

  /// \par Effects
  ///   Transfers all elements in the sorted other into the sorted *this so that
  ///   all elements are in stable sorted order.
  ///
  /// \par Complexity
  ///   Linear in size() + other.size() if enough temporary memory is available.
  ///   NlogN, where N is the maximum of size() and other.size() otherwise.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void merge(seq&& other) { merge(other); }

  /// \par Effects
  ///   Transfers all elements in the sorted other into the sorted *this so that
  ///   all elements are in stable sorted order using the specified compare
  ///   functor.
  ///
  /// \par Complexity
  ///   Linear in size() + other.size() if enough temporary memory is available.
  ///   NlogN, where N is the maximum of size() and other.size() otherwise.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class Compare>
  void merge(seq& other, Compare comp) {
    auto sz = size();
    splice(end(), other, other.begin(), other.end());
    std::inplace_merge(begin(), nth(sz), end(), comp);
  }

  /// \par Effects
  ///   Transfers all elements in the sorted other into the sorted *this so that
  ///   all elements are in stable sorted order using the specified compare
  ///   functor.
  ///
  /// \par Complexity
  ///   Linear in size() + other.size() if enough temporary memory is available.
  ///   NlogN, where N is the maximum of size() and other.size() otherwise.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class Compare>
  void merge(seq&& other, Compare comp) {
    merge(other, comp);
  }

  /// \par Effects
  ///   Transfers all elements in other to the specified position.
  ///
  /// \par Complexity
  ///   MlogN, where M is other.size(), and N is the maximum of size(), and
  ///   other.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void splice(const_iterator pos, seq& other) {
    insert(pos, std::make_move_iterator(other.begin()),
           std::make_move_iterator(other.end()));
    other.clear();
  }

  /// \par Effects
  ///   Transfers all elements in other to the specified position.
  ///
  /// \par Complexity
  ///   MlogN, where M is other.size(), and N is the maximum of size(), and
  ///   other.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void splice(const_iterator pos, seq&& other) { splice(pos, other); }

  /// \par Effects
  ///   Transfers the element pointer to by it to the specified position.
  ///
  /// \par Complexity
  ///   Logarithmic in the maximum of size() and other.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void splice(const_iterator pos, seq& other, const_iterator it) {
    insert(pos, std::move(*it));
    other.erase(it);
  }

  /// \par Effects
  ///   Transfers the element pointer to by it to the specified position.
  ///
  /// \par Complexity
  ///   Logarithmic in the maximum of size() and other.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void splice(const_iterator pos, seq&& other, const_iterator it) {
    splice(pos, other, it);
  }

  /// \par Effects
  ///   Transfers all elements in the range [first, last) to the specified
  ///   position.
  ///
  /// \par Complexity
  ///   MlogN, where M is the size of the range, and N is the maximum of size(),
  ///   other.size(), and the size of the range.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void splice(const_iterator pos, seq& other, const_iterator first,
              const_iterator last) {
    insert(pos, std::make_move_iterator(iterator{first.it_}),
           std::make_move_iterator(iterator{last.it_}));
    other.erase(first, last);
  }

  /// \par Effects
  ///   Transfers all elements in the range [first, last) to the specified
  ///   position.
  ///
  /// \par Complexity
  ///   MlogN, where M is the size of the range, and N is the maximum of size(),
  ///   other.size(), and the size of the range.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators in *this and other.
  ///
  /// \par Exception safety
  ///   Basic.
  void splice(const_iterator pos, seq&& other, const_iterator first,
              const_iterator last) {
    splice(pos, other, first, last);
  }

  /// \par Effects
  ///   Removes all elements matching the specified value.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void remove(const T& value) {
    erase(std::remove(begin(), end(), value), end());
  }

  /// \par Effects
  ///   Removes all elements matching the specified predicate.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class UnaryPredicate>
  void remove_if(UnaryPredicate p) {
    erase(std::remove_if(begin(), end(), p), end());
  }

  /// \par Effects
  ///   Reverses the sequence.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated, but reflect the change in ordering.
  ///
  /// \par Exception safety
  ///   Basic.
  void reverse() { std::reverse(begin(), end()); }

  /// \par Effects
  ///   Removes all consecutive duplicate elements from the sequence.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void unique() { unique(std::equal_to<value_type>{}); }

  /// \par Effects
  ///   Removes all consecutive duplicate elements from the sequence using the
  ///   specified equality functor.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class BinaryPredicate>
  void unique(BinaryPredicate p) {
    erase(std::unique(begin(), end(), p), end());
  }

  /// \par Effects
  ///   Stable sorts the sequence.
  ///
  /// \par Complexity
  ///   NlogN, where N is size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated, but reflect the change in ordering.
  ///
  /// \par Exception safety
  ///   Basic.
  void sort() { sort(std::less<value_type>{}); }

  /// \par Effects
  ///   Stable sort the sequence using the specified compare functor.
  ///
  /// \par Complexity
  ///    NlogN, where N is size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated, but reflect the change in ordering.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class Compare>
  void sort(Compare comp) {
    std::stable_sort(begin(), end(), comp);
  }

  /// \par Returns
  ///   True if both sequences are of the same length and have each element in
  ///   both sequences are equal. False otherwise.
  ///
  /// \par Complexity
  ///    Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  friend bool operator==(seq const& lhs, seq const& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  /// \par Returns
  ///   False if both sequences are of the same length and have each element in
  ///   both sequences are equal. True otherwise.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  friend bool operator!=(seq const& lhs, seq const& rhs) {
    return !(lhs == rhs);
  }

  /// \par Returns
  ///   True if the first sequence is lexicographically less than the second.
  ///   False otherwise.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  friend bool operator<(seq const& lhs, seq const& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
  }

  /// \par Returns
  ///   True if the first sequence is equal or lexicographically less than the
  ///   second. False otherwise.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  friend bool operator<=(seq const& lhs, seq const& rhs) {
    return !(rhs < lhs);
  }

  /// \par Returns
  ///   True if the first sequence is lexicographically greater than the second.
  ///   False otherwise.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  friend bool operator>(seq const& lhs, seq const& rhs) { return rhs < lhs; }

  /// \par Returns
  ///   True if the first sequence is lexicographically greater or equal to the
  ///   second. False otherwise.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  friend bool operator>=(seq const& lhs, seq const& rhs) {
    return !(lhs < rhs);
  }

  /// \par Effects
  ///   Swaps the contents of the sequences.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Every iterator referring to an element in one container before the
  ///   swap shall refer to the same element in the other container after the
  ///   swap.
  ///
  /// \par Exception safety
  ///   No-throw if the allocator propagates on swap or the allocator doesn't
  ///   throw on swap. Strong otherwise.
  friend void swap(seq& a, seq& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};
}
}

#endif  // #ifndef BOOST_SEGMENTED_TREE_SEQ
