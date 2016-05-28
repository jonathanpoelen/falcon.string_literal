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
* \brief     string hash function
*
* \ingroup strings
*/

#pragma once

#include <falcon/functional/fnv.hpp>
#include <falcon/cxx/string_view.hpp>
#include <falcon/container/string_literal_fwd.hpp>

namespace falcon {

/// FNV-1a hash
template<class Ch>
constexpr std::size_t
string_id(Ch const * s, std::size_t n) noexcept
{ return fnv1a_hash_fn{}(s, s+n); }

/// FNV-1a hash
template<class Ch>
constexpr std::size_t
string_id(Ch const * s) noexcept
{
  struct znull {};
  struct iterator {
    Ch const * s;
    iterator & operator++() noexcept { ++s; return *this; }
    Ch operator*() const noexcept { return *s; }
    constexpr bool operator != (znull) const noexcept { return *s; }
  };
  return fnv1a_hash_fn{}(iterator{s}, znull{});
}

/// FNV-1a hash
template<class Ch, class Tr, class Alloc>
std::size_t string_id(std::basic_string<Ch, Tr, Alloc> const & str) noexcept
{ return fnv1a_hash_fn{}(str.data(), str.data()+str.size()); }

#ifdef FALCON_STD_STRING_VIEW
/// FNV-1a hash
template<class Ch, class Tr>
constexpr std::size_t
string_id(FALCON_STD_STRING_VIEW<Ch, Tr> const & str) noexcept
{ return fnv1a_hash_fn{}(str.data(), str.data()+str.size()); }
#endif

inline namespace container
{
  /// FNV-1a hash
  template<class Ch, class Tr, std::size_t N>
  constexpr std::size_t string_id(basic_string_literal<Ch, N, Tr> const & str)
  { return fnv1a_hash_fn{}(str.data(), str.data() + str.size()); }
}

}
