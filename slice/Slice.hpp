#include <cstddef>
#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>
#include <type_traits>


inline constexpr std::ptrdiff_t dynamic_stride = -1;

namespace detail {
  struct Empty {
    Empty() = default;
    Empty(std::size_t) {}
  };
  struct Extent {
    std::size_t Extent = 0;
  };
  struct Stride {
    std::size_t Stride = 0;
  };
} // detail

template
  < class T
  , std::size_t extent = std::dynamic_extent
  , std::ptrdiff_t stride = 1
  >
class Slice : std::conditional_t<extent != std::dynamic_extent, typename Iftrue, typename Iffalse>{
public:
  template<class U>
  Slice(U& container);

  template <std::contiguous_iterator It>
  Slice(It first, std::size_t count, std::ptrdiff_t skip);

  // Data, Size, Stride, begin, end, casts, etc...

  Slice<T, std::dynamic_extent, stride>
    First(std::size_t count) const;

  template <std::size_t count>
  Slice<T, /*?*/, stride>
    First() const;

  Slice<T, std::dynamic_extent, stride>
    Last(std::size_t count) const;

  template <std::size_t count>
  Slice<T, /*?*/, stride>
    Last() const;

  Slice<T, std::dynamic_extent, stride>
    DropFirst(std::size_t count) const;

  template <std::size_t count>
  Slice<T, /*?*/, stride>
    DropFirst() const;

  Slice<T, std::dynamic_extent, stride>
    DropLast(std::size_t count) const;

  template <std::size_t count>
  Slice<T, /*?*/, stride>
    DropLast() const;

  Slice<T, /*?*/, /*?*/>
    Skip(std::ptrdiff_t skip) const;

  template <std::ptrdiff_t skip>
  Slice<T, /*?*/, /*?*/>
    Skip() const;

private:
  T* data_;
  // std::size_t extent_; ?
  // std::ptrdiff_t stride_; ?
};
