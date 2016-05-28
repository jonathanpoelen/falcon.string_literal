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

#pragma once

#include <falcon/container/string_literal_fwd.hpp>
#include <falcon/iostreams/ostream_insert.hpp>
#include <falcon/cxx/string_view.hpp>
#include <falcon/functional/fnv.hpp>

#include <iosfwd>
#include <limits>
#include <algorithm> // std::min

#ifdef __cpp_exceptions
# include <stdexcept>
# include <cstdio>
#endif

#include <cstddef>
#include <cstring>


namespace falcon {
inline namespace container {

namespace detail_
{
  template<class Ch, class Tr>
  struct constexpr_char_traits;

  struct private_ctor_str_lit
  { explicit private_ctor_str_lit() = default; };

  struct core_access;

  // fixes g++ -std=c++1z constexpr string_view
  template<class Ch, class Tr> struct string_view;
}


/**
 * \brief  High level interface to the string literals.
 *
 * \ingroup strings
 * \ingroup sequences
 *
 * \tparam Ch  Type of character.
 * \tparam N  Number of characters, not including any null-termination.
 * \tparam Traits  Traits for character type.
 */
template<class Ch, std::size_t N, class Traits>
struct basic_string_literal
{
  using value_type = Ch;
  using traits_type = Traits;

  using const_pointer = Ch const *;
  using const_reference = Ch const &;
  using const_iterator = const_pointer;

  using pointer = Ch const *;
  using reference = Ch const &;
  using iterator = pointer;

  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;


  static constexpr size_type npos = size_type(-1);

  /// Returns the number of characters in the string,
  /// not including any null-termination.
  constexpr size_type size() const noexcept { return N; }

  /// Returns the number of characters in the string,
  /// not including any null-termination.
  constexpr size_type length() const noexcept { return size(); }

  ///  Returns the size() of the largest possible %string.
  constexpr size_type max_size() const noexcept
  {
    return (npos - sizeof(size_type) - sizeof(void*))
      / sizeof(value_type) / 4;
  }

  /// Returns true if the %string is empty.
  /// Equivalent to <code>*this == ""</code>.
  constexpr bool empty() const noexcept { return !size(); }

  /**
   * \param  pos  The index of the character to access.
   * \return  Read-only (constant) reference to the character.
   *
   * Returns a const reference to the character at specified location pos.
   * No bounds checking is performed: the behavior is undefined if
   * \a pos >= size().
   */
  constexpr const_reference operator[](size_type pos) const noexcept { return data_[pos]; }

  /// Returns reference to the first character in the %string.
  /// The behavior is undefined if empty() == true.
  constexpr const_reference front() const noexcept { return *data_; }
  /// Returns reference to the last character in the %string.
  ///  The behavior is undefined if empty() == true.
  constexpr const_reference back() const noexcept { return data_[size()-1]; }

  constexpr const_iterator begin() const noexcept { return data_; }
  constexpr const_iterator end() const noexcept { return data_+size(); }

  constexpr const_iterator cbegin() const noexcept { return data_; }
  constexpr const_iterator cend() const noexcept { return data_+size(); }

  /// Return const pointer to null-terminated contents.
  constexpr const_pointer c_str() const noexcept { return data_; }
  /// Return const pointer to null-terminated contents.
  constexpr const_pointer data() const noexcept { return data_; }

  /**
   * \brief  Copy substring into C string.
   * \param s  C string to copy value into.
   * \param n  Number of characters to copy.
   * \param pos  Index of first character to copy.
   * \return  Number of characters actually copied.
   * \throw  std::out_of_range  If \a pos > size().
   *
   * Copies up to \a n characters starting at \a pos into the C string \a s.
   * If \a pos is %greater than size(), out_of_range is thrown.
   */
  size_type copy(Ch * s, size_type n, size_type pos = 0) const
  {
    auto v = view_("copy", n, pos);
    std::memcpy(s, v.data(), v.size());
    return v.size();
  }

  /**
   * \brief  Get a substring.
   * \tparam pos  Index of first character (default 0).
   * \tparam n  Number of characters in substring (default remainder).
   * \return  The new string.
   *
   * Construct and return a new string using the \a n characters starting
   * at \a pos.  If the string is too short, use the remainder of the
   * characters.  If \a pos is beyond the end of the string,
   * compiler error is thrown.
   */
  template<std::size_t pos, std::size_t n = npos>
  constexpr basic_string_literal<Ch, std::min(n, N - pos), Traits>
  substr() const noexcept;

  // string compare
  //@{
  /**
   * \brief  Compare to a string.
   * \param str  String to compare against.
   * \return  Integer < 0, 0, or > 0.
   *
   * Returns an integer < 0 if this string is ordered before \a str,
   * 0 if their values are equivalent, or > 0 if this string is ordered
   * after \a str.  Determines the effective length rlen of the strings
   * to compare as the smallest of size() and str.size().  The function
   * then compares the two strings by calling
   * <code>traits::compare(data(),str.data(),rlen)</code>. If the result
   * of the comparison is nonzero returns it, otherwise the shorter one
   * is ordered first.
   */
  template<std::size_t M>
  constexpr int
  compare(basic_string_literal<Ch, M, Traits> const & str) const noexcept
  { return compare_(view_(), view_(str)); }

  /**
   * \brief  Compare substring to a string.
   * \param pos  Index of first character of substring.
   * \param n  Number of characters in substring.
   * \param str  String to compare against.
   * \return  Integer < 0, 0, or > 0.
   *
   * Form the substring of this string from the \a n characters starting
   * at \a pos.  Returns an integer < 0 if the substring is ordered before
   * \a str, 0 if their values are equivalent, or > 0 if the substring is
   * ordered after \a str.  Determines the effective length rlen of the
   * strings to compare as the smallest of the length of the substring and
   * \a str.size().  The function then compares the two strings by calling
   * <code>traits::compare(substring.data(),str.data(),rlen)</code>.  If the
   * result of the comparison is nonzero returns it, otherwise the shorter
   * one is ordered first.
   */
  template<std::size_t M>
  constexpr int
  compare(
    size_type pos1, size_type n1
  , basic_string_literal<Ch, M, Traits> const & str) const
  { return compare_(view_("compare", pos1, n1), view_(str)); }

  /**
   * \brief  Compare substring to a substring.
   * \param pos1  Index of first character of substring.
   * \param n1  Number of characters in substring.
   * \param str  String to compare against.
   * \param pos2  Index of first character of substring of str.
   * \param n2  Number of characters in substring of str.
   * \return  Integer < 0, 0, or > 0.
   *
   * Form the substring of this string from the \a n1 characters starting
   * at \a pos1.  Form the substring of \a str from the \a n2 characters
   * starting at \a pos2. Returns an integer < 0 if this substring is ordered
   * before the substring of \a str, 0 if their values are equivalent, or > 0
   * if this substring is ordered after the substring of \a str.  Determines
   * the effective length rlen of the strings to compare as the smallest of
   * the lengths of the substrings.  The function then compares the two
   * strings by calling
   * <code>traits::compare(substring.data(),str.substr(pos2,n2).data(),rlen)</code>.
   * If the result of the comparison is nonzero returns it,
   * otherwise the shorter one is ordered first.
   */
  template<std::size_t M>
  constexpr int
  compare(
    size_type pos1, size_type n1
  , basic_string_literal<Ch, M, Traits> const & str
  , size_type pos2, size_type n2) const
  {
    return compare_(
      view_("compare", pos1, n1),
      view_(str, "compare", pos2, n2));
  }

  /**
   * \brief  Compare to a C string.
   * \param s  C string to compare against.
   * \return  Integer < 0, 0, or > 0.
   *
   * Returns an integer < 0 if this string is ordered before \a s, 0 if
   * their values are equivalent, or > 0 if this string is ordered after
   * \a s.  Determines the effective length rlen of the strings to
   * compare as the smallest of size() and the length of a string
   * constructed from \a s.  The function then compares the two strings
   * by calling <code>traits::compare(data(),s,rlen)</code>.  If the
   * result of the comparison is nonzero returns it, otherwise the shorter
   * one is ordered first.
   */
  constexpr int
  compare(Ch const * s) const noexcept
  { return compare_(view_(), {s}); }

  /**
   * \brief  Compare substring to a C string.
   * \param pos  Index of first character of substring.
   * \param n1  Number of characters in substring.
   * \param s  C string to compare against.
   * \return  Integer < 0, 0, or > 0.
   *
   * Form the substring of this string from the \a n1 characters starting
   * at \a pos.  Returns an integer < 0 if the substring is ordered before
   * \a s, 0 if their values are equivalent, or > 0 if the substring is
   * ordered after \a s.  Determines the effective length rlen of the strings
   * to compare as the smallest of the length of the substring and the length
   * of a string constructed from \a s.  The function then compares the two
   * string by calling
   * <code>traits::compare(substring.data(),s,rlen)</code>.  If the result
   * of the comparison is nonzero returns it, otherwise the shorter one is
   * ordered first.
   */
  constexpr int
  compare(size_type pos1, size_type n1, Ch const * s) const
  { return compare_(view_("compare", pos1, n1), {s}); }

  /**
   * \brief  Compare substring against a character %array.
   * \param pos  Index of first character of substring.
   * \param n1  Number of characters in substring.
   * \param s  character %array to compare against.
   * \param n2  Number of characters of s.
   * \return  Integer < 0, 0, or > 0.
   *
   * Form the substring of this string from the \a n1 characters starting
   * at \a pos.  Form a string from the first \a n2 characters of \a s.
   * Returns an integer < 0 if this substring is ordered before the string
   * from \a s, 0 if their values are equivalent, or > 0 if this substring
   * is ordered after the string from \a s.  Determines the effective length
   * rlen of the strings to compare as the smallest of the length of the
   * substring and \a n2.  The function then compares the two strings by
   * calling traits::compare(substring.data(),s,rlen).  If the result of
   * the comparison is nonzero returns it, otherwise the shorter
   * one is ordered first.
   */
  constexpr int
  compare(size_type pos1, size_type n1, Ch const * s, size_type n2) const
  { return compare_(view_("compare", pos1, n1), {s, n2}); }
  //@}


  // string search
  //@{

  /**
   * \brief  Find position of a string.
   * \param str  String to locate.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of start of first occurrence.
   *
   * Starting from \a pos, searches forward for value of \a str within this
   * string.  If found, returns the index where it begins.  If not found,
   * returns npos.
   */
  template<std::size_t M>
  constexpr size_type
  find(
    basic_string_literal<Ch, M, Traits> const & str
  , std::size_t pos = 0) const noexcept
  { return find_(view_(str), pos); }

  /**
   * \brief  Find position of a character.
   * \param c  Character to locate.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for \a c within this string.  If
   * found, returns the index where it was found.  If not found, returns npos.
   */
  constexpr size_type
  find(Ch c, std::size_t pos = 0) const noexcept;

  /**
   * \brief  Find position of a C substring.
   * \param s  C string to locate.
   * \param pos  Index of character to search from.
   * \param n  Number of characters from \a s to search for.
   * \return  Index of start of first occurrence.
   *
   * Starting from \a pos, searches forward for the first \a n characters in
   * \a s within this string.  If found, returns the index where it begins.
   * If not found, returns npos.
   */
  constexpr size_type
  find(Ch const * s, size_type pos, size_type n) const noexcept
  { return find_({s, n}, pos); }

  /**
   * \brief  Find position of a character.
   * \param c  Character to locate.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for \a c within this string.  If
   * found, returns the index where it was found.  If not found, returns npos.
   */
  constexpr size_type
  find(Ch const * s, size_type pos = 0) const noexcept
  { return find_({s}, pos); }

  /**
   * \brief  Find last position of a string.
   * \param str  String to locate.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of start of last occurrence.
   *
   * Starting from \a pos, searches backward for value of \a str within this
   * string.  If found, returns the index where it begins.  If not found,
   * returns npos.
   */
  template<std::size_t M>
  constexpr size_type
  rfind(
    basic_string_literal<Ch, M, Traits> const & str
  , size_type pos = npos) const noexcept
  { return rfind_(view_(str), pos); }

  /**
   * \brief  Find last position of a character.
   * \param c  Character to locate.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for \a c within this string.  If
   * found, returns the index where it was found.  If not found, returns npos.
   */
  constexpr size_type
  rfind(Ch c, size_type pos = npos) const noexcept;

  /**
   * \brief  Find last position of a C substring.
   * \param s  C string to locate.
   * \param pos  Index of character to search back from.
   * \param n  Number of characters from s to search for.
   * \return  Index of start of last occurrence.
   *
   * Starting from \a pos, searches backward for the first \a n characters in
   * \a s within this string.  If found, returns the index where it begins.
   * If not found, returns npos.
  */
  constexpr size_type
  rfind(Ch const * s, size_type pos, size_type n) const noexcept
  { return rfind_({s, n}, pos); }

  /**
   * \brief  Find last position of a C string.
   * \param s  C string to locate.
   * \param pos  Index of character to start search at (default end).
   * \return  Index of start of  last occurrence.
   *
   * Starting from \a pos, searches backward for the value of \a s within this
   * string.  If found, returns the index where it begins.  If not found,
   * returns npos.
   */
  constexpr size_type
  rfind(Ch const * s, size_type pos = npos) const noexcept
  { return rfind_({s}, pos); }

  /**
   * \brief  Find position of a character of string.
   * \param str  String containing characters to locate.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for one of the characters of
   * \a str within this string.  If found, returns the index where it was
   * found.  If not found, returns npos.
   */
  template<std::size_t M>
  constexpr size_type
  find_first_of(
    basic_string_literal<Ch, M, Traits> const & str
  , size_type pos = 0) const noexcept
  { return find_first_of_(view_(str), pos); }

  /**
   * \brief  Find position of a character.
   * \param c  Character to locate.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for the character \a c within this
   * string.  If found, returns the index where it was found.  If not found,
   * returns npos.
   *
   * \note  Equivalent to find(c, pos).
   */
  constexpr size_type
  find_first_of(Ch c, size_type pos = 0) const noexcept
  { return find(c, pos); }

  /**
   * \brief  Find position of a character of C substring.
   * \param s  String containing characters to locate.
   * \param pos  Index of character to search from.
   * \param n  Number of characters from s to search for.
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for one of the first \a n
   * characters of \a s within this string.  If found, returns the index
   * where it was found.  If not found, returns npos.
   */
  constexpr size_type
  find_first_of(Ch const * s, size_type pos, size_type n) const
  { return find_first_of_({s, n}, pos); }

  /**
   * \brief  Find position of a character of C string.
   * \param s  String containing characters to locate.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for one of the characters of \a s
   * within this string.  If found, returns the index where it was found.
   * If not found, returns npos.
   */
  constexpr size_type
  find_first_of(Ch const * s, size_type pos = 0) const noexcept
  { return find_first_of_({s}, pos); }


  /**
   * \brief  Find last position of a character of string.
   * \param str  String containing characters to locate.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for one of the characters of
   * \a str within this string.  If found, returns the index where it was
   * found.  If not found, returns npos.
   */
  template<std::size_t M>
  constexpr size_type
  find_last_of(
    basic_string_literal<Ch, M, Traits> const & str
  , size_type pos = npos) const noexcept
  { return find_last_of_(view_(str), pos); }

  /**
   * \brief  Find last position of a character.
   * \param c  Character to locate.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for \a c within this string.  If
   * found, returns the index where it was found.  If not found, returns npos.
   *
   * \note Equivalent to rfind(c, pos).
   */

  constexpr size_type
  find_last_of(Ch c, size_type pos=npos) const noexcept
  { return this->rfind(c, pos); }

  /**
   * \brief  Find last position of a character of C substring.
   * \param s  C string containing characters to locate.
   * \param pos  Index of character to search back from.
   * \param n  Number of characters from s to search for.
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for one of the first \a n
   * characters of \a s within this string.  If found, returns the index
   * where it was found.  If not found, returns npos.
   */
  constexpr size_type
  find_last_of(Ch const * s, size_type pos, size_type n) const
  { return find_last_of_({s, n}, pos); }

  /**
   * \brief  Find last position of a character of C string.
   * \param s  C string containing characters to locate.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for one of the characters of \a s
   * within this string.  If found, returns the index where it was found.
   * If not found, returns npos.
   */
  constexpr size_type
  find_last_of(Ch const * s, size_type pos = npos) const noexcept
  { return find_last_of_({s}, pos); }

  /**
   * \brief  Find position of a character not in string.
   * \param str  String containing characters to avoid.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for a character not contained
   * in \a str within this string.  If found, returns the index where it
   * was found.  If not found, returns npos.
   */
  template<std::size_t M>
  constexpr size_type
  find_first_not_of(
    basic_string_literal<Ch, M, Traits> const & str
  , size_type pos = 0) const noexcept
  { return find_first_not_of_(view_(str), pos); }

  /**
   * \brief  Find position of a different character.
   * \param c  Character to avoid.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for a character other than \a c
   * within this string.  If found, returns the index where it was found.
   * If not found, returns npos.
   */
  constexpr size_type
  find_first_not_of(Ch c, size_type pos = 0) const noexcept;

  /**
   * \brief  Find position of a character not in C substring.
   * \param s  C string containing characters to avoid.
   * \param pos  Index of character to search from.
   * \param n  Number of characters from __s to consider.
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for a character not contained in
   * the first \a n characters of \a s within this string.  If found, returns
   * the index where it was found.  If not found, returns npos.
   */
  constexpr size_type
  find_first_not_of(Ch const * s, size_type pos, size_type n) const
  { return find_first_not_of_({s, n}, pos); }

  /**
   * \brief  Find position of a character not in C string.
   * \param s  C string containing characters to avoid.
   * \param pos  Index of character to search from (default 0).
   * \return  Index of first occurrence.
   *
   * Starting from \a pos, searches forward for a character not contained in
   * \a s within this string.  If found, returns the index where it was found.
   *   If not found, returns npos.
   */
  constexpr size_type
  find_first_not_of(Ch const * s, size_type pos = 0) const noexcept
  { return find_first_not_of_({s}, pos); }

  /**
   * \brief  Find last position of a character not in string.
   * \param str  String containing characters to avoid.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for a character not contained
   * in \a str within this string.  If found, returns the index where it was
   * found.  If not found, returns npos.
   */
  template<std::size_t M>
  constexpr size_type
  find_last_not_of(
    basic_string_literal<Ch, M, Traits> const & str
  , size_type pos = npos) const noexcept
  { return find_last_not_of_(view_(str), pos); }

  /**
   * \brief  Find last position of a different character.
   * \param c  Character to avoid.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for a character other than \a c
   * within this string.  If found, returns the index where it was found.
   * If not found, returns npos.
   */
  constexpr size_type
  find_last_not_of(Ch c, size_type pos = npos) const noexcept;

  /**
   * \brief  Find last position of a character not in C substring.
   * \param s  C string containing characters to avoid.
   * \param pos  Index of character to search back from.
   * \param n  Number of characters from s to consider.
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for a character not contained in
   * the first \a n characters of \a s within this string. If found, returns
   * the index where it was found.  If not found, returns npos.
   */
  constexpr size_type
  find_last_not_of(Ch const * s, size_type pos, size_type n) const
  { return find_last_not_of_({s, n}, pos); }

  /**
   * \brief  Find last position of a character not in C string.
   * \param s  C string containing characters to avoid.
   * \param pos  Index of character to search back from (default end).
   * \return  Index of last occurrence.
   *
   * Starting from \a pos, searches backward for a character not contained in
   * \a s within this string.  If found, returns the index where it was found.
   * If not found, returns npos.
   */
  constexpr size_type
  find_last_not_of(Ch const * s, size_type pos = npos) const noexcept
  { return find_last_not_of_({s}, pos); }
  //@}


  /// Creates a std::basic_string with a copy of the content of the current string.
  template<class Allocator = std::allocator<Ch>>
  std::basic_string<Ch, Traits, Allocator>
  to_string(Allocator const & a = Allocator()) const
  { return std::basic_string<Ch, Traits, Allocator>(data(), size(), a); }

  /// Creates a std::basic_string with a copy of the content of the current string.
  template<class Allocator>
  explicit operator std::basic_string<Ch, Traits, Allocator> const & () const
  { return to_string<Allocator>(); }

#ifdef FALCON_STD_STRING_VIEW
  /// Creates a std::basic_string_view with the content of the current string.
  constexpr FALCON_STD_STRING_VIEW<Ch, Traits> const &
  to_string_view() const
  { return {data(), size()}; }

  /// Creates a std::basic_string_view with the content of the current string.
  constexpr operator FALCON_STD_STRING_VIEW<Ch, Traits> const & () const
  { return {data(), size()}; }
#endif

private:
  Ch data_[N+1] {};


#ifndef __cpp_exceptions
  void throw_out_of_range_(char const *, size_type) const {}
#else
  [[noreturn]] void throw_out_of_range_(char const *, size_type pos) const;
#endif

  using expr_traits = detail_::constexpr_char_traits<Ch, Traits>;


  using string_view_ = detail_::string_view<Ch, Traits>;

  template<std::size_t M>
  static constexpr string_view_
  view_(basic_string_literal<Ch, M, Traits> const & str) noexcept
  { return {str.data(), str.size()}; }

  constexpr string_view_ view_() const noexcept
  { return {data(), size()}; }

  template<std::size_t M>
  constexpr string_view_ view_(
    basic_string_literal<Ch, M, Traits> const & str
  , char const * func, size_type pos, size_type n) const noexcept
  {
    return pos <= size()
      ? string_view_{
        str.data() + pos, std::min(n, size_type(str.size() - pos))
      } : (throw_out_of_range_(func, pos), string_view_{});
  }

  constexpr string_view_ view_(
    char const * func, size_type pos, size_type n) const noexcept
  {
    return pos <= size()
      ? string_view_{data() + pos, std::min(n, size_type(size() - pos))}
      : (throw_out_of_range_(func, pos), string_view_{});
  }

  constexpr int
  compare_(string_view_ str1, string_view_ str2) const noexcept;

  constexpr size_type
  find_(string_view_ str, std::size_t pos) const noexcept;

  constexpr size_type
  rfind_(string_view_ str, std::size_t pos) const noexcept;

  constexpr size_type
  find_first_of_(string_view_ str, std::size_t pos) const noexcept;

  constexpr size_type
  find_last_of_(string_view_ str, std::size_t pos) const noexcept;

  constexpr size_type
  find_first_not_of_(string_view_ str, std::size_t pos) const noexcept;

  constexpr size_type
  find_last_not_of_(string_view_ str, std::size_t pos) const noexcept;


  friend detail_::core_access;

  constexpr basic_string_literal(
    detail_::private_ctor_str_lit, Ch const * arr, std::size_t n) noexcept;

  constexpr basic_string_literal(
    detail_::private_ctor_str_lit, std::size_t n, Ch c) noexcept;

  constexpr basic_string_literal(
    detail_::private_ctor_str_lit
  , Ch const * arr1, size_type n1
  , Ch const * arr2, size_type n2) noexcept;
};


template<class Ch, class Tr, std::size_t N>
std::basic_ostream<Ch, Tr>&
ostream_write(
  std::basic_ostream<Ch, Tr> & out
, basic_string_literal<Ch, N, Tr> const & str)
{ return iostreams::ostream_write(out, str.data(), str.size()); }

template<class Ch, class Tr, std::size_t N>
std::basic_ostream<Ch, Tr>&
ostream_insert(
  std::basic_ostream<Ch, Tr> & out
, basic_string_literal<Ch, N, Tr> const & str)
{ return iostreams::ostream_insert(out, str.data(), str.size()); }

/**
 * \brief  Write string to a stream.
 * \param os  Output stream.
 * \param str  String to write out.
 * \return  Reference to the output stream.
 *
 * Output characters of \a str into os following the same rules as for
 * writing a C string.
 */
template<class Ch, class Tr, std::size_t N>
std::basic_ostream<Ch, Tr> &
operator<<(
  std::basic_ostream<Ch, Tr> & out
, basic_string_literal<Ch, N, Tr> const & str)
{ return ostream_insert(out, str); }


// string comparison
//@{
template<class Ch, class Tr, std::size_t n1, std::size_t n2>
constexpr bool
operator==(
  basic_string_literal<Ch, n1, Tr> const & x
, basic_string_literal<Ch, n2, Tr> const & y) noexcept
{ return n1 == n2 && x.compare(y) == 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator==(basic_string_literal<Ch, n, Tr> const & x, Ch const * y) noexcept
{ return x.compare(y) == 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator==(Ch const * x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{ return y.compare(x) == 0; }

template<class Ch, class Tr, std::size_t n1, std::size_t n2>
constexpr bool
operator!=(
  basic_string_literal<Ch, n1, Tr> const & x
, basic_string_literal<Ch, n2, Tr> const & y) noexcept
{ return !(x == y); }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator!=(basic_string_literal<Ch, n, Tr> const & x, Ch const * y) noexcept
{ return !(x == y); }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator!=(Ch const * x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{ return !(x == y); }

template<class Ch, class Tr, std::size_t n1, std::size_t n2>
constexpr bool
operator< (
  basic_string_literal<Ch, n1, Tr> const & x
, basic_string_literal<Ch, n2, Tr> const & y) noexcept
{ return x.compare(y) < 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator< (basic_string_literal<Ch, n, Tr> const & x, Ch const * y) noexcept
{ return x.compare(y) < 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator< (Ch const * x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{ return y.compare(x) >= 0; }

template<class Ch, class Tr, std::size_t n1, std::size_t n2>
constexpr bool
operator> (
  basic_string_literal<Ch, n1, Tr> const & x
, basic_string_literal<Ch, n2, Tr> const & y) noexcept
{ return x.compare(y) > 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator> (basic_string_literal<Ch, n, Tr> const & x, Ch const * y) noexcept
{ return x.compare(y) > 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator> (Ch const * x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{ return y.compare(x) <= 0; }

template<class Ch, class Tr, std::size_t n1, std::size_t n2>
constexpr bool
operator<=(
  basic_string_literal<Ch, n1, Tr> const & x
, basic_string_literal<Ch, n2, Tr> const & y) noexcept
{ return x.compare(y) <= 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator<=(basic_string_literal<Ch, n, Tr> const & x, Ch const * y) noexcept
{ return x.compare(y) <= 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator<=(Ch const * x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{ return y.compare(x) > 0; }

template<class Ch, class Tr, std::size_t n1, std::size_t n2>
constexpr bool
operator>=(
  basic_string_literal<Ch, n1, Tr> const & x
, basic_string_literal<Ch, n2, Tr> const & y) noexcept
{ return x.compare(y) >= 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator>=(basic_string_literal<Ch, n, Tr> const & x, Ch const * y) noexcept
{ return x.compare(y) >= 0; }

template<class Ch, class Tr, std::size_t n>
constexpr bool
operator>=(Ch const * x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{ return y.compare(x) < 0; }
//@}


namespace detail_
{
  struct core_access
  {
    template<std::size_t N, class Tr, class Ch>
    static constexpr basic_string_literal<Ch, N, Tr>
    mk_lit(std::size_t n, Ch c) noexcept
    { return {detail_::private_ctor_str_lit{}, n, c}; }

    template<std::size_t N, class Tr, class Ch>
    static constexpr basic_string_literal<Ch, N, Tr>
    mk_lit(Ch const * arr, std::size_t n) noexcept
    { return {detail_::private_ctor_str_lit{}, arr, n}; }

    template<std::size_t N, class Tr, class Ch>
    static constexpr basic_string_literal<Ch, N, Tr>
    mk_lit(
      Ch const * arr1, std::size_t n1,
      Ch const * arr2, std::size_t n2) noexcept
    { return {detail_::private_ctor_str_lit{}, arr1, n1, arr2, n2}; }
  };
}


/// Creates a basic_string_literal object, deducing the target type from the types of arguments.
template<class Ch, class Tr = std::char_traits<Ch>, std::size_t N>
constexpr basic_string_literal<Ch, N-1, Tr>
make_string_literal(Ch const (&arr)[N]) noexcept
{ return detail_::core_access::mk_lit<N-1, Tr>(arr, N-1); }

/// Creates a basic_string_literal object, deducing the target type from the types of arguments.
template<std::size_t N, class Ch, class Tr = std::char_traits<Ch>>
constexpr basic_string_literal<Ch, N, Tr>
make_string_literal(Ch c) noexcept
{ return detail_::core_access::mk_lit<N, Tr>(N, c); }

namespace make_string_literal_shortcut {
  /// Creates a basic_string_literal object, deducing the target type from the types of arguments.
  template<class Ch, class Tr = std::char_traits<Ch>, std::size_t N>
  constexpr basic_string_literal<Ch, N-1, Tr>
  lit(Ch const (&arr)[N]) noexcept
  { return detail_::core_access::mk_lit<N-1, Tr>(arr, N-1); }

  /// Creates a basic_string_literal object, deducing the target type from the types of arguments.
  template<std::size_t N, class Ch, class Tr = std::char_traits<Ch>>
  constexpr basic_string_literal<Ch, N, Tr>
  lit(Ch c) noexcept
  { return detail_::core_access::mk_lit<N, Tr>(N, c); }
}


template<class CharT, std::size_t n1, std::size_t n2, class Tr>
constexpr basic_string_literal<CharT, n1+n2, Tr>
operator+(
  basic_string_literal<CharT, n1, Tr> const & x
, basic_string_literal<CharT, n2, Tr> const & y) noexcept
{
  return detail_::core_access::
    mk_lit<n1+n2, Tr>(x.data(), x.size(), y.data(), y.size());
}

template<class CharT, std::size_t n1, std::size_t n2, class Tr>
constexpr basic_string_literal<CharT, n1-1+n2, Tr>
operator+(
  CharT const (&x) [n1]
, basic_string_literal<CharT, n2, Tr> const & y) noexcept
{
  return detail_::core_access::
    mk_lit<n1-1+n2, Tr>(x, n1-1, y.data(), y.size());
}

template<class CharT, std::size_t n1, std::size_t n2, class Tr>
constexpr basic_string_literal<CharT, n1-1+n2, Tr>
operator+(
  basic_string_literal<CharT, n1, Tr> const & x
, CharT const (&y) [n2]) noexcept
{
  return detail_::core_access::
    mk_lit<n1+n2-1, Tr>(x.data(), x.size(), y, n2-1);
}

template<class Ch, std::size_t n, class Tr>
constexpr basic_string_literal<Ch, n+1, Tr>
operator+(Ch x, basic_string_literal<Ch, n, Tr> const & y) noexcept
{
  return detail_::core_access::
    mk_lit<n+1, Tr>(&x, 1, y.data(), y.size());
}

template<class Ch, std::size_t n, class Tr>
constexpr basic_string_literal<Ch, n+1, Tr>
operator+(basic_string_literal<Ch, n, Tr> const & x, Ch y) noexcept
{
  return detail_::core_access::
    mk_lit<n+1, Tr>(x.data(), x.size(), &y, 1);
}

namespace detail_ {

inline constexpr unsigned digits10_for(bool)
{ return 1; }

template<class T>
constexpr unsigned digits10_for(T val)
{
  unsigned n = (val < 0);
  do {
    ++n;
  } while (val /= T(10));
  return n;
}

}

template<class Ch, class T, T val_, class Tr = std::char_traits<Ch>>
constexpr basic_string_literal<Ch, detail_::digits10_for(val_), Tr>
to_basic_string_literal() noexcept
{
  constexpr unsigned sz = detail_::digits10_for(val_);
  T val = val_;
  Ch buf[sz]{};
  Ch * p = buf + sz;
  if (val < 0) {
    buf[0] = '-';
    *--p = char(-(val % 10) + '0');
    val = -(val / T(10));
  }
  do {
    *--p = char(val % 10 + '0');
  } while (val /= T(10));
  return detail_::core_access::mk_lit<sz, Tr>(buf, sz);
}


template<long long val>
constexpr string_literal<detail_::digits10_for(val)>
to_string_literal_i() noexcept
{ return to_basic_string_literal<char, long long, val>(); }

template<unsigned long long val>
constexpr string_literal<detail_::digits10_for(val)>
to_string_literal_u() noexcept
{ return to_basic_string_literal<char, unsigned long long, val>(); }


template<long long val>
constexpr string_literal<detail_::digits10_for(val)>
to_wstring_literal_i() noexcept
{ return to_basic_string_literal<wchar_t, long long, val>(); }

template<unsigned long long val>
constexpr string_literal<detail_::digits10_for(val)>
to_wstring_literal_u() noexcept
{ return to_basic_string_literal<wchar_t, unsigned long long, val>(); }



// Implementation


template<class Ch, std::size_t N, class Tr>
template<std::size_t pos, std::size_t n>
constexpr basic_string_literal<Ch, std::min(n, N - pos), Tr>
basic_string_literal<Ch, N, Tr>
::substr() const noexcept
{
  static_assert(pos < N, "out of range");
  constexpr auto sz = std::min(n, N - pos);
  return detail_::core_access::mk_lit<sz, Tr>(data_ + pos, sz);
}

#ifdef __cpp_exceptions
template<class Ch, std::size_t N, class Tr>
[[noreturn]] void basic_string_literal<Ch, N, Tr>
::throw_out_of_range_(char const * func, size_type pos) const
{
  char what_str[256];
  std::snprintf(
    what_str, sizeof(what_str),
    "basic_string_literal::%s: pos "
    "(which is %zu) >= this->size() "
    "(which is %zu)",
    func, pos, this->size());
  throw std::out_of_range(what_str);
}
#endif

namespace detail_
{
  template<class Ch, class Tr>
  struct string_view
  {
    using const_pointer = Ch const *;
    using size_type = std::size_t;

    const_pointer s = nullptr;
    size_type sz = 0;

    constexpr string_view() noexcept = default;

    constexpr string_view(const_pointer str, size_type n) noexcept
    : s(str), sz(n)
    {}

    constexpr string_view(const_pointer str) noexcept
    : s(str), sz(detail_::constexpr_char_traits<Ch, Tr>::length(str))
    {}

    constexpr size_type size() const noexcept { return sz; }
    constexpr const_pointer data() const noexcept { return s; }
    constexpr Ch const & operator[](std::size_t i) const noexcept { return s[i]; }
  };
}

template<class Ch, std::size_t N, class Tr>
constexpr int
basic_string_literal<Ch, N, Tr>
::compare_(string_view_ str1, string_view_ str2) const noexcept
{
  int ret = expr_traits
    ::compare(str1.data(), str2.data(), std::min(str1.size(), str2.size()));
  if (0 == ret) {
    using limits = std::numeric_limits<int>;
    auto diff = static_cast<difference_type>(str1.size() - str2.size());
    ret = diff > limits::max() ? limits::max()
      : diff < limits::min() ? limits::min()
      : static_cast<int>(diff);
  }
  return ret;
}


template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find(Ch c, size_type pos) const noexcept
{
  size_type ret = (pos <= size())
    ? npos
    : (throw_out_of_range_("find", pos), npos);
  if (pos < size()) {
    size_type const n = size() - pos;
    Ch const * p = expr_traits::find(data_ + pos, n, c);
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 6)
    if (p)
#else
    if (data_ <= p)
#endif
    {
      ret = size_type(p - data_);
    }
  }
  return ret;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_(string_view_ str, std::size_t pos) const noexcept
{
  if (str.size() == 0) {
    return pos <= size() ? pos : npos;
  }

  if (str.size() <= size()) {
    for (; pos <= size() - str.size(); ++pos) {
      if (traits_type::eq(data_[pos], str[0])
        && 0 == expr_traits::compare(
          data_ + pos + 1, str.data() + 1, str.size() - 1
      )) {
        return pos;
      }
    }
  }

  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::rfind(Ch c, size_type pos) const noexcept
{
  size_type sz = size();
  if (sz > 0) {
    if (--sz > pos) {
      sz = pos;
    }
    for (++sz; sz-- > 0; ) {
      if (traits_type::eq(data_[sz], c)) {
        return sz;
      }
    }
  }
  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::rfind_(string_view_ str, std::size_t pos) const noexcept
{
  if (str.size() <= size()) {
    pos = std::min(size_type(size() - str.size()), pos);
    do {
      if (0 == expr_traits::compare(
        data_ + pos, str.data(), str.size()
      )) {
        return pos;
      }
    }
    while (pos-- > 0);
  }

  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_first_of_(string_view_ str, std::size_t pos) const noexcept
{
  if (str.size()) {
    for (; pos < size(); ++pos) {
      // expr_traits::find fails with gcc
      // if (expr_traits::find(str.data(), str.size(), data_[pos])) {
      //  return sz;
      // }
      Ch const * p = str.data();
      Ch const * e = p + str.size();
      for (; p != e; ++p) {
        if (traits_type::eq(*p, data_[pos])) {
          return pos;
        }
      }
    }
  }
  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_last_of_(string_view_ str, std::size_t pos) const noexcept
{
  size_type sz = this->size();
  if (sz && str.size()) {
    if (--sz > pos) {
      sz = pos;
    }
    do {
      // expr_traits::find fails with gcc
      // if (expr_traits::find(str.data(), str.size(), data_[sz])) {
      //   return sz;
      // }
      Ch const * p = str.data();
      Ch const * e = p + str.size();
      for (; p != e; ++p) {
        if (traits_type::eq(*p, data_[sz])) {
          return sz;
        }
      }
    }
    while (sz-- != 0);
  }
  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_first_not_of(Ch c, size_type pos) const noexcept
{
  for (; pos < size(); ++pos) {
    if (!traits_type::eq(c, data_[pos])) {
      return pos;
    }
  }
  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_first_not_of_(string_view_ str, std::size_t pos) const noexcept
{
  for (; pos < size(); ++pos) {
    // expr_traits::find fails with gcc
    // if (!expr_traits::find(str.data(), str.size(), data_[pos])) {
    //  return pos;
    // }
    Ch const * p = str.data();
    Ch const * e = p + str.size();
    for (; p != e && !traits_type::eq(*p, data_[pos]); ++p) {
    }
    if (p == e) {
      return pos;
    }
  }
  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_last_not_of(Ch c, size_type pos) const noexcept
{
  size_type sz = size();
  if (sz) {
    if (--sz > pos) {
      sz = pos;
    }
    do {
      if (!traits_type::eq(c, data_[sz])) {
        return sz;
      }
    }
    while (sz--);
  }
  return npos;
}

template<class Ch, std::size_t N, class Tr>
constexpr typename basic_string_literal<Ch, N, Tr>::size_type
basic_string_literal<Ch, N, Tr>
::find_last_not_of_(string_view_ str, std::size_t pos) const noexcept
{
  size_type sz = size();
  if (sz) {
    if (--sz > pos) {
      sz = pos;
    }
    do {
      // expr_traits::find fails with gcc
      // if (!expr_traits::find(str.data(), str.size(), data_[sz])) {
      //   return sz;
      // }
      Ch const * p = str.data();
      Ch const * e = p + str.size();
      for (; p != e && !traits_type::eq(*p, data_[sz]); ++p) {
      }
      if (p == e) {
        return sz;
      }
    }
    while (sz--);
  }
  return npos;
}


template<class Ch, std::size_t N, class Tr>
constexpr basic_string_literal<Ch, N, Tr>::basic_string_literal(
  detail_::private_ctor_str_lit, std::size_t n, Ch c) noexcept
{
  Ch * p = data_;
  Ch * e = data_ + n;
  while (p != e) {
    *p++ = c;
  }
}

namespace detail_ {

template<class Ch>
constexpr Ch * acpy(Ch * p, Ch const * arr, std::size_t n) noexcept {
  Ch * e = p + n;
  while (p != e) {
    *p++ = *arr++;
  }
  return p;
}

}

template<class Ch, std::size_t N, class Tr>
constexpr basic_string_literal<Ch, N, Tr>::basic_string_literal(
  detail_::private_ctor_str_lit, Ch const * arr, std::size_t n) noexcept
{ detail_::acpy(data_, arr, n); }

template<class Ch, std::size_t N, class Tr>
constexpr basic_string_literal<Ch, N, Tr>::basic_string_literal(
  detail_::private_ctor_str_lit ctor
, Ch const * arr1, size_type n1
, Ch const * arr2, size_type n2) noexcept
: basic_string_literal(ctor, arr1, n1)
{ detail_::acpy(detail_::acpy(data_, arr1, n1), arr2, n2); }


namespace detail_
{
  template<class Ch, class Tr>
  struct constexpr_char_traits
  {
    static constexpr int
    compare(Ch const * s1, Ch const * s2, std::size_t n) noexcept
    { return Tr::compare(s1, s2, n); }

    static constexpr Ch const *
    find(Ch const * p, std::size_t n, Ch const & ch) noexcept
    { return Tr::find(p, n, ch); }

    static constexpr std::size_t
    length(Ch const * p) noexcept
    { return Tr::length(p); }
  };

  template<class Ch>
  struct constexpr_std_char_traits
  {
    static constexpr int
    compare(Ch const * s1, Ch const * s2, std::size_t n) noexcept
    {
      Ch const * e1 = s1 + n;
      while (s1 != e1 && std::char_traits<Ch>::eq(*s1, *s2)) {
        ++s1, ++s2;
      }
      return s1 == e1 ? 0 : std::char_traits<Ch>::lt(*s1, *s2) ? -1 : 1;
    }

    static constexpr Ch const *
    find(Ch const * p, std::size_t n, Ch const & ch) noexcept
    {
      Ch const * e = p + n;
      for (; p != e; ++p) {
        if (std::char_traits<Ch>::eq(*p, ch)) {
          return p;
        }
      }
      return nullptr;
    }

    static constexpr std::size_t
    length(Ch const * p) noexcept
    {
      std::size_t n = 0;
      for (; *p; ++p) {
        ++n;
      }
      return n;
    }
  };

  template<>
  struct constexpr_char_traits<char, std::char_traits<char>>
  : constexpr_std_char_traits<char>
  {};

  template<>
  struct constexpr_char_traits<wchar_t, std::char_traits<wchar_t>>
  : constexpr_std_char_traits<wchar_t>
  {};

  template<>
  struct constexpr_char_traits<char16_t, std::char_traits<char16_t>>
  : constexpr_std_char_traits<char16_t>
  {};

  template<>
  struct constexpr_char_traits<char32_t, std::char_traits<char32_t>>
  : constexpr_std_char_traits<char32_t>
  {};
}

} // container

namespace iostreams {
  using ::falcon::container::ostream_insert;
  using ::falcon::container::ostream_write;
}

}

namespace std
{
  template<class Ch, size_t N, class Tr>
  struct hash<::falcon::container::basic_string_literal<Ch, N, Tr>>
  : ::falcon::fnv1a_hash<::falcon::container::basic_string_literal<Ch, N, Tr>>
  {};
}
