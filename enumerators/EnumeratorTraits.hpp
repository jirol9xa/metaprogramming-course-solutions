#pragma once

#include <cstddef>
#include <cstdio>
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

        std::size_t Size = 0;
        std::pair<EnumType, std::string_view> Enumerators[1025] = {};
        std::string_view PrettyStrings[1025] = {};

        template <auto... Ids>
        constexpr void CollectPretties(std::integer_sequence<int, Ids...>) {
            ((PrettyStrings[Ids] = detail::GetPretty<static_cast<Enum>(Ids - MAXN)>()), ...);
        }

        constexpr Meta() {
            constexpr size_t offset = std::string_view{"std::string_view detail::GetPretty() [$ = "}.size();

            CollectPretties(std::make_integer_sequence<int, 2 * MAXN + 1>{});
            for (int i = -(int)MAXN; i <= (int) MAXN; ++i) {
                auto name = PrettyStrings[i + MAXN].substr(offset);
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
