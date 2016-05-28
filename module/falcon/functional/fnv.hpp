/* The MIT License (MIT)

Copyright (c) 2016 jonathan poelen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
* \author    Jonathan Poelen <jonathan.poelen+falcon@gmail.com>
* \version   1.0
* \brief     Fowler–Noll–Vo hash (FNV)
*
* \ingroup functors
*/

#pragma once

#include <type_traits>
#include <iterator>

#include <cstddef>
#include <cstdint>


namespace falcon {

template<class T, T offset_basis, T prime>
struct fnv1_fn
{
  template<class FwIt, class Senti>
  constexpr T operator()(FwIt first, Senti const & senti) noexcept
  {
    T h{offset_basis};
    for (; first != senti; ++first) {
      h *= prime;
      h ^= T(*first);
    }
    return h;
  }
};

template<class T, T offset_basis, T prime>
struct fnv1a_fn
{
  template<class FwIt, class Senti>
  constexpr T operator()(FwIt first, Senti const & senti) noexcept
  {
    T h{offset_basis};
    for (; first != senti; ++first) {
      h ^= T(*first);
      h *= prime;
    }
    return h;
  }
};


template<class FNV, unsigned Bits, class T = void>
struct fnv_xor_folding_fn
{
  template<
    class FwIt, class Senti,
    class R = std::conditional_t<std::is_same<void, T>{},
      decltype(FNV{}(std::declval<FwIt>(), std::declval<Senti>())), T>>
  constexpr std::size_t operator()(FwIt && first, Senti const & senti) noexcept
  {
    auto h = FNV{}(first, senti);
    return R((h >> Bits) ^ (h & ((1u << Bits) - 1u)));
  }
};

constexpr std::size_t fnv_32_offset_basis = 0x811c9dc5u;
constexpr std::size_t fnv_32_prime = 0x1000193u;
constexpr std::size_t fnv_64_offset_basis = 0xcbf29ce484222325u;
constexpr std::size_t fnv_64_prime = 0x100000001b3u;

#ifdef INT32_MIN
using fnv1_32_fn = fnv1_fn<uint32_t, fnv_32_offset_basis, fnv_32_prime>;
using fnv1_64_fn = fnv1_fn<uint64_t, fnv_64_offset_basis, fnv_64_prime>;

using fnv1a_32_fn = fnv1a_fn<uint32_t, fnv_32_offset_basis, fnv_32_prime>;
using fnv1a_64_fn = fnv1a_fn<uint64_t, fnv_64_offset_basis, fnv_64_prime>;
#endif

using fnv1_hash_fn = fnv1_fn<std::size_t,
  (sizeof(std::size_t) > 4 ? fnv_64_offset_basis : fnv_32_offset_basis),
  (sizeof(std::size_t) > 4 ? fnv_64_prime : fnv_32_prime)
>;
using fnv1a_hash_fn = fnv1a_fn<std::size_t,
  (sizeof(std::size_t) > 4 ? fnv_64_offset_basis : fnv_32_offset_basis),
  (sizeof(std::size_t) > 4 ? fnv_64_prime : fnv_32_prime)
>;


template<class FNV, class T>
struct fnv_hash
{
  constexpr std::size_t operator()(T const & k) const noexcept
  {
    using std::begin;
    using std::end;
    return FNV{}(begin(k), end(k));
  }
};

template<class T> using fnv1_hash = fnv_hash<fnv1_hash_fn, T>;
template<class T> using fnv1a_hash = fnv_hash<fnv1a_hash_fn, T>;

}
