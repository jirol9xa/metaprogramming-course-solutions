#include "lib/assert.hpp"
#include <cstddef>
#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>
#include <type_traits>
#include <ranges>

namespace detail {
  inline constexpr std::ptrdiff_t dynamic_stride = -1;

  template <std::size_t N>
  struct Empty {
    Empty() = default;
    Empty(std::size_t) {}
  };
  struct Extent {
    std::size_t Extent = 0;
  };
  struct Stride {
    std::ptrdiff_t Stride = 0;
  };
} // detail

using detail::dynamic_stride;

template
  < class T
  , std::size_t extent = std::dynamic_extent
  , std::ptrdiff_t stride = 1
  >
class Slice : std::conditional_t<extent == std::dynamic_extent, detail::Extent, detail::Empty<0>>, std::conditional_t<stride == detail::dynamic_stride, detail::Stride, detail::Empty<1>> {
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

  constexpr pointer Data() const noexcept { return data_; }

  constexpr size_type Size() const {
    if constexpr (extent == std::dynamic_extent) {
      return detail::Extent::Extent;
    } else {
      return extent;
    }
  }

  constexpr difference_type Stride() const noexcept {
    if constexpr (stride != detail::dynamic_stride) {
      return stride;
    } else {
      return detail::Stride::Stride;
    }
  }

  constexpr iterator begin() const noexcept { return data_; }
  constexpr iterator end() const noexcept {
    difference_type strideDiff = 0;
    if constexpr (stride != detail::dynamic_stride) {
      strideDiff = stride;
    } else {
      strideDiff = detail::Stride::Stride;
    }

    if constexpr (extent != std::dynamic_extent) {
      return data_ + extent * strideDiff;
    } else {
      return data_ + detail::Extent::Extent * strideDiff;
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
    difference_type strideDiff = 0;
    if constexpr (stride != detail::dynamic_stride) {
      strideDiff = stride;
    } else {
      strideDiff = detail::Stride::Stride;
    }

    if constexpr (extent != std::dynamic_extent) {
      MPC_VERIFYF(idx < extent, "index out of range in operator[]");
    } else {
      MPC_VERIFYF(idx < detail::Extent::Extent, "index out of range in operator[]");
    }
    return data_[idx * strideDiff];
  }

  template<class U>
  Slice(U& container) : std::conditional_t<extent == std::dynamic_extent, detail::Extent, detail::Empty<0>>(std::ranges::size(container)), data_(std::ranges::data(container)) {}

  template <std::contiguous_iterator It>
  Slice(It first, std::size_t count, std::ptrdiff_t skip = 1) : std::conditional_t<extent == std::dynamic_extent, detail::Extent, detail::Empty<0>>(count),
                                                                std::conditional_t<stride == detail::dynamic_stride, detail::Stride, detail::Empty<1>>(skip),
                                                                data_(&*first)
  {
    if constexpr (extent != std::dynamic_extent) {
      MPC_VERIFYF(extent == count, "Template and func args should match");
    }
    if constexpr (stride != detail::dynamic_stride) {
      MPC_VERIFYF(stride == skip, "Template and func args should match");
    }
  }

  operator Slice<T, std::dynamic_extent, detail::dynamic_stride>() const {
    return Slice<T, std::dynamic_extent, detail::dynamic_stride>(data_, Size(), Stride());
  }

  // TODO: Move implicit convertion ops to base classes
  // operator Slice<T, extent, detail::dynamic_stride>() const {
  //   static_assert(extent != std::dynamic_extent, "Casting to Slice<T, extent> must be done with extent != std::dynamic_extent");
  //   return Slice<T, extent, detail::dynamic_stride>(data_, extent, Stride());
  // }

  // operator Slice<T, std::dynamic_extent, stride>() const {
  //   static_assert(stride != detail::dynamic_stride, "Casting to Slice<T, std::dynamic_extent, stride> must be done with stride != dynamic_stride");
  //   return Slice<T, std::dynamic_extent, stride>(data_, Size(), stride);
  // }

  // TODO: implicit convertion from Slice<T> -> Slice<typename std::remove_const_t<T>>

  Slice<T, std::dynamic_extent, stride> First(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>(data_, count, stride);
  }

  template <std::size_t count>
  Slice<T, count, stride> First() const {
    if constexpr (extent != std::dynamic_extent) {
      static_assert(count < extent);
    } else {
      MPC_VERIFY(count < Size());
    }

    return Slice<T, count, stride>(data_, count, stride);
  }

  Slice<T, std::dynamic_extent, stride> Last(std::size_t count) const {
    MPC_VERIFY(count < Size());
    return Slice<T, std::dynamic_extent, stride>(data_ + (Size() - count) * stride, count, stride);
  }

  template <std::size_t count>
  Slice<T, count, stride> Last() const {
    if constexpr (extent != std::dynamic_extent) {
      static_assert(count < extent);
    } else {
      MPC_VERIFY(count < Size());
    }

    return Slice<T, count, stride>(data_ + (Size() - count) * stride, count, stride);
  }

  Slice<T, std::dynamic_extent, stride> DropFirst(std::size_t count) const {
    return Last(Size() - count);
  }

  template <std::size_t count>
  Slice<T, extent - count, stride> DropFirst() const {
    static_assert(extent != std::dynamic_extent, "Can not call NTTP parametrized method on not NTTP parametrizes object");
    return Last<extent - count>();
  }

  Slice<T, std::dynamic_extent, stride> DropLast(std::size_t count) const {
    return First(Size() - count);
  }

  template <std::size_t count>
  Slice<T, extent - count, stride> DropLast() const {
    static_assert(extent != std::dynamic_extent, "Can not call NTTP parametrized method on not NTTP parametrizes object");
    return First<extent - count>();
  }

  Slice<T, std::dynamic_extent, detail::dynamic_stride> Skip(std::ptrdiff_t skip) const {
    return Slice<T, std::dynamic_extent, detail::dynamic_stride>(data_, Size() * Stride() / skip, skip);
  }

  template <std::ptrdiff_t skip>
  Slice<T, std::dynamic_extent, skip> Skip() const {
    return Slice<T, std::dynamic_extent, skip>(data_, Size() * Stride() / skip, skip);
  }

private:
  T* data_;
};

template <std::contiguous_iterator It>
Slice(It, std::size_t, std::ptrdiff_t) -> Slice<typename std::remove_reference_t<It>::value_type>;

template <class U>
Slice(U&) -> Slice<typename std::remove_reference_t<U>::value_type>;