#pragma once

#include <cstddef>
#include <stdexcept>
#include <cstring>

#include "Utility/hash.h"

#ifndef _MSC_VER
#  if __cplusplus < 201103
#    define CONSTEXPR11_TN
#    define CONSTEXPR14_TN
#    define NOEXCEPT_TN
#  elif __cplusplus < 201402
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN
#    define NOEXCEPT_TN noexcept
#  else
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN constexpr
#    define NOEXCEPT_TN noexcept
#  endif
#else  // _MSC_VER
#  if _MSC_VER < 1900
#    define CONSTEXPR11_TN
#    define CONSTEXPR14_TN
#    define NOEXCEPT_TN
#  elif _MSC_VER < 2000
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN constexpr
#    define NOEXCEPT_TN noexcept
#  else
#    define CONSTEXPR11_TN constexpr
#    define CONSTEXPR14_TN constexpr
#    define NOEXCEPT_TN noexcept
#  endif
#endif  // _MSC_VER

class static_string {
	const char* const p_;
	const std::size_t sz_;

public:
	typedef const char* const_iterator;

	template <std::size_t N>
	CONSTEXPR11_TN static_string(const char(&a)[N]) NOEXCEPT_TN
		: p_(a)
		, sz_(N - 1) {}

	CONSTEXPR11_TN static_string(const char* p, std::size_t N) NOEXCEPT_TN
		: p_(p)
		, sz_(N) {}

	CONSTEXPR11_TN const char* data() const NOEXCEPT_TN { return p_; }
	CONSTEXPR11_TN std::size_t size() const NOEXCEPT_TN { return sz_; }

	CONSTEXPR11_TN const_iterator begin() const NOEXCEPT_TN { return p_; }
	CONSTEXPR11_TN const_iterator end()   const NOEXCEPT_TN { return p_ + sz_; }

	CONSTEXPR11_TN char operator[](std::size_t n) const {
		return n < sz_ ? p_[n] : throw std::out_of_range("static_string");
	}
};

template <class T>
CONSTEXPR14_TN
static_string
type_name() {
#ifdef __clang__
	static_string p = __PRETTY_FUNCTION__;
	return static_string(p.data() + 31, p.size() - 31 - 1);
#elif defined(__GNUC__)
	static_string p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
	return static_string(p.data() + 36, p.size() - 36 - 1);
#  else
	return static_string(p.data() + 46, p.size() - 46 - 1);
#  endif
#elif defined(_MSC_VER)
	static_string p = __FUNCSIG__;
	static_string d = { p.data() + 38, p.size() - 38 - 7 };
	return d;
#endif
}

template <typename T> std::string name_of(void) { return std::string{ type_name<T>().data(), type_name<T>().size() }; }
// was originally +- 7, but that truncated the front and back, respectively.
template <typename T> std::string stripped_name_of(void) { return std::string{ type_name<T>().data() + 6, type_name<T>().size() - 6 }; }
template <typename T> constexpr uint32_t hash_of(void) { return hash::fnv1a::hash(type_name<T>().data()); }
