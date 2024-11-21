#pragma once

#include <cstddef>
#include <cstdio>
#include <string_view>
#include <type_traits>
#include <utility>

namespace detail {
    template<typename E>
    concept ScopedEnum = requires {
        requires std::is_enum_v<E>;
        requires !std::is_convertible_v<E, std::underlying_type_t<E>>;
    };
} // detail namespace

template <class Enum, std::size_t MAXN = 512>
	requires std::is_enum_v<Enum>
struct EnumeratorTraits {

    template <class MetaEnum, std::size_t MetaMAXN>
    class Meta{
        using EnumType = std::underlying_type_t<MetaEnum>;

    public:
        std::size_t Size = 0;
        std::pair<EnumType, std::string_view> Enumerators[1025] = {};
        std::string_view PrettyStrings[1025] = {};
    
        template <auto Value>
        constexpr std::string_view GetPretty() {
            return {__PRETTY_FUNCTION__};
        }
    
        template <auto... Ids>
        constexpr void CollectPretties(std::integer_sequence<int, Ids...>) {
            ((PrettyStrings[Ids] = GetPretty<static_cast<MetaEnum>(Ids - MetaMAXN)>()), ...);
        }

        constexpr void FillMetaImpl() {
            if constexpr (std::is_signed<EnumType>()) {
                CollectPretties(std::make_integer_sequence<int, 2 * MetaMAXN + 1>{});

                for (int i = -(int)MetaMAXN; i <= (int) MetaMAXN; ++i) {
                    auto pretty = PrettyStrings[i + MetaMAXN];
                    constexpr std::string_view substrToFind = "Value = ";
                    auto name = pretty.substr(pretty.find(substrToFind) + substrToFind.size());
                    // for clang
                    // name = name.substr(0, name.find_first_of("]"));

                    // for gcc
                    name = name.substr(0, name.find_first_of(";"));

                    if (name[0] == '(') {
                        continue;
                    }

                    if constexpr (!detail::ScopedEnum<Enum>) {
                        Enumerators[Size++] = {i, name};
                    } else {
                        name = name.substr(name.find_last_of(":") + 1);
                        Enumerators[Size++] = {i, name};
                    }
                }
            } else {
                //for_loop<0, static_cast<EnumType>(MetaMAXN)>();
            }
        }

        bool IsFilled = false;
    };

    static constexpr std::size_t size() noexcept {
        Meta<Enum, MAXN> meta;
        meta.FillMetaImpl();
        return meta.Size;
    }
    static constexpr Enum at(std::size_t i) noexcept {
        Meta<Enum, MAXN> meta;
        meta.FillMetaImpl();
        return static_cast<Enum>(meta.Enumerators[i].first);
    }
    static constexpr std::string_view nameAt(std::size_t i) noexcept {
        Meta<Enum, MAXN> meta;
        meta.FillMetaImpl();
        return meta.Enumerators[i].second;
    }
};
