#pragma once

namespace detail {
    
} // namespace detail

namespace type_tuples
{

template<class... Ts>
struct TTuple {};

template<class TT>
concept TypeTuple = requires(TT t) { []<class... Ts>(TTuple<Ts...>){}(t); };

template <class T1, class T2>
struct TupleConcat;

template <class... Ts1, class... Ts2>
struct TupleConcat<TTuple<Ts1...>, TTuple<Ts2...>> {
    using type = TTuple<Ts1..., Ts2...>;
};

template <class T, class... Ts>
struct TakeFirstImpl {
    using type = T;
    using others = TTuple<Ts...>;
};

template <typename T>
struct TakeFirstImpl<T> {
    using type = T;
};

} // namespace type_tuples
