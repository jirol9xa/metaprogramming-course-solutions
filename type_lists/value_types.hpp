#pragma once

#include <cstddef>
#include <iterator>
#include "type_tuples.hpp"

namespace value_types
{

template<auto V>
struct ValueTag{ static constexpr auto Value = V; };

template<class T, T... ts>
using VTuple = type_tuples::TTuple<ValueTag<ts>...>;


////////////////////////////////////////////////////////////////////////////////

}

template <int N>
struct NatsImpl {
    using Head = value_types::ValueTag<N>;
    using Tail = NatsImpl<N+1>;
};

template <int N>
struct FibImpl {
    using Head = value_types::ValueTag<FibImpl<N - 1>::Head::Value + FibImpl<N - 2>::Head::Value>;
    using Tail = FibImpl<N + 1>;
};

template <int N>
    requires (N <= 1)
struct FibImpl<N> {
    using Head = value_types::ValueTag<N>;
    using Tail = FibImpl<N + 1>;
};

template <int N, int Devider, bool IsAlreadyNonPrime>
struct IsPrime;

template <int N, int Devider>
struct IsPrime<N, Devider, true> {
    static constexpr bool Value = false;
};

template <int N, int Devider>
struct IsPrime<N, Devider, false> {
    static constexpr bool Value = IsPrime<N, Devider - 1, (N % Devider) == 0>::Value;
};

template <int N>
struct IsPrime<N, 1, false> {
    static constexpr bool Value = true;
};

template <int N, bool ShouldCalc>
struct FindNextPrime;

template <int N>
struct FindNextPrime<N, false> {
    static constexpr int Value = 0;
};

template <int N>
struct FindNextPrime<N, true> {
    static constexpr int Value = IsPrime<N, N - 1, false>::Value * N + FindNextPrime<N + 1, !IsPrime<N, N - 1, false>::Value>::Value;
};

template <int N>
struct PrimesImpl {
    using Prime = value_types::ValueTag<FindNextPrime<N, true>::Value>;
    using Head = Prime;
    using Tail = PrimesImpl<Prime::Value + 1>;
};
