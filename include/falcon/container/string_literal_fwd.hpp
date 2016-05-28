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
*
* \ingroup strings
* \ingroup sequences
*/

#pragma once

#include <iosfwd>


namespace falcon {
inline namespace container {

template<class Ch, std::size_t N, class Traits = std::char_traits<Ch>>
struct basic_string_literal;

template<std::size_t n> using string_literal    = basic_string_literal<char, n>;
template<std::size_t n> using wstring_literal   = basic_string_literal<wchar_t, n>;
template<std::size_t n> using u16string_literal = basic_string_literal<char16_t, n>;
template<std::size_t n> using u32string_literal = basic_string_literal<char32_t, n>;

} // container
}
