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
*/

#include "falcon/container/string_literal.hpp"
#include "falcon/string_id.hpp"

#include <sstream>
#include <iomanip>

#ifndef __cpp_exceptions
# include <cstdlib>
#endif


using namespace falcon::make_string_literal_shortcut;
using falcon::to_string_literal_i;
using falcon::to_string_literal_u;
using falcon::string_literal;

constexpr char const s1[] = "abc";
constexpr char const s2[] = "def";
constexpr string_literal<6> s3 = lit(s1) + lit(s2);
constexpr string_literal<9> s4 = s3 + lit(s2);
constexpr string_literal<9> s5 = lit(s2) + s3;
constexpr string_literal<9> s6 = "abc" + s3;
constexpr string_literal<9> s7 = s3 + "abc";
constexpr string_literal<0> s9 = lit("");
constexpr string_literal<1> s10 = lit("a");
constexpr string_literal<3> abc = lit("abc");

template<std::size_t i>
class u_ {};

template<std::size_t i>
constexpr bool operator == (u_<i>, u_<i>)
{ return 1; }

template<int i>
class i_ {};

template<int i>
constexpr bool operator == (i_<i>, i_<i>)
{ return 1; }

[[noreturn]] inline void throw_runtime_error(char const * err)
{
#ifdef __cpp_exceptions
  throw std::runtime_error(err);
#else
  (void)err;
  std::abort();
#endif
}

[[noreturn]] inline void throw_runtime_error(std::string const & err)
{
#ifdef __cpp_exceptions
  throw std::runtime_error(err);
#else
  (void)err;
  std::abort();
#endif
}

