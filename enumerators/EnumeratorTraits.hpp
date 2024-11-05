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
        constexpr void FillMeta() {
            if (!IsFilled) {
                FillMetaImpl();
            }
        }
        std::size_t Size = 0;
        std::pair<std::underlying_type_t<MetaEnum>, std::string_view> Enumerators[1025] = {};
    
    private:
        template <auto Value>
        consteval std::string_view GetPrettyFunc() {
            return {__PRETTY_FUNCTION__};
        }

        template <auto Start, auto End, class F>
        constexpr void for_loop(F&& f) {
            if constexpr (Start < End) {
                f(std::integral_constant<decltype(Start), Start>());
                for_loop<Start + 1, End>(f);
            }
        }

        consteval void FillMetaImpl() {
            if constexpr (std::is_signed_v<EnumType>) {
                for_loop<static_cast<EnumType>(-MetaMAXN), static_cast<EnumType>(MetaMAXN)>([&](auto I) {
                    //std::string_view pretty = GetPrettyFunc<static_cast<MetaEnum>(static_cast<EnumType>(I))>();
                    // constexpr std::string_view substrToFind = "Value = ";
                    // auto name = pretty.substr(pretty.find(substrToFind) + substrToFind.size());
                    // name = name.substr(0, name.find_first_of(";"));

                    // if (name[0] == '(') {
                    //     return;
                    // }

                    // if constexpr (!detail::ScopedEnum<Enum>) {
                    //     Enumerators[Size] = {I, name};
                    // } else {
                    //     name = name.substr(name.find_last_of(":"));
                    //     Enumerators[Size] = {I, name};
                    // }

                    ++Size;
                });
            } else {
                for_loop<static_cast<EnumType>(0), static_cast<EnumType>(MetaMAXN)>([&](auto I) {
                    // std::string_view pretty = GetPrettyFunc<static_cast<MetaEnum>(static_cast<EnumType>(I))>();
                    // constexpr std::string_view substrToFind = "Value = ";
                    // auto name = pretty.substr(pretty.find(substrToFind) + substrToFind.size());
                    // name = name.substr(0, name.find_first_of(";"));

                    // if (name[0] == '(') {
                    //     return;
                    // }

                    // if constexpr (!detail::ScopedEnum<Enum>) {
                    //     Enumerators[Size] = {(EnumType)I, name};
                    // } else {
                    //     name = name.substr(name.find_last_of(":"));
                    //     Enumerators[Size] = {(EnumType)I, name};
                    // }

                    ++Size;
                });
            }
        }

        bool IsFilled = false;
    };

    static constexpr std::size_t size() noexcept {
        Meta<Enum, MAXN> meta;
        meta.FillMeta();
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
