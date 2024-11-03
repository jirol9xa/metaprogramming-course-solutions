#pragma once

#include <concepts>

#include <cstddef>
#include <type_tuples.hpp>


namespace type_lists
{

template<class TL>
concept TypeSequence =
    requires {
        typename TL::Head;
        typename TL::Tail;
    };

struct Nil {};

template<class TL>
concept Empty = std::derived_from<TL, Nil>;

template<class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;


////////////////////////////////////////////////////////////////////////////////


template <typename H, typename T> requires TypeList<T>
struct Cons {
    using Head = H;
    using Tail = T;
};


////////////////////////////////////////////////////////////////////////////////


template <typename T>
struct Repeat {
    using Head = T;
    using Tail = Repeat<T>;
};

////////////////////////////////////////////////////////////////////////////////

template <TypeList TL>
struct ToTupleImpl {
    using type = type_tuples::TupleConcat
                                        <
                                            type_tuples::TTuple<typename TL::Head>,
                                            typename ToTupleImpl<typename TL::Tail>::type
                                        >::type;
};

template <>
struct ToTupleImpl<Nil> {
    using type = type_tuples::TTuple<>;
};

template <TypeList TL>
using ToTuple = ToTupleImpl<TL>::type;


////////////////////////////////////////////////////////////////////////////////


template <class T>
struct FromTupleImpl;

template <class T>
struct FromTupleImpl<type_tuples::TTuple<T>> {
    using type = Cons<T, Nil>;
};

template <class T, class... Ts>
struct FromTupleImpl<type_tuples::TTuple<T, Ts...>> {
    using type = Cons<T, typename FromTupleImpl<type_tuples::TTuple<Ts...>>::type>;
};

template <type_tuples::TypeTuple Tuple>
using FromTuple = FromTupleImpl<Tuple>::type;


////////////////////////////////////////////////////////////////////////////////


template <std::size_t Idx, TypeList TL>
struct TakeImpl {
    using type = Cons<typename TL::Head, typename TakeImpl<Idx - 1, typename TL::Tail>::type>;
};

template <TypeList TL>
struct TakeImpl<1, TL> {
    using type = Cons<typename TL::Head, Nil>;
};

template <TypeList TL>
struct TakeImpl<0, TL> {
    using type = Nil;
};

template <std::size_t Idx>
struct TakeImpl<Idx, Nil> {
    using type = Nil;
};

template <std::size_t N, TypeList TL>
using Take = TakeImpl<N, TL>::type;


////////////////////////////////////////////////////////////////////////////////


template <std::size_t Amnt, TypeList TL>
struct DropImpl {
    using type = DropImpl<Amnt - 1, typename TL::Tail>::type;
};

template <TypeList TL>
struct DropImpl<0, TL> {
    using type = TL;
};

template <std::size_t Amnt>
struct DropImpl<Amnt, Nil> {
    using type = Nil;
};

template <std::size_t N, TypeList TL>
using Drop = DropImpl<N, TL>::type;


////////////////////////////////////////////////////////////////////////////////


template <std::size_t N, typename T>
using Replicate = Take<N, Repeat<T>>;


////////////////////////////////////////////////////////////////////////////////


template <template <typename> class Func, typename T>
struct Iterate {
    using Head = T;
    using Tail = Iterate<Func, Func<T>>;
};


////////////////////////////////////////////////////////////////////////////////


template <TypeList CurTL, TypeList OrigTL>
struct CycleImpl {
    using Head = CurTL::Head;
    using Tail = CycleImpl<typename CurTL::Tail, OrigTL>;
};

template <TypeList OrigTL>
struct CycleImpl<Nil, OrigTL> {
    using Head = OrigTL::Head;
    using Tail = CycleImpl<typename OrigTL::Tail, OrigTL>;
};

template <TypeList TL>
using Cycle = CycleImpl<TL, TL>;


////////////////////////////////////////////////////////////////////////////////


template <template <typename> class Func, TypeList TL>
struct MapImpl {
    using type = Cons<Func<typename TL::Head>, typename MapImpl<Func, typename TL::Tail>::type>;
};

template <template <typename> class Func>
struct MapImpl<Func, Nil> {
    using type = Nil;
};

template <template <typename> class Func, TypeList TL>
using Map = MapImpl<Func, TL>::type;

// Your fun, fun metaalgorithms :)

} // namespace type_lists
