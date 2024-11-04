#pragma once

#include <optional>
#include <tuple>


template <typename T>
struct Holder {
  std::optional<T> Value;

  Holder(int) : Value(std::nullopt) {}
  Holder(const std::optional<T>& value) : Value(value) {}

  friend Holder<T> operator+(const Holder<T>& first, const Holder<T>& second) {
    if (!first.Value && !second.Value) {
      return Holder<T>{std::nullopt};
    } else if (!first.Value) {
      return second.Value;
    } else {
      return first.Value;
    }
  }
};

template <class From, auto target>
struct Mapping {
  using FromType = From;
  static Holder<decltype(target)> GetTarget(const From* ptr) {
    if (!ptr) {
      return Holder<decltype(target)>(std::nullopt);
    } else {
      return std::optional{target};
    }
  }
};

template <class Base, class Target, class... Mappings>
struct PolymorphicMapper : Mappings...{
  using Mappings::GetTarget...;

  static std::optional<Target> map(const Base& object) {
    bool canCast = (dynamic_cast<const Mappings::FromType*>(&object) ||  ...);
    if (!canCast) {
      return {};
    }

    return (GetTarget(dynamic_cast<const Mappings::FromType*>(&object)) + ... + Holder<Target>(0)).Value;
  }
};
