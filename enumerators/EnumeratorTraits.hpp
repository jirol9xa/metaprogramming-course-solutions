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
    public:
        constexpr void FillMeta() {
            if (!IsFilled) {
                FillMetaImpl();
            }
        }
        std::size_t Size = 0;
        std::pair<std::underlying_type_t<MetaEnum>, std::string_view> Enumerators[1025] = {};
    
    private:
        using EnumType = std::underlying_type_t<MetaEnum>;
        
        template <Enum Value>
        constexpr std::string_view GetPrettyFunc() {
            return {__PRETTY_FUNCTION__};
        }

        constexpr void FillMetaImpl() {
            for (EnumType i = std::numeric_limits<EnumType>::min(); i < MetaMAXN; ++i) {
                std::string_view pretty = GetPrettyFunc<static_cast<MetaEnum>(i)>();
                constexpr std::string_view substrToFind = "Value = ";
                auto name = pretty.substr(pretty.find(substrToFind) + substrToFind.size());
                name = name.substr(0, name.find_first_of(";"));

                if (name[0] == '(') {
                    return;
                }

                if constexpr (!detail::ScopedEnum<Enum>) {
                    Enumerators[Size] = {i, name};
                } else {
                    name = name.substr(name.find_last_of(":"));
                    Enumerators[Size] = {i, name};
                }

                ++Size;
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
