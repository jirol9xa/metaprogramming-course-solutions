#pragma once

#include <concepts>

#include <cstddef>
#include <type_traits>
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

template <TypeList TL>
    requires Empty<typename TL::Head>
struct ToTupleImpl<TL> {
    using type = typename ToTupleImpl<typename TL::Tail>::type;
};

template <Empty T>
struct ToTupleImpl<T> {
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

template <Empty TL>
struct TakeImpl<0, TL> {
    using type = Nil;
};

template <TypeList TL>
struct TakeImpl<0, TL> {
    using type = Nil;
};

template <std::size_t Idx, Empty TL>
struct TakeImpl<Idx, TL> {
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

template<>
struct DropImpl<0, Nil> : Nil {
    using type = Nil;
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

template <>
struct CycleImpl<Nil, Nil> : Nil {};

template <TypeList TL>
using Cycle = CycleImpl<TL, TL>;


////////////////////////////////////////////////////////////////////////////////


template <template <typename> class Func, TypeList TL>
struct MapImpl {
    using Head = Func<typename TL::Head>;
    using Tail = MapImpl<Func, typename TL::Tail>;
};

template <template <typename> class Func, typename T>
struct MapImpl<Func, Cons<T, Nil>> {
    using Head = Func<T>;
    using Tail = Nil;
};

template <template <typename> class Func, Empty TL>
struct MapImpl<Func, TL> : Nil {};

template <template <typename> class Func, TypeList TL>
using Map = MapImpl<Func, TL>;


////////////////////////////////////////////////////////////////////////////////

template <template <typename> class Pred, TypeList TL, bool IsMatched>
struct FirstMatchImpl;

template <template <typename> class Pred, TypeList TL>
struct FirstMatch {
    using type = FirstMatchImpl<Pred, TL, Pred<typename TL::Head>::Value>::type; 
};

template <template <typename> class Pred, TypeList TL>
struct FirstMatchImpl<Pred, TL, true> {
    using type = TL;
};

template <template <typename> class Pred, TypeList TL>
struct FirstMatchImpl<Pred, TL, false> {
    using type = FirstMatch<Pred, typename TL::Tail>::type;
};

template <template <typename> class Pred>
struct FirstMatch<Pred, Nil> : Nil {
    using type = Nil;
};

template <template <typename> class Pred, TypeList TL>
struct FilterImpl {
    using FirstMatch = FirstMatch<Pred, TL>::type;
    using Head = FirstMatch::Head;
    using Tail = FilterImpl<Pred, typename FirstMatch::Tail>;
};

template <template <typename> class Pred, TypeList TL>
    requires Empty<FirstMatch<Pred, TL>>
struct FilterImpl<Pred, TL> : Nil {};

template <template <typename> class Pred>
struct FilterImpl<Pred, Nil> : Nil {};

template <template <typename> class Pred, TypeList TL>
using Filter = FilterImpl<Pred, TL>;


////////////////////////////////////////////////////////////////////////////////


template <template <typename, typename> class Op, TypeList TL>
struct ScanlImpl {
    using Head = Op<typename TL::Head, typename TL::Tail::Head>;
    using Tail = ScanlImpl<Op, typename TL::Tail>;
};

template <template <typename, typename> class Op, TypeList TL>
    requires Empty<typename TL::Tail>
struct ScanlImpl<Op, TL> : Nil {};

template <template <typename, typename> class Op, typename T, TypeList TL>
struct ScanlInit {
    using Head = T;
    // Are u sure, that we should concat???
    using Tail = ScanlImpl<Op, Cons<T, TL>>;
};

template <template <typename, typename> class Op, typename T, TypeList TL>
using Scanl = ScanlInit<Op, T, TL>;


////////////////////////////////////////////////////////////////////////////////


template <template <typename, typename> class Op, typename T, TypeList TL>
struct FoldlImpl {
    using newT = Op<T, typename TL::Head>;
    using type = FoldlImpl<Op, newT, typename TL::Tail>::type;
};

template <template <typename, typename> class Op, typename T, Empty TL>
struct FoldlImpl<Op, T, TL> {
    using type = T;
};

template <template <typename, typename> class Op, typename T, TypeList TL>
using Foldl = FoldlImpl<Op, T, TL>::type;


////////////////////////////////////////////////////////////////////////////////


template <template <typename, typename> class Op, typename T, TypeList TL>
struct FoldrImpl {
    using type = Op<T, typename FoldrImpl<Op, typename TL::Head, typename TL::Tail>::type>;
};

template <template <typename, typename> class Op, typename T, Empty TL>
struct FoldrImpl<Op, T, TL> {
    using type = Op<T, Nil>;
};

template <template <typename, typename> class Op, typename T, TypeList TL>
using Foldr = FoldrImpl<Op, T, TL>::type;


////////////////////////////////////////////////////////////////////////////////


template <TypeList TL, std::size_t Length, TypeList PrevTL>
struct InitsImpl {
    using Head = Take<Length, TL>;
    using Tail = InitsImpl<TL, Length + 1, Head>;
};

template <TypeList TL, std::size_t Length, TypeList PrevTL>
    requires (Length != 0) && std::same_as<ToTuple<PrevTL>, ToTuple<Take<Length, TL>>>
struct InitsImpl<TL, Length, PrevTL> : Nil {};

template <TypeList TL>
using Inits = InitsImpl<TL, 0, Nil>;


////////////////////////////////////////////////////////////////////////////////


template <TypeList TL>
struct TailsImpl {
    using Head = TL;
    using Tail = TailsImpl<Drop<1, TL>>;
};

template <>
struct TailsImpl<Nil> {
    using Head = Nil;
    using Tail = Nil;
};

template <TypeList TL>
using Tails = TailsImpl<TL>;


////////////////////////////////////////////////////////////////////////////////


template <TypeList L, TypeList R>
struct Zip2Impl {
    using Head = type_tuples::TTuple<typename L::Head, typename R::Head>;
    using Tail = Zip2Impl<Drop<1, L>, Drop<1, R>>;
};

template <TypeList L, TypeList R>
    requires Empty<L> || Empty<R>
struct Zip2Impl<L, R> : Nil {};

template <TypeList L, TypeList R>
using Zip2 = Zip2Impl<L, R>;


////////////////////////////////////////////////////////////////////////////////

template <TypeList... TLs>
struct ZipImpl {
    using Head = type_tuples::TTuple<typename TLs::Head...>;
    using Tail = ZipImpl<typename TLs::Tail...>;
};

template <TypeList... TLs>
constexpr bool AnyEmpty = (Empty<TLs> || ...);

template <TypeList T, TypeList... TLs>
    requires AnyEmpty<T, TLs...>
struct ZipImpl<T, TLs...> : Nil {};

template <TypeList... TLs>
using Zip = ZipImpl<TLs...>;


////////////////////////////////////////////////////////////////////////////////



// Your fun, fun metaalgorithms :)

} // namespace type_lists
