
// think-cell public library
//
// Copyright (C) 2016-2019 think-cell Software GmbH
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include "range_defines.h"
#include "generic_macros.h"
#include "append.h"

namespace tc {
	TC_HAS_EXPR(as_c_str, (T), tc::as_c_str(std::declval<T>()))

	namespace no_adl {
		template< typename Char, typename Rng, typename Enable = void >
		struct has_convertible_as_c_str final
		: std::integral_constant<bool, false> {};

		template< typename Char, typename Rng >
		struct has_convertible_as_c_str< Char, Rng, std::enable_if_t< tc::has_as_c_str<Rng>::value > > final
		: std::integral_constant<bool, tc::is_safely_convertible<decltype(tc::as_c_str(std::declval<Rng>())), Char*>::value> {};

		template< typename Char, typename Str >
		struct make_c_str_impl final {
			explicit make_c_str_impl(Str&& str) noexcept: m_str(std::forward<Str>(str)) {}

			operator bool() noexcept = delete;

			operator Char const*() const /*no &*/ noexcept {
				return tc::as_c_str(m_str);
			}

			operator Char*() /*no &*/ noexcept {
				return tc::as_c_str(m_str);
			}
		private:
			Str m_str;
		};
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//  make_c_str(<Char>): create a value or reference holder which is castable to c string

	//   1. One input range, tc::as_c_str(rng) is valid and convertible to Char const*: hold the value or reference of the rng, castable to tc::as_c_str(rng)
	//   2. Otherwise: create and hold a std::basic_string<Char>, castable to Char const*
	//   3. Explicitly specified <Char> could be omitted if the char pointer type deduced from the first rng is convertible to destination c string
	//   4. make_mutable_c_str(<Char>): create a value or reference holder which is castable to mutable c string

	template< typename Char, typename Rng0, std::enable_if_t<tc::no_adl::has_convertible_as_c_str<Char const, Rng0>::value>* = nullptr >
	auto make_c_str(Rng0&& rng0) noexcept {
		static_assert(tc::is_decayed<Char>::value);
		return tc::no_adl::make_c_str_impl<Char const, Rng0>(std::forward<Rng0>(rng0));
	}

	template< typename Rng0, std::enable_if_t<tc::has_as_c_str<Rng0>::value>* = nullptr >
	auto make_c_str(Rng0&& rng0) noexcept {
		return tc::make_c_str<std::remove_cv_t<typename std::pointer_traits<decltype(tc::as_c_str(std::declval<Rng0>()))>::element_type>>(std::forward<Rng0>(rng0));
	}

	template< typename Char, typename Rng0, typename ... Rng >
	auto make_c_str(Rng0&& rng0, Rng&& ... rng) MAYTHROW {
		static_assert(tc::is_decayed<Char>::value);
		return tc::no_adl::make_c_str_impl<Char const, std::basic_string<Char>>(tc::make_str<Char>(std::forward<Rng0>(rng0), std::forward<Rng>(rng)...));
	}

	template< typename Rng0, typename ... Rng >
	auto make_c_str(Rng0&& rng0, Rng&& ... rng) MAYTHROW {
		return tc::make_c_str<tc::common_range_value_t<Rng0, Rng...>>(std::forward<Rng0>(rng0), std::forward<Rng>(rng)...);
	}

	template< typename Char, typename Rng0, std::enable_if_t<tc::no_adl::has_convertible_as_c_str<Char, Rng0>::value>* = nullptr >
	auto make_mutable_c_str(Rng0&& rng0) noexcept {
		static_assert(tc::is_decayed<Char>::value);
		return tc::no_adl::make_c_str_impl<Char, Rng0>(std::forward<Rng0>(rng0));
	}

	template< typename Rng0, std::enable_if_t<tc::has_as_c_str<Rng0>::value>* = nullptr >
	auto make_mutable_c_str(Rng0&& rng0) noexcept {
		return tc::make_mutable_c_str<std::remove_cv_t<typename std::pointer_traits<decltype(tc::as_c_str(std::declval<Rng0>()))>::element_type>>(std::forward<Rng0>(rng0));
	}

	template< typename Char, typename Rng0, typename ... Rng >
	auto make_mutable_c_str(Rng0&& rng0, Rng&& ... rng) MAYTHROW {
		static_assert(tc::is_decayed<Char>::value);
		return tc::no_adl::make_c_str_impl<Char, std::basic_string<Char>>(tc::make_str<Char>(std::forward<Rng0>(rng0), std::forward<Rng>(rng)...));
	}

	template< typename Rng0, typename ... Rng >
	auto make_mutable_c_str(Rng0&& rng0, Rng&& ... rng) MAYTHROW {
		return tc::make_mutable_c_str<tc::common_range_value_t<Rng0, Rng...>>(std::forward<Rng0>(rng0), std::forward<Rng>(rng)...);
	}
}

