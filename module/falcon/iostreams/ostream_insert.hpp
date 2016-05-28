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

#pragma once

#include <falcon/cxx/string_view.hpp>

#include <iosfwd>
#include <cstddef>


namespace falcon {
namespace iostreams {

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
ostream_fill(
  std::basic_ostream<CharT, Traits> & out
, std::streamsize n, CharT fill);

template<class CharT, class Traits>
std::basic_ostream<CharT, Traits>&
ostream_fill(std::basic_ostream<CharT, Traits> & out, std::streamsize n)
{ return ostream_fill(out, n, out.fill()); }


template <class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
ostream_write(
  std::basic_ostream<CharT, Traits> & out
, CharT const * s, std::streamsize n);

template<class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
ostream_insert(
  std::basic_ostream<CharT, Traits> & out
, CharT const * s, std::size_t n);


template<class CharT, class Traits, class Alloc>
std::basic_ostream<CharT, Traits>&
ostream_write(
  std::basic_ostream<CharT, Traits> & out
, std::basic_string<CharT, Traits, Alloc> const & str)
{ return ostream_write(out, str.data(), str.size()); }

template<class CharT, class Traits, class Alloc>
std::basic_ostream<CharT, Traits>&
ostream_insert(
  std::basic_ostream<CharT, Traits> & out
, std::basic_string<CharT, Traits, Alloc> const & str)
{ return ostream_insert(out, str.data(), str.size()); }


#ifdef FALCON_STD_STRING_VIEW
template<class CharT, class Traits>
std::basic_ostream<CharT, Traits>&
ostream_write(
  std::basic_ostream<CharT, Traits> & out
, FALCON_STD_STRING_VIEW<CharT, Traits> const & str)
{ return ostream_write(out, str.data(), str.size()); }}

template<class CharT, class Traits>
std::basic_ostream<CharT, Traits>&
ostream_insert(
  std::basic_ostream<CharT, Traits> & out
, FALCON_STD_STRING_VIEW<CharT, Traits> const & str)
{ return ostream_insert(out, str.data(), str.size()); }
#endif


namespace detail_
{
  template <class CharT, class Traits>
  inline bool ostream_fill(
    std::basic_streambuf<CharT, Traits> & buf,
    std::streamsize n, CharT fill)
  {
    for (std::streamsize i = 0; i < n; ++i) {
      if (Traits::eq_int_type(buf.sputc(fill), Traits::eof())) {
        return false;
      }
    }
    return true;
  }
}

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
ostream_fill(
  std::basic_ostream<CharT, Traits> & out
, std::streamsize n, CharT fill)
{
  using ostream_type = std::basic_ostream<CharT, Traits>;
  using ios_base = typename ostream_type::ios_base;

  if (typename ostream_type::sentry cerb{out}) {
    if (!detail_::ostream_fill(out.rdbuf(), n, fill)) {
      out.setstate(ios_base::failbit);
    }
  }

  return out;
}

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
ostream_write(
  std::basic_ostream<CharT, Traits> & out,
  CharT const * s, std::streamsize n)
{
  using ostream_type = std::basic_ostream<CharT, Traits>;
  using ios_base = typename ostream_type::ios_base;

  if (typename ostream_type::sentry cerb{out}) {
    if (out.rdbuf()->sputn(s, n) != n) {
      out.setstate(ios_base::badbit);
    }
  }

  return out;
}

template<class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
ostream_insert(
  std::basic_ostream<CharT, Traits> & out,
  CharT const * s, std::size_t n)
{
#ifdef FALCON_STD_STRING_VIEW
  return out << FALCON_STD_STRING_VIEW<CharT, Traits>(s, n);
#else
  using ostream_type = std::basic_ostream<CharT, Traits>;
  using ios_base = typename ostream_type::ios_base;

  if (typename ostream_type::sentry cerb{out}) {
    std::streamsize const w = out.width();
    std::streamsize const n_ = static_cast<std::streamsize>(n);
    auto & buf = *out.rdbuf();
    bool ok = true;

    if (w > n_) {
      bool const left = (out.flags() & ios_base::left);

      if (!left) {
        ok = detail_::ostream_fill(buf, w - n_, out.fill());
      }
      if (ok) {
        ok = buf.sputn(s, n_) == n_;
      }
      if (left && ok) {
        ok = detail_::ostream_fill(buf, w - n_, out.fill());
      }
    }
    else {
      ok = buf.sputn(s, n_) == n_;
    }

    out.width(0);

    if (!ok) {
      out.setstate(ios_base::failbit);
    }
  }

  return out;
#endif
}

} }
