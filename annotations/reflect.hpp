#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

template <class...>
class Annotate {
    using IsAnnotation = std::true_type;
};

namespace detail {
    template <typename T, std::size_t N>
    struct Tag {
        constexpr friend auto loophole(Tag<T, N>);
    };

    template <typename T, std::size_t N, class F>
    struct LoopholeSet {
        constexpr friend auto loophole(Tag<T, N>) { return F{}; }
    };

    template <typename T, std::size_t I>
    struct LoopholeGet {
        using type = decltype(loophole(Tag<T, I>{}));
    };

    template <typename T, std::size_t I>
    struct LoopholeUbiq {
        template <typename Type>
        constexpr operator Type() const noexcept{
            [[maybe_unused]] LoopholeSet<T, I, Type> set{};
            return {};
        }
    };

    template <class T, typename... Ts>
    concept AggregateConstructibe = requires (Ts... Args ) { T{Args...}; };

    template <class T, std::size_t AnnotCnt, std::size_t... Ids>
    constexpr std::size_t IterateTypes(std::index_sequence<Ids...>) {
        return sizeof...(Ids) - 1 - AnnotCnt;
    }

    template <class T, std::size_t AnnotCnt, std::size_t... Ids>
        requires AggregateConstructibe<T, LoopholeUbiq<T, Ids>...>
    constexpr std::size_t IterateTypes(std::index_sequence<Ids...>) {
        if constexpr (sizeof...(Ids) > 0) {
            if constexpr ((requires {{LoopholeGet<T, sizeof...(Ids) - 1>::type::IsAnnotation};})) {
                return IterateTypes<T, AnnotCnt + 1>(std::make_index_sequence<sizeof...(Ids) + 1>());
            }
        }
        return IterateTypes<T, AnnotCnt>(std::make_index_sequence<sizeof...(Ids) + 1>());
    }

} // namespace detail

template <typename T, std::size_t I>
struct FieldDescriptor {
    using Type = detail::LoopholeGet<T, I>::type;
    using Annotations = Annotate</* ... */>;

    // template <template <class...> class AnnotationTemplate>
    // static constexpr bool has_annotation_template = /* ... */;
    
    // template <class Annotation>
    // static constexpr bool has_annotation_class = /* ... */;

    // template <template <class...> class AnnotationTemplate>
    // using FindAnnotation = /* ... */;
};

template <class T>
struct Describe {
    static constexpr std::size_t num_fields = detail::IterateTypes<T, 0>({});
    
    template <std::size_t I>
    using Field = FieldDescriptor<T, I>;
};
