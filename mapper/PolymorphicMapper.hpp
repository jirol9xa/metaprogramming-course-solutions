#pragma once

#include <optional>


template <class From, auto target>
struct Mapping {
  auto GetTarget(const Mapping&) {
    return target;
  }
};

template <class Base, class Target, class... Mappings>
struct PolymorphicMapper : Mappings... {
  using Mappings::GetTarget...;
  static std::optional<Target> map(const Base& object) {
    if constexpr (requires {GetTarget(object);}) {
      return GetTarget(object);
    } else {
      return {};
    }
  }
};
