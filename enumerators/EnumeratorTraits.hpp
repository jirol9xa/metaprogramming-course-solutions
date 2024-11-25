#pragma once

#include <cstddef>
#include <cstdio>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace detail {
    template<typename E>
    concept ScopedEnum = requires {
        requires std::is_enum_v<E>;
        requires !std::is_convertible_v<E, std::underlying_type_t<E>>;
    };

    template <auto $>
    constexpr std::string_view GetPretty() {
        return {__PRETTY_FUNCTION__};
    }
} // detail namespace

template <class Enum, std::size_t MAXN = 512>
	requires std::is_enum_v<Enum>
struct EnumeratorTraits {

    struct  Meta{
        using EnumType = std::underlying_type_t<Enum>;
        static inline constexpr int MAX = []() {
            if constexpr (std::is_signed_v<EnumType>) {
                // |min| = |max| + 1 for signed types
                return std::numeric_limits<EnumType>::min() > -int(MAXN) ? -int(std::numeric_limits<EnumType>::min()) : MAXN + 1;
            } else {
                return std::numeric_limits<EnumType>::max() < MAXN ? std::numeric_limits<EnumType>::max() + 1 : MAXN + 1;
            }
        }();

        std::size_t Size = 0;
        std::pair<EnumType, std::string_view> Enumerators[2 * MAX + 1] = {};
        std::string_view PrettyStrings[2 * MAX + 1] = {};

        template <auto... Ids>
        constexpr void CollectPretties(std::integer_sequence<int, Ids...>) {
            if constexpr (std::is_signed_v<EnumType>) {
                ((PrettyStrings[Ids] = detail::GetPretty<static_cast<Enum>(Ids - MAX)>()), ...);
            } else {
                // Fill with invalid values for skip in future
                ((PrettyStrings[Ids] = std::string_view{"std::string_view detail::GetPretty() [$ = ("}), ...);
            }
            ((PrettyStrings[Ids + MAX] = detail::GetPretty<static_cast<Enum>(Ids)>()), ...);
        }

        constexpr Meta() {
            constexpr size_t offset = std::string_view{"std::string_view detail::GetPretty() [$ = "}.size();

            CollectPretties(std::make_integer_sequence<int, MAX>{});
            for (int i = -(int)MAX; i < (int) MAX; ++i) {
                auto name = PrettyStrings[i + MAX].substr(offset);
                if (name[0] == '(') {
                    continue;
                }

                name = name.substr(0, name.find_first_of("]"));

                if constexpr (!detail::ScopedEnum<Enum>) {
                    Enumerators[Size++] = {i, name};
                } else {
                    name = name.substr(name.find_last_of(":") + 1);
                    Enumerators[Size++] = {i, name};
                }
            }
        }
    };

    static constexpr inline Meta meta;

    static constexpr std::size_t size() noexcept {
        return meta.Size;
    }
    static constexpr Enum at(std::size_t i) noexcept {
        return static_cast<Enum>(meta.Enumerators[i].first);
    }
    static constexpr std::string_view nameAt(std::size_t i) noexcept {
        return meta.Enumerators[i].second;
    }
};
