// The MIT License (MIT)
//
// Copyright (c) 2018 Mateusz Pusz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <mp-units/systems/si.h>
#ifdef MP_UNITS_IMPORT_STD
import std;
#else
#include <concepts>
#include <cstdint>
#endif

// Implicit conversions between integer quantities must not overflow small values
// (magnitude up to MP_UNITS_OVERFLOW_SAFETY_THRESHOLD, default 2147)

namespace {

using namespace mp_units;
using namespace mp_units::si::unit_symbols;

template<auto From, auto To, typename Rep>
inline constexpr bool implicit = std::convertible_to<quantity<From, Rep>, quantity<To, Rep>>;

template<auto From, auto To, typename FromRep, typename ToRep>
inline constexpr bool implicit_between = std::convertible_to<quantity<From, FromRep>, quantity<To, ToRep>>;

// x1000: overflows std::int16_t from 33 km on, int from 2'147'484 km on
static_assert(!implicit<km, m, std::int16_t>);
static_assert(implicit<km, m, int>);
static_assert(implicit<km, m, std::int64_t>);

// x10^6 still fits 2147 in int (2'147'000'000 <= 2'147'483'647), x10^9 does not
static_assert(implicit<km, mm, int>);
static_assert(!implicit<km, um, int>);
static_assert(implicit<km, um, std::int64_t>);

// For types narrower than the threshold, the largest source value is the limit
static_assert(implicit<m, m, std::int8_t>);
static_assert(!implicit<m, dm, std::int8_t>);
static_assert(implicit<m, dm, std::int16_t>);  // 2147 * 10 = 21470 fits

// Unsigned
static_assert(!implicit<km, m, std::uint16_t>);
static_assert(implicit<km, m, std::uint32_t>);

// Narrower target representation
static_assert(implicit_between<m, m, int, std::int16_t>);
static_assert(!implicit_between<km, m, int, std::int16_t>);
static_assert(implicit_between<km, m, std::int16_t, int>);

// Truncating conversions stay explicit, floating point stays implicit
static_assert(!implicit<m, km, int>);
static_assert(implicit<km, m, float>);
static_assert(implicit_between<km, m, std::int16_t, double>);

// An explicit cast is still available and does what it says
static_assert(value_cast<m>(quantity<km, std::int16_t>{2 * km}) == 2000 * m);

// Arithmetic on mixed units converts both operands to the common unit implicitly
template<typename A, typename B>
concept addable = requires(A a, B b) { a + b; };

static_assert(!addable<quantity<km, int>, quantity<um, int>>);  // x10^9 in int
static_assert(addable<quantity<km, int>, quantity<m, int>>);
// std::int16_t operands are promoted to int, so km + m is computed in int
static_assert(std::same_as<decltype(quantity<km, std::int16_t>{} + quantity<m, std::int16_t>{})::rep, int>);
static_assert(addable<quantity<m, std::int16_t>, quantity<m, std::int16_t>>);
static_assert(quantity<km, int>{1 * km} + quantity<m, int>{1 * m} == 1001 * m);

}  // namespace