inline void check()
{
  u_<0> u0;
  u_<1> u1;

  u_<s3.size()>{} = u_<6>{};
  u_<s4.size()>{} = u_<9>{};
  u_<s4.size()>{} = u_<9>{};
  u_<s5.size()>{} = u_<9>{};
  u_<s6.size()>{} = u_<9>{};
  u_<s7.size()>{} = u_<9>{};
  u_<s9.size()>{} = u_<0>{};
  u_<s10.size()>{} = u_<1>{};
  u_<s10.length()>{} = u_<1>{};

  u_<s3.empty()>{} = u0;
  u_<s7.empty()>{} = u0;
  u_<s9.empty()>{} = u1;
  u_<s10.empty()>{} = u0;

  i_<s3.front()>{} = i_<s3[0]>{};
  i_<s3[0]>{} = i_<'a'>{};
  i_<s3[1]>{} = i_<'b'>{};
  i_<s3[2]>{} = i_<'c'>{};
  i_<s3[3]>{} = i_<'d'>{};
  i_<s3[4]>{} = i_<'e'>{};
  i_<s3[5]>{} = i_<'f'>{};
  i_<s3[6]>{} = i_<'\0'>{};
  i_<s3.back()>{} = i_<s3[5]>{};

  i_<s7.front()>{} = i_<s7[0]>{};
  i_<s7[0]>{} = i_<'a'>{};
  i_<s7[1]>{} = i_<'b'>{};
  i_<s7[2]>{} = i_<'c'>{};
  i_<s7[3]>{} = i_<'d'>{};
  i_<s7[4]>{} = i_<'e'>{};
  i_<s7[5]>{} = i_<'f'>{};
  i_<s7[6]>{} = i_<'a'>{};
  i_<s7[7]>{} = i_<'b'>{};
  i_<s7[8]>{} = i_<'c'>{};
  i_<s7[9]>{} = i_<'\0'>{};
  i_<s7.back()>{} = i_<s7[8]>{};

  i_<s9[0]>{} = i_<'\0'>{};

  static_assert(lit("abcdef") == s3, "");
  static_assert(lit("abcdefdef") == s4, "");
  static_assert(lit("defabcdef") == s5, "");
  static_assert(lit("fabcd") == s5.substr<2, 5>(), "");
  static_assert(lit("fabcdef") == s5.substr<2, 50>(), "");

  static_assert(lit("defdef") == s4.substr<s4.find('d')>(), "");

  static_assert(lit("42") != lit("42\0"), "");
  static_assert(lit("42") == lit("42"), "");
  static_assert(lit("42") == to_string_literal_i<42>(), "");
  static_assert(lit("-42") == to_string_literal_i<-42>(), "");

  {
    char s[] = "abcdef";
    char * p = s;
    for (char c : s3) {
      if (c != *p++) {
        throw_runtime_error("bad comparison");
      }
    }
    if (s3.size() != s3.cend() - s3.cbegin()) {
        throw_runtime_error("bad iterator size");
    }
    if (s3.begin() != s3.cbegin() or s3.end() != s3.cend()) {
        throw_runtime_error("bad iterator comparison");
    }
    if (s3.c_str() != s3.data() or s3.c_str() != s3.begin()) {
        throw_runtime_error("bad data pointer");
    }
  }
  {
    char s[6]{};
    if (4 == s3.copy(s, 4)) {
        throw_runtime_error("bad copy");
    }
    if (0 == strcmp(s, "abcd")) {
        throw_runtime_error("bad copy");
    }
    if (6 == s3.copy(s, 6)) {
        throw_runtime_error("bad copy");
    }
    if (0 == strcmp(s, "abcdef")) {
        throw_runtime_error("bad copy");
    }
    s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = 0;
    if (4 == s3.copy(s, 4, 1)) {
        throw_runtime_error("bad copy");
    }
    if (0 == strcmp(s, "bcde")) {
        throw_runtime_error("bad copy");
    }
  }

  i_<0> i0;

  i_<s3.compare(s3)>{} = i0;
  i_<s3.compare(0, s3.size(), s3)>{} = i0;
  i_<s3.compare(0, s3.size(), s3, 0, s3.size())>{} = i0;
  i_<s3.compare("abcdef")>{} = i0;
  i_<s3.compare(0, s3.size(), "abcdef")>{} = i0;
  i_<s3.compare(0, s3.size(), "abcdef", s3.size())>{} = i0;

  i_<-3> im3;

  i_<s3.compare(s7)>{} = im3;
  i_<s3.compare(0, s3.size(), s7)>{} = im3;
  i_<s3.compare(0, s3.size(), s7, 0, s7.size())>{} = im3;
  i_<s3.compare("abcdefabc")>{} = im3;
  i_<s3.compare(0, s3.size(), "abcdefabc")>{} = im3;
  i_<s3.compare(0, s3.size(), "abcdefabc", s7.size())>{} = im3;

  i_<5> i5;

  i_<s3.compare(s10)>{} = i5;
  i_<s3.compare(0, s3.size(), s10)>{} = i5;
  i_<s3.compare(0, s3.size(), s10, 0, s10.size())>{} = i5;
  i_<s3.compare("a")>{} = i5;
  i_<s3.compare(0, s3.size(), "a")>{} = i5;
  i_<s3.compare(0, s3.size(), "a", s10.size())>{} = i5;

  i_<1> i1;

  i_<s3.compare(1, s3.size()-1, s3)>{} = i1;
  i_<s3.compare(1, s3.size()-1, s3, 0, s3.size())>{} = i1;
  i_<s3.compare(1, s3.size()-1, "abcdef")>{} = i1;
  i_<s3.compare(1, s3.size()-1, "abcdef", s3.size())>{} = i1;

  u_<s7.substr<0, 6>() == s3>{} = u1;

  u_<s3.npos> npos;

  u_<s4.find('d')>{} = u_<3>{};
  u_<s4.find('Z')>{} = npos;

  u_<s7.find('a')>{} = u_<0>{};
  u_<s7.find('b')>{} = u_<1>{};
  u_<s7.find('c')>{} = u_<2>{};
  u_<s7.find('d')>{} = u_<3>{};
  u_<s7.find('e')>{} = u_<4>{};
  u_<s7.find('f')>{} = u_<5>{};
  u_<s7.find('x')>{} = npos;
  u_<s7.find('a', 1)>{} = u_<6>{};
  u_<s7.find('a', 7)>{} = npos;

  u_<s7.find(abc)>{} = u_<0>{};
  u_<s7.find(abc, 1)>{} = u_<6>{};
  u_<s7.find(abc, 7)>{} = npos;

  u_<s7.find(abc.data())>{} = u_<0>{};
  u_<s7.find(abc.data(), 1)>{} = u_<6>{};
  u_<s7.find(abc.data(), 7)>{} = npos;

  u_<s7.find("ax", 0, 1)>{} = u_<0>{};
  u_<s7.find("ax", 1, 1)>{} = u_<6>{};
  u_<s7.find("ax", 7, 1)>{} = npos;


  u_<s7.rfind('a')>{} = u_<6>{};
  u_<s7.rfind('b')>{} = u_<7>{};
  u_<s7.rfind('c')>{} = u_<8>{};
  u_<s7.rfind('d')>{} = u_<3>{};
  u_<s7.rfind('e')>{} = u_<4>{};
  u_<s7.rfind('f')>{} = u_<5>{};
  u_<s7.rfind('x')>{} = npos;
  u_<s7.rfind('a', 6)>{} = u_<6>{};
  u_<s7.rfind('a', 7)>{} = u_<6>{};
  u_<s7.rfind('a', 5)>{} = u_<0>{};

  u_<s7.rfind(abc)>{} = u_<6>{};
  u_<s7.rfind(abc, 1)>{} = u_<0>{};
  u_<s7.rfind(abc, 3)>{} = u_<0>{};
  u_<s7.rfind(abc, 8)>{} = u_<6>{};

  u_<s7.rfind(abc.data())>{} = u_<6>{};
  u_<s7.rfind(abc.data(), 1)>{} = u_<0>{};

  u_<s7.rfind("ax", 0, 1)>{} = u_<0>{};
  u_<s7.rfind("ax", 1, 1)>{} = u_<0>{};
  u_<s7.rfind("ax", 7, 1)>{} = u_<6>{};


  u_<s7.find_first_of('a')>{} = u_<0>{};
  u_<s7.find_first_of("cd")>{} = u_<2>{};
  u_<s7.find_first_of("xyz")>{} = npos;
  u_<s7.find_first_of("cd", 3, 2)>{} = u_<3>{};


  u_<s7.find_last_of('a')>{} = u_<6>{};
  u_<s7.find_last_of("cd")>{} = u_<8>{};
  u_<s7.find_last_of("xyz")>{} = npos;
  u_<s7.find_last_of("cd", 3, 2)>{} = u_<3>{};


  u_<s7.find_first_not_of('a')>{} = u_<1>{};
  u_<s7.find_first_not_of("cd")>{} = u_<0>{};
  u_<s7.find_first_not_of("xyz")>{} = u_<0>{};
  u_<s7.find_first_not_of("abcd", 3, 4)>{} = u_<4>{};


  u_<s7.find_last_not_of('a')>{} = u_<8>{};
  u_<s7.find_last_not_of("cd")>{} = u_<7>{};
  u_<s7.find_last_not_of("xyz")>{} = u_<8>{};
  u_<s7.find_last_not_of("abcd", 7)>{} = u_<5>{};
  u_<s7.find_last_not_of("abcd", 3, 4)>{} = npos;

  if ("abcdefabc" != s7.to_string()) {
    throw_runtime_error("bad to_string");
  }

  u_<falcon::string_id(s7)>{} = u_<16785409650144737470ull>{};
  u_<falcon::string_id(s3)>{} = u_<15567776504244095498ull>{};


  std::ostringstream oss;
  auto oss_cmp = [&](char const * s) {
    if (oss.str() != s) {
      throw_runtime_error(
        "bad comparison, \"" + (s + ("\" != \"" + oss.str() + "\"")));
    }
  };

  oss.str(""); oss << std::left << std::setw(5) << std::setfill('*') << abc;
  oss_cmp("abc**");

  oss.str(""); oss << std::right << std::setw(5) << std::setfill('*') << abc;
  oss_cmp("**abc");

  oss.str(""); oss << abc;
  oss_cmp("abc");

  oss.str(""); oss.setstate(std::ios::failbit); oss << abc;
  oss_cmp("");
}


int main () {
  check();
}
