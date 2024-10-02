#include "lib/assert.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <concepts>
#include <cstdlib>
#include <iterator>
#include <type_traits>
#include <ranges>

struct Empty {
  Empty() = default;
  Empty(std::size_t) {}
};

struct Extent {
  std::size_t Extent = 0;
};

template
  < class T
  , std::size_t extent = std::dynamic_extent
  >
class Span : private std::conditional_t<extent == std::dynamic_extent, Extent, Empty>{
public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using reverse_iterator = std::reverse_iterator<T*>;

  constexpr Span() = default;
  constexpr Span(const Span&) = default;

  template <std::contiguous_iterator It>
  explicit (extent != std::dynamic_extent)
  constexpr Span(It first, std::size_t count) : std::conditional_t<extent == std::dynamic_extent, Extent, Empty>(count), data_(&*first) {}

  constexpr Span(std::array<T, extent>& arr) : data_(arr.data()) {}

  template <std::contiguous_iterator It>
  explicit (extent != std::dynamic_extent)
  constexpr Span(It first, It second) : std::conditional_t<extent == std::dynamic_extent, Extent, Empty>(second - first), data_(&*first) {}

  template <class Range> 
  requires std::ranges::contiguous_range<Range> && std::ranges::sized_range<Range>
  explicit (extent != std::dynamic_extent)
  constexpr Span(Range&& range) : std::conditional_t<extent == std::dynamic_extent, Extent, Empty>(std::ranges::size(range)), data_(std::ranges::data(range)) {
    if constexpr (extent != std::dynamic_extent) {
      MPC_VERIFYF(extent == std::ranges::size(range), "Range size should be equal to extent");
    }
  }

  constexpr size_type Size() const {
    if constexpr (extent == std::dynamic_extent) {
      return Extent::Extent;
    } else {
      return extent;
    }
  }

  constexpr iterator begin() const noexcept { return data_; }
  constexpr iterator end() const noexcept {
    if constexpr (extent != std::dynamic_extent) {
      return data_ + extent;
    } else {
      return data_ + Extent::Extent;
    }
  }

  constexpr reverse_iterator rbegin() const noexcept { 
    return std::make_reverse_iterator(end());
  }
  constexpr reverse_iterator rend() const noexcept {
    return std::make_reverse_iterator(begin());
  }

  constexpr reference Front() const {
    MPC_VERIFYF(Size() != 0, "Can not access front element of empty span");
    return *begin();
  }
  constexpr reference Back() const {
    MPC_VERIFYF(Size() != 0, "Can not access front element of empty span");
    return *rbegin();
  }

  constexpr reference operator[](size_type idx) const { 
    if constexpr (extent != std::dynamic_extent) {
      MPC_VERIFYF(idx < extent, "index out of range in operator[]");
    } else {
      MPC_VERIFYF(idx < Extent::Extent, "index out of range in operator[]");
    }
    return data_[idx];
  }

  template <std::size_t Count>
  constexpr Span<T, Count> First() const {
    if constexpr (extent != std::dynamic_extent) {
      static_assert(extent > Count, "Length of subspan should be less than length of parent span");
    } else {
      MPC_VERIFYF(Extent::Extent > Count, "Length of subspan should be less than length of parent span");
    }
    return Span<T, Count>(begin(), Count);
  }
  constexpr Span<T> First(size_type count) const {
    if constexpr (extent != std::dynamic_extent) {
      MPC_VERIFYF(extent > count, "Length of subspan should be less than length of parent span");
    } else {
      MPC_VERIFYF(Extent::Extent > count, "Length of subspan should be less than length of parent span");
    }
    return Span<T>(begin(), count);
  }
  template <std::size_t Count>
  constexpr Span<T, Count> Last() const {
    std::size_t offset = 0;
    if constexpr (extent != std::dynamic_extent) {
      static_assert(extent > Count, "Length of subspan should be less than length of parent span");
      offset = extent - Count;
    } else {
      MPC_VERIFYF(Extent::Extent > Count, "Length of subspan should be less than length of parent span");
      offset = Extent::Extent - Count;
    }

    return Span<T, Count>(begin() + offset, Count);
  }
  constexpr Span<T> Last(size_type count) const {
    std::size_t offset = 0;
    if constexpr (extent != std::dynamic_extent) {
      MPC_VERIFYF(extent > count, "Length of subspan should be less than length of parent span");
      offset = extent - count;
    } else {
      MPC_VERIFYF(Extent::Extent > count, "Length of subspan should be less than length of parent span");
      offset = Extent::Extent - count;
    }
    return Span<T>(begin() + offset, count);
  }

  constexpr pointer Data() const noexcept { return data_; }

private:
  T* data_ = nullptr;
};

template <std::contiguous_iterator It>
Span(It, std::size_t) -> Span<typename std::remove_reference_t<It>::value_type>; 

template <std::contiguous_iterator It>
Span(It, It) -> Span<typename std::remove_reference_t<It>::value_type>;

template <class Range>
Span(Range&&) -> Span<typename std::remove_reference_t<Range>::value_type>;
