#include "lib/assert.hpp"
#include <algorithm>
#include <compare>
#include <cstddef>
#include <span>
#include <concepts>
#include <cstdlib>
#include <vector>
#include <array>
#include <iterator>
#include <type_traits>
#include <iterator>

#include <iostream>

namespace detail {
  inline constexpr std::ptrdiff_t dynamic_stride = -1;

  template <std::size_t N = 0>
  struct Empty {
    Empty() = default;
    Empty(std::size_t) {}
  };
  struct Extent {
    std::size_t ExtentImpl;

    Extent(std::size_t extent = 0) : ExtentImpl(extent) {}
    Extent(const Extent&) = default;
  };
  struct Stride {
    std::ptrdiff_t StrideImpl;

    Stride(std::ptrdiff_t stride = 1) : StrideImpl(stride) {}
    Stride(const Stride&) = default;
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
    using element_type = U;
    using pointer = U*;
    using reference = U&;
    using iterator_category = std::contiguous_iterator_tag;

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
    constexpr pointer operator->() const { return data_; }
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
        return detail::Stride::StrideImpl;
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
  constexpr Slice(const Slice<T, extent, stride>&) = default;

  constexpr Slice(std::array<T, extent>& arr) : data_(arr.data()) {}

  template <std::size_t count>
  constexpr Slice(std::array<T, count>& arr, std::ptrdiff_t skip) : detail::Extent(count / skip + (count % skip != 0)), 
                                                                        std::conditional_t<stride == detail::dynamic_stride, detail::Stride, detail::Empty<1>>(skip),
                                                                        data_(arr.data()) {}

  template <std::contiguous_iterator It>
  constexpr Slice(It first, std::size_t count, std::ptrdiff_t skip = 1) : std::conditional_t<extent == std::dynamic_extent, detail::Extent, detail::Empty<0>>(count / skip + (count % skip != 0)),
                                                                          std::conditional_t<stride == detail::dynamic_stride, detail::Stride, detail::Empty<1>>(skip),
                                                                          data_(&*first)  {}
  template <typename U>
  constexpr Slice(std::vector<U>& vec, std::ptrdiff_t skip = 1) : Slice(vec.begin(), vec.size() / skip + (vec.size() % skip != 0), skip) {}


  constexpr pointer Data() const noexcept { return data_; }

  constexpr size_type Size() const {
    if constexpr (extent != std::dynamic_extent) {
      return extent;
    } else {
      return detail::Extent::ExtentImpl;
    }
  }

  constexpr difference_type Stride() const noexcept {
    if constexpr (stride != detail::dynamic_stride) {
      return stride;
    } else {
      return detail::Stride::StrideImpl;
    }
  }

  constexpr iterator begin() const noexcept { return SliceIter<T, stride>(data_, Stride()); }
  constexpr iterator end() const noexcept {
    difference_type strideDiff = 0;
    if constexpr (stride != detail::dynamic_stride) {
      strideDiff = stride;
    } else {
      strideDiff = detail::Stride::StrideImpl;
    }

    if constexpr (extent != std::dynamic_extent) {
      return SliceIter<T, stride>(data_ + extent * strideDiff, Stride());
    } else {
      return SliceIter<T, stride>(data_ + detail::Extent::ExtentImpl * strideDiff, Stride());
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
      MPC_VERIFYF(idx < detail::Extent::ExtentImpl, "index out of range in operator[]");
    }
    return begin()[idx];
  }

  template <typename U, std::size_t outer_extent = std::dynamic_extent, std::ptrdiff_t outer_stride = detail::dynamic_stride>
    requires (
              std::same_as<std::remove_cv_t<T>, std::remove_cv_t<U>> 
          && (outer_extent == std::dynamic_extent || outer_extent == extent)
          && (outer_stride == detail::dynamic_stride || outer_stride == stride)
    )
  constexpr operator Slice<U, outer_extent, outer_stride>() const {
    return Slice<U, outer_extent, outer_stride>(begin(), Size(), Stride());
  }

  constexpr Slice<T, std::dynamic_extent, stride> First(std::size_t count) const 
  {
    return Slice<T, std::dynamic_extent, stride>(begin(), count, stride);
  }

  template <std::size_t count>
  constexpr Slice<T, count, stride> First() const {
    if constexpr (extent != std::dynamic_extent) {
      static_assert(count < extent);
    } else {
      MPC_VERIFY(count < Size());
    }

    return Slice<T, count, stride>(begin(), count, stride);
  }

  constexpr Slice<T, std::dynamic_extent, stride> Last(std::size_t count) const {
    MPC_VERIFY(count < Size());
    return Slice<T, std::dynamic_extent, stride>(begin() + (Size() - count) * stride, count, stride);
  }

  template <std::size_t count>
  constexpr Slice<T, count, stride> Last() const {
    if constexpr (extent != std::dynamic_extent) {
      static_assert(count < extent);
    } else {
      MPC_VERIFY(count < Size());
    }

    return Slice<T, count, stride>(begin() + (Size() - count) * stride, count, stride);
  }

  constexpr Slice<T, std::dynamic_extent, stride> DropFirst(std::size_t count) const {
    return Last(Size() - count);
  }

  template <std::size_t count>
  constexpr Slice<T, extent - count, stride> DropFirst() const {
    return Last<extent - count>();
  }

  constexpr Slice<T, std::dynamic_extent, stride> DropLast(std::size_t count) const {
    return First(Size() - count);
  }

  template <std::size_t count>
  constexpr Slice<T, extent - count, stride> DropLast() const {
    return First<extent - count>();
  }

  constexpr Slice<T, std::dynamic_extent, detail::dynamic_stride> Skip(std::ptrdiff_t skip) const {
    return Slice<T, std::dynamic_extent, detail::dynamic_stride>(begin(), Size() * Stride(), skip * Stride());
  }

  template <std::ptrdiff_t skip>
  constexpr auto Skip() const {
    auto getNTTPParams = [] () {
      difference_type newStride = detail::dynamic_stride;
      size_type newExtent = std::dynamic_extent;
      if constexpr (stride != detail::dynamic_stride) {
        newStride = skip * stride;
        if constexpr (extent != std::dynamic_extent) {
          newExtent = extent / skip + (extent % skip != 0);
        }
      }

      return std::pair{newExtent, newStride};
    };

    constexpr std::pair params = getNTTPParams();
    return Slice<T, params.first, params.second>(begin(), Size() * Stride(), skip * Stride());
  }

  template <typename OtherT, std::size_t OtherExtent = std::dynamic_extent, std::ptrdiff_t OtherStride = detail::dynamic_stride>
    requires (std::same_as<std::remove_cv_t<T>, std::remove_cv_t<OtherT>>)
  constexpr bool operator==(const Slice<OtherT, OtherExtent, OtherStride>& other) const {
    if (Size() != other.Size()) {
      return false;
    }

    return std::equal(begin(), end(), other.begin());
  }

private:
  T* data_;
};

template <std::contiguous_iterator It>
Slice(It, std::size_t, std::ptrdiff_t) -> Slice<typename std::remove_reference_t<It>::value_type, std::dynamic_extent, detail::dynamic_stride>;

template <typename U>
Slice(const std::vector<U>&, std::ptrdiff_t = 1) -> Slice<U>;

template <typename U, std::size_t count>
Slice(const std::array<U, count>&, std::ptrdiff_t) -> Slice<U, std::dynamic_extent>;
