#pragma once

#include <cstddef>
#include <cstdio>
#include <string_view>
#include <type_traits>
#include <cstdint>
#include <limits>

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
        EnumType EnumeratorValues[1025] = {};
        std::string_view EnumeratorsNames[1025] = {};
    
        template <auto Start, auto End>
        constexpr inline void for_loop() {
            if constexpr (Start <= End) {
                std::string_view pretty = __PRETTY_FUNCTION__;
                constexpr std::string_view substrToFind = "Start = ";
                auto name = pretty.substr(pretty.find(substrToFind) + substrToFind.size());
                name = name.substr(0, name.find_first_of(";"));

                if (name[0] == '(') {
                    return;
                }

                if constexpr (!detail::ScopedEnum<Enum>) {
                    EnumeratorValues[Size] = Start;
                    EnumeratorsNames[Size] = name;
                } else {
                    name = name.substr(name.find_last_of(":"));
                    EnumeratorValues[Size] = Start;
                    EnumeratorsNames[Size] = name;
                }

                ++Size;
                
                for_loop<Start + 1, End>();
            }
        }

        consteval inline void FillMetaImpl() {
            if constexpr (std::is_signed<EnumType>()) {
                for_loop<static_cast<EnumType>(-MetaMAXN), static_cast<EnumType>(MetaMAXN)>();
            } else {
                for_loop<0, static_cast<EnumType>(MetaMAXN)>();
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
        meta.FillMeta();
        return static_cast<Enum>(meta.Enumerators[i].first);
    }
    static constexpr std::string_view nameAt(std::size_t i) noexcept {
        Meta<Enum, MAXN> meta;
        meta.FillMeta();
        return meta.Enumerators[i].second;
    }
};
