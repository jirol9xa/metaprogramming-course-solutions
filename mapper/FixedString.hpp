#pragma once

#include <cstddef>
#include <iterator>
#include <string_view>
#include <array>
#include <algorithm>

template<size_t max_length>
struct FixedString {
  constexpr FixedString(const char* string, size_t length) : length(length){
    for (size_t i = 0; i < length; ++i) {
      Storage[i] = string[i];
    }
    Storage[length] = '\0';
  }
  constexpr operator std::string_view() const {
    return {Storage, length};
  }

  constexpr FixedString(const FixedString&) = default;

  size_t length;
  char Storage[max_length] = {};
};

constexpr inline FixedString<256> operator ""_cstr (const char* string, size_t length) {
  return {string, length};
}
