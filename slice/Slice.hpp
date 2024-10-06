#include "lib/assert.hpp"
#include <algorithm>
#include <compare>
#include <cstddef>
#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>
#include <type_traits>
#include <iterator>

namespace detail {
  inline constexpr std::ptrdiff_t dynamic_stride = -1;

  template <std::size_t N = 0>
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

  template <typename U, std::ptrdiff_t iter_stride = detail::dynamic_stride>
  class SliceIter : std::conditional_t<iter_stride == detail::dynamic_stride, detail::Stride, detail::Empty<0>> {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = U;
    using pointer = U*;
    using reference = U&;

    constexpr SliceIter() = default;
    constexpr explicit SliceIter(U* data, std::ptrdiff_t itStride) : 
                              std::conditional_t<iter_stride == detail::dynamic_stride, detail::Stride, detail::Empty<0>>(itStride), data_(data) {}

    constexpr SliceIter<U, iter_stride> operator++(int) {
      auto tmp(*this);
      data_ += Stride();
      return tmp;
    }
    constexpr auto& operator++() {
      data_ += Stride();
      return *this;
    }
    constexpr auto operator--(int) {
      auto tmp(*this);
      data_ -= Stride();
      return tmp;
    }
    constexpr auto& operator--() {
      data_ -= Stride();
      return *this;
    }

    constexpr auto& operator+=(difference_type offset) {
      data_ += offset * Stride();
      return *this;
    }
    constexpr auto operator+(difference_type offset) const {
      auto tmp(*this);
      tmp += offset;
      return tmp;
    }
    constexpr auto& operator-=(difference_type offset) {
      data_ -= offset * Stride();
      return *this;
    }
    constexpr auto operator-(difference_type offset) const {
      auto tmp(*this);
      tmp -= offset;
      return tmp;
    }

    friend constexpr auto operator+(difference_type offset, const SliceIter<U, iter_stride>& iter) {
      return iter + offset;
    }
    friend constexpr auto operator-(difference_type offset, const SliceIter<U, iter_stride>& iter) {
      return iter - offset;
    }

    constexpr difference_type operator-(const SliceIter<U, iter_stride>& other) const {
      return (data_ - other.data_) / Stride();
    }

    constexpr reference operator[](difference_type idx) const {
      return data_[idx * Stride()];
    }

    constexpr reference operator*() const { return *data_; }
    bool operator==(const SliceIter<U, iter_stride>& other) const { return data_ == other.data_; }
    bool operator>(const SliceIter<U, iter_stride>& other) const { return data_ > other.data_; }
    bool operator>=(const SliceIter<U, iter_stride>& other) const { return data_ >= other.data_; }
    bool operator<(const SliceIter<U, iter_stride>& other) const { return data_ < other.data_; }
    bool operator<=(const SliceIter<U, iter_stride>& other) const { return data_ <= other.data_; }

  private:
    std::ptrdiff_t Stride() const {
      if constexpr (iter_stride != detail::dynamic_stride) {
        return iter_stride;
      } else {
        return detail::Stride::Stride;
      }
    }

    U* data_; 
  };

  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = SliceIter<T, stride>;
  using const_iterator = SliceIter<const T, stride>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  constexpr Slice() = default;

  constexpr explicit Slice(const std::array<T, extent> &arr, std::ptrdiff_t skip = 1) : std::conditional_t<stride == detail::dynamic_stride, detail::Stride, detail::Empty<1>>(skip), data_(arr.data()) {}

  template<class U>
  constexpr Slice(U& container) : std::conditional_t<extent == std::dynamic_extent, detail::Extent, detail::Empty<0>>(std::size(container)), data_(std::data(container)) {}

  template <std::contiguous_iterator It>
  constexpr Slice(It first, std::size_t count, std::ptrdiff_t skip = 1) : std::conditional_t<extent == std::dynamic_extent, detail::Extent, detail::Empty<0>>(count),
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

  constexpr pointer Data() const noexcept { return data_; }

  constexpr size_type Size() const {
    if constexpr (extent != std::dynamic_extent) {
      return extent;
    } else {
      return detail::Extent::Extent;
    }
  }

  constexpr difference_type Stride() const noexcept {
    if constexpr (stride != detail::dynamic_stride) {
      return stride;
    } else {
      return detail::Stride::Stride;
    }
  }

  constexpr iterator begin() const noexcept { return SliceIter<T, stride>(data_, Size()); }
  constexpr iterator end() const noexcept {
    difference_type strideDiff = 0;
    if constexpr (stride != detail::dynamic_stride) {
      strideDiff = stride;
    } else {
      strideDiff = detail::Stride::Stride;
    }

    if constexpr (extent != std::dynamic_extent) {
      return SliceIter<T, stride>(data_ + extent * strideDiff, Size());
    } else {
      return SliceIter<T, stride>(data_ + detail::Extent::Extent * strideDiff, Size());
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

  template <typename U, std::size_t outer_extent = std::dynamic_extent, std::ptrdiff_t outer_stride = detail::dynamic_stride>
    requires (
              std::same_as<std::remove_cv_t<T>, std::remove_cv_t<U>> 
          && (outer_extent == std::dynamic_extent || outer_extent == extent)
          && (outer_stride == detail::dynamic_stride || outer_stride == stride)
    )
  constexpr operator Slice<U, outer_extent, outer_stride>() const {
    return Slice<U, outer_extent, outer_stride>(data_, Size(), Stride());
  }

  constexpr Slice<T, std::dynamic_extent, stride> First(std::size_t count) const 
  {
    return Slice<T, std::dynamic_extent, stride>(data_, count, stride);
  }

  template <std::size_t count>
  constexpr Slice<T, count, stride> First() const {
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
    return Last<extent - count>();
  }

  Slice<T, std::dynamic_extent, stride> DropLast(std::size_t count) const {
    return First(Size() - count);
  }

  template <std::size_t count>
  Slice<T, extent - count, stride> DropLast() const {
    return First<extent - count>();
  }

  Slice<T, std::dynamic_extent, detail::dynamic_stride> Skip(std::ptrdiff_t skip) const {
    return Slice<T, std::dynamic_extent, detail::dynamic_stride>(data_, Size() / skip, skip * Stride());
  }

  template <std::ptrdiff_t skip>
  Slice<T, std::dynamic_extent, skip * stride * (stride != detail::dynamic_stride) + detail::dynamic_stride * (stride == detail::dynamic_stride)> Skip() const {
    constexpr auto newStride = skip * stride * (stride != detail::dynamic_stride) + detail::dynamic_stride * (stride == detail::dynamic_stride);
    return Slice<T, std::dynamic_extent, newStride>(data_, Size() / skip, skip * Stride());
  }

  // TODO: MB should pass const Slice<T>& ???
  constexpr bool operator==(const Slice<T, extent, stride>& other) const {
    return std::equal(other.begin(), other.end(), begin());
  }

private:
  T* data_;
};

template <std::contiguous_iterator It>
Slice(It, std::size_t, std::ptrdiff_t) -> Slice<typename std::remove_reference_t<It>::value_type>;

template <class U>
Slice(U& obj) -> Slice<typename std::remove_reference_t<U>::value_type>;

// template <typename T, std::size_t size>
// Slice(const std::array<T, size>&, std::ptrdiff_t) -> Slice<T, size>;
