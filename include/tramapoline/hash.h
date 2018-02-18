/************************************************************************/
/* Borrowed from https://gist.github.com/filsinger/1255697 and adapted  */
/* by marttyfication													*/
/************************************************************************/

#pragma once

#include <iostream>
#include <cassert>

namespace hash {
template <typename S> struct fnv_internal;
template <typename S> struct fnv1a_tpl;

template <> struct fnv_internal<uint32_t> {
	constexpr static uint32_t default_offset_basis = 0x811C9DC5;
	constexpr static uint32_t prime = 0x01000193;
};

template <> struct fnv1a_tpl<uint32_t> : public fnv_internal<uint32_t> {
	constexpr static inline uint32_t hash(char const*const aString, const uint32_t val = default_offset_basis) {
		return (aString[0] == '\0') ? val : hash(&aString[1], (val ^ uint32_t(aString[0])) * prime);
	}

	constexpr static inline uint32_t hash(char const*const aString, const size_t aStrlen, const uint32_t val) {
		return (aStrlen == 0) ? val : hash(aString + 1, aStrlen - 1, (val ^ uint32_t(aString[0])) * prime);
	}
};

using fnv1a = fnv1a_tpl<uint32_t>;
} // namespace hash

inline constexpr uint32_t operator "" _fnv1a(const char* aString, const size_t aStrlen) {
	typedef hash::fnv1a_tpl<uint32_t> hash_type;
	return hash_type::hash(aString, aStrlen, hash_type::default_offset_basis);
}
