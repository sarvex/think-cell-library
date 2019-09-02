
// think-cell public library
//
// Copyright (C) 2016-2019 think-cell Software GmbH
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#include "range.t.h"
#include "type_traits.h"


STATICASSERTSAME(tc::remove_rvalue_reference_t<int>, int);
STATICASSERTSAME(tc::remove_rvalue_reference_t<int const>, int const);
STATICASSERTSAME(tc::remove_rvalue_reference_t<int&>, int&);
STATICASSERTSAME(tc::remove_rvalue_reference_t<int const&>, int const&);
STATICASSERTSAME(tc::remove_rvalue_reference_t<int&&>, int);
STATICASSERTSAME(tc::remove_rvalue_reference_t<int const&&>, int const);
STATICASSERTSAME(tc::remove_rvalue_reference_t<std::vector<int>>, std::vector<int>);
STATICASSERTSAME(tc::remove_rvalue_reference_t<std::vector<int> const>, std::vector<int> const);
STATICASSERTSAME(tc::remove_rvalue_reference_t<std::vector<int>&>, std::vector<int>&);
STATICASSERTSAME(tc::remove_rvalue_reference_t<std::vector<int> const&>, std::vector<int> const&);
STATICASSERTSAME(tc::remove_rvalue_reference_t<std::vector<int>&&>, std::vector<int>);
STATICASSERTSAME(tc::remove_rvalue_reference_t<std::vector<int> const&&>, std::vector<int> const);

namespace void_t_test {
	template< typename T, typename=void >
	struct Foo {
		constexpr static int const value = 0;
	};
	template< typename T >
	struct Foo<T, tc::void_t<typename T::type1>> {
		constexpr static int const value = 1;
	};
	template< typename T >
	struct Foo<T, tc::void_t<typename T::type2>> {
		constexpr static int const value = 2;
	};

	struct Bar0 { };
	struct Bar1 {
		using type1 = int;
	};
	struct Bar2 {
		using type2 = int;
	};

	STATICASSERTEQUAL(0, Foo<Bar0>::value);
	STATICASSERTEQUAL(1, Foo<Bar1>::value);
	STATICASSERTEQUAL(2, Foo<Bar2>::value);

	struct WithFunction {
		void func();
	};
	struct WithoutFunction { };

	TC_HAS_EXPR(func, (T), std::declval<T&>().func());

	static_assert(has_func<WithFunction>::value);
	static_assert(!has_func<WithoutFunction>::value);

	std::false_type check_has_func1(...);
	template< typename T >
	std::true_type check_has_func1(T&& t, std::enable_if_t<has_func<T>::value>* = nullptr);

	static_assert(decltype(check_has_func1(std::declval<WithFunction>()))::value);
	static_assert(!decltype(check_has_func1(std::declval<WithoutFunction>()))::value);

	template< typename T, std::enable_if_t<!has_func<T>::value>* = nullptr >
	std::false_type check_has_func2(T&&);
	template< typename T, std::enable_if_t<has_func<T>::value>* = nullptr >
	std::true_type check_has_func2(T&& t);

	static_assert(decltype(check_has_func2(std::declval<WithFunction>()))::value);
	static_assert(!decltype(check_has_func2(std::declval<WithoutFunction>()))::value);
}

static_assert( tc::is_safely_convertible<int, double>::value );

static_assert( std::is_convertible<double, int>::value );
static_assert( !tc::is_safely_convertible<double, int>::value );

static_assert( std::is_convertible<float, int>::value );
static_assert( !tc::is_safely_convertible<float, int>::value );

static_assert( std::is_convertible<int, unsigned int>::value );
static_assert( !tc::is_safely_convertible<int, unsigned int>::value );

static_assert( std::is_convertible<unsigned int, int>::value );
static_assert( !tc::is_safely_convertible<unsigned int, int>::value );

static_assert(std::is_convertible<int*, bool>::value);
static_assert(!tc::is_safely_convertible<int*, bool>::value);

// scoped enum (enum class)
enum class TEnumClass { a, b, c };
enum TEnum { x, y, z };
static_assert( !tc::is_safely_convertible<int, TEnumClass>::value );
static_assert( !tc::is_safely_convertible<TEnumClass, int>::value );
static_assert( !tc::has_common_reference_prvalue_as_val<tc::type::list<TEnumClass, int>>::value);

// unscoped enum (primitive enum)
enum TPrimitiveEnum { a, b, c };
static_assert( !tc::is_safely_convertible<int, TPrimitiveEnum>::value );
static_assert( std::is_convertible<TPrimitiveEnum, std::underlying_type_t<TPrimitiveEnum> >::value );
static_assert( !tc::is_safely_convertible<TPrimitiveEnum, std::underlying_type_t<TPrimitiveEnum>>::value );
static_assert( !tc::has_common_reference_prvalue_as_val<tc::type::list<TPrimitiveEnum, int>>::value);

enum TPrimitiveEnum2 { l, m };
static_assert( !tc::is_safely_convertible<TPrimitiveEnum, TPrimitiveEnum2>::value );
static_assert( !tc::has_common_reference_prvalue_as_val<tc::type::list<TPrimitiveEnum, TPrimitiveEnum2>>::value);

struct SBase {};
struct SDerived final : SBase {};
static_assert(std::is_convertible<SDerived, SBase>::value);
static_assert(!tc::is_safely_convertible<SDerived, SBase>::value);

static_assert(tc::is_safely_convertible<SDerived&, SDerived>::value);
static_assert(!tc::is_safely_convertible<SDerived&, SBase>::value);
static_assert(tc::is_safely_convertible<SDerived&, SBase&>::value);
static_assert(tc::is_safely_convertible<SDerived&, SBase const&>::value);
static_assert(!tc::is_safely_convertible<SDerived&, SBase&&>::value);
static_assert(tc::is_safely_convertible<SDerived&, SBase const&&>::value);

static_assert(tc::is_safely_convertible<SDerived const&, SDerived>::value);
static_assert(!tc::is_safely_convertible<SDerived const&, SBase>::value);
static_assert(!tc::is_safely_convertible<SDerived const&, SBase&>::value);
static_assert(tc::is_safely_convertible<SDerived const&, SBase const&>::value);
static_assert(!tc::is_safely_convertible<SDerived const&, SBase&&>::value);
static_assert(tc::is_safely_convertible<SDerived const&, SBase const&&>::value);

static_assert(tc::is_safely_convertible<SDerived, SDerived>::value);
static_assert(!tc::is_safely_convertible<SDerived, SBase>::value);
static_assert(!tc::is_safely_convertible<SDerived, SBase const&>::value);
static_assert(!tc::is_safely_convertible<SDerived, SBase&>::value);
static_assert(!tc::is_safely_convertible<SDerived, SBase&&>::value);
static_assert(!tc::is_safely_convertible<SDerived, SBase const&&>::value);

static_assert(tc::is_safely_convertible<SDerived&&, SDerived>::value);
static_assert(!tc::is_safely_convertible<SDerived&&, SBase>::value);
static_assert(!tc::is_safely_convertible<SDerived&&, SBase const&>::value);
static_assert(!tc::is_safely_convertible<SDerived&&, SBase&>::value);
static_assert(tc::is_safely_convertible<SDerived&&, SBase&&>::value);
static_assert(tc::is_safely_convertible<SDerived&&, SBase const&&>::value);

static_assert(tc::is_safely_convertible<SDerived const&&, SDerived>::value);
static_assert(!tc::is_safely_convertible<SDerived const&&, SBase>::value);
static_assert(!tc::is_safely_convertible<SDerived const&&, SBase const&>::value);
static_assert(!tc::is_safely_convertible<SDerived const&&, SBase&>::value);
static_assert(!tc::is_safely_convertible<SDerived const&&, SBase&&>::value);
static_assert(tc::is_safely_convertible<SDerived const&&, SBase const&&>::value);

struct SToInt final {
	operator int() const& noexcept;
};

static_assert(!std::is_convertible<SBase, int>::value);
static_assert(std::is_convertible<SToInt, int>::value);
static_assert(tc::is_safely_convertible<SToInt, int>::value);

static_assert(std::is_convertible<SToInt, int const&>::value);
static_assert(!tc::is_safely_convertible<SToInt, int const&>::value);

static_assert(!tc::is_safely_convertible<SToInt, int&&>::value);
static_assert(!tc::is_safely_convertible<SToInt, int const&&>::value);
static_assert(!tc::is_safely_convertible<SToInt&, int&&>::value);
static_assert(!tc::is_safely_convertible<SToInt&, int const&&>::value);
static_assert(!tc::is_safely_convertible<SToInt&&, int&&>::value);
static_assert(!tc::is_safely_convertible<SToInt&&, int const&&>::value);

static_assert(tc::is_safely_convertible<std::string&, tc::ptr_range<char>>::value);
static_assert(tc::is_safely_convertible<std::string&, tc::ptr_range<char const>>::value);
static_assert(!tc::is_safely_convertible<std::string, tc::ptr_range<char const>>::value);
static_assert(!tc::is_safely_convertible<std::string const, tc::ptr_range<char const>>::value);
static_assert(!tc::is_safely_convertible<std::string&&, tc::ptr_range<char const>>::value);
static_assert(!tc::is_safely_convertible<std::string const&&, tc::ptr_range<char const>>::value);
static_assert(tc::is_safely_convertible<std::string const&, tc::ptr_range<char const>>::value);
static_assert(!tc::is_safely_convertible<char const*, tc::ptr_range<char>>::value);
static_assert(tc::is_safely_convertible<char const*, tc::ptr_range<char const>>::value);
static_assert(tc::is_safely_convertible<char const* &, tc::ptr_range<char const>>::value);
static_assert(tc::is_safely_convertible<char const* &&, tc::ptr_range<char const>>::value);
static_assert(tc::is_safely_convertible<int(&)[3], tc::ptr_range<int const>>::value);
static_assert(tc::is_safely_convertible<int(&)[3], tc::ptr_range<int>>::value);
static_assert(tc::is_safely_convertible<tc::ptr_range<int>, tc::ptr_range<int>>::value);
static_assert(tc::is_safely_convertible<tc::ptr_range<int>, tc::ptr_range<int const>>::value);
static_assert(tc::is_safely_convertible<tc::ptr_range<int>&&, tc::ptr_range<int const>>::value);
static_assert(tc::is_safely_convertible<tc::ptr_range<int>&, tc::ptr_range<int const>>::value);
static_assert(tc::is_safely_convertible<tc::ptr_range<int> const&, tc::ptr_range<int const>>::value);
static_assert(!tc::is_safely_convertible<tc::ptr_range<char const>, decltype(tc::concat("abc", "def"))>::value);
static_assert(!tc::is_safely_convertible<tc::ptr_range<char const>, tc::vector<int>>::value);

static_assert(!tc::is_safely_convertible<std::string&, tc::ptr_range<char>&>::value);
static_assert(!tc::is_safely_convertible<std::string&, tc::ptr_range<char> const&>::value);

#ifdef TC_MAC
static_assert(tc::is_safely_constructible<void (^)(), void (^)()>::value);
static_assert(!tc::is_safely_constructible<void (^)(), int (^)()>::value);
static_assert(!tc::is_safely_constructible<void (^)(), void (^)(int)>::value);
static_assert(tc::is_safely_constructible<void (^)(), nullptr_t>::value);
static_assert(!tc::is_safely_constructible<void (^)(), int*>::value);
static_assert(!tc::is_safely_constructible<void (^)(), void*>::value);

namespace {
	using TVoidFunction = void (*)();
}
static_assert(!tc::is_safely_constructible<void (^)(), TVoidFunction>::value);

static_assert(tc::no_adl::is_objc_block<void (^)()>::value);
static_assert(tc::no_adl::is_objc_block<void (^)(int, char, bool)>::value);
static_assert(!tc::no_adl::is_objc_block<void (*)(int, char, bool)>::value);
#endif

struct A {};
struct B : A {
	operator int(){return 0;}
};

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&&>,
		A&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&>,
		A&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&&, A&&>,
		A&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&, A&&>,
		A const&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&, B&>,
		A&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&&, B&&>,
		A&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&, B&&>,
		A const&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&, A&, A const&>,
		A const&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<A&&, A&&, B&&>,
		A&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char const>>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char const> const>,
		tc::ptr_range<char const> const
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char const> const&>,
		tc::ptr_range<char const> const&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char const>&, tc::ptr_range<char const> const&>,
		tc::ptr_range<char const> const&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char>, tc::ptr_range<char const> const&>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char const>, tc::ptr_range<char const> const&>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char const>, std::string&>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char>, std::string const&>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char>, tc::ptr_range<char>>,
		tc::ptr_range<char>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<char>, tc::ptr_range<char const>>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::vector<char>&, tc::vector<char> const&>,
		tc::vector<char> const&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<int(&)[17], int(&)[17]>,
		int(&)[17]
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<int const(&)[17], int(&)[17]>,
		int const(&)[17]
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<int(&)[17], int(&)[18]>,
		tc::ptr_range<int>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<int const(&)[17], int(&)[18]>,
		tc::ptr_range<int const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<int>, int(&)[19]>,
		tc::ptr_range<int>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<int> const, int(&)[19]>,
		tc::ptr_range<int>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<int(&)[17], int(&)[18], int(&)[19]>,
		tc::ptr_range<int>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<int&, short&>,
		int
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::vector<char>&, std::basic_string<char>&>,
		tc::ptr_range<char>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::vector<char>&, std::basic_string<char> const&>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<const wchar_t (&)[6], tc::ptr_range<wchar_t>&&>,
		tc::ptr_range<wchar_t const>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::vector<char>&&, tc::vector<char>&>,
		tc::vector<char> const&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::ptr_range<int>&&>,
		tc::ptr_range<int>&&
	>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<tc::ptr_range<char>, std::string&&>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<tc::vector<char>&, std::basic_string<char>&&>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<char const*, tc::ptr_range<char>, std::string const&&>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<tc::vector<char>&&, std::basic_string<char>&>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<A, A&&>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<tc::ptr_range<char const>, std::string>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<A, A>>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<A&&, A&&>,
		A&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<A&, A&>,
		A&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<A&, A&&>,
		A const&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<A const&&, A&&>,
		A const&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<A const&, A&>,
		A const&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<B const&, A volatile&>,
		A const volatile&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<B&, A volatile&&>,
		A const volatile&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<B const&&, A volatile&>,
		A const volatile&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<B const&&, B volatile&>,
		B const volatile &&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<tc::vector<char>&, tc::vector<char> const&>,
		tc::vector<char> const&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<int(&)[17], int(&)[17]>,
		int(&)[17]
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<int const(&)[17], int(&)[17]>,
		int const(&)[17]
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<int&, short&>,
		int
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<tc::vector<char>&&, tc::vector<char>&>,
		tc::vector<char> const&&
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<tc::ptr_range<char const>>,
		tc::ptr_range<char const>
	>::value
);

static_assert(
	!tc::has_common_reference_prvalue_as_val<tc::type::list<tc::vector<char>&&, std::string&>>::value
);

static_assert(
	!tc::has_common_reference_prvalue_as_val<tc::type::list<tc::vector<char>&&, std::string&, tc::ptr_range<char const>>>::value
);

static_assert(
	!tc::has_common_reference_prvalue_as_val<tc::type::list<tc::vector<char>, std::string&>>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::vector<int>&, tc::sub_range<tc::vector<int>&>>,
		tc::sub_range<tc::vector<int>&>
	>::value
);

static_assert(
	std::is_same <
		tc::common_reference_xvalue_as_ref_t<tc::vector<int>&, tc::sub_range<tc::vector<int>>&>,
		tc::sub_range<tc::vector<int>&>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_xvalue_as_ref_t<tc::sub_range<tc::vector<int>&>, tc::sub_range<tc::vector<int>>&>,
		tc::sub_range<tc::vector<int>&>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<tc::vector<int>&, tc::sub_range<tc::vector<int>&>>,
		tc::sub_range<tc::vector<int>&>
	>::value
);

static_assert(
	std::is_same <
		tc::common_reference_prvalue_as_val_t<tc::vector<int>&, tc::sub_range<tc::vector<int>>&>,
		tc::sub_range<tc::vector<int>&>
	>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<tc::sub_range<tc::vector<int>&>, tc::sub_range<tc::vector<int>>&>,
		tc::sub_range<tc::vector<int>&>
	>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<tc::vector<int>&, tc::sub_range<tc::vector<int>>>>::value
);

static_assert(
	!tc::has_common_reference_xvalue_as_ref<tc::type::list<tc::vector<int>, tc::sub_range<tc::vector<int>&>>>::value
);

static_assert(
	std::is_same<
		tc::common_reference_prvalue_as_val_t<tc::vector<char>&, std::string&>,
		tc::ptr_range<char>
	>::value
);

namespace {
struct S;
tc::unordered_set<S const*> g_sets;

struct S{
	S() {
		tc::cont_must_emplace(g_sets, this);
	}

	S(S const& other) {
		tc::cont_must_emplace(g_sets, this);
	}

	S(S&& other) {
		tc::cont_must_emplace(g_sets, this);
	}

	~S() {
		tc::cont_must_erase(g_sets, this);
	}

	void foo() & {
		_ASSERT(tc::end(g_sets) != g_sets.find(this));
	}

	void foo() && {
		_ASSERT(tc::end(g_sets) != g_sets.find(this));
	}

	void foo() const & {
		_ASSERT(tc::end(g_sets) != g_sets.find(this));
	}

	void foo() const && {
		_ASSERT(tc::end(g_sets) != g_sets.find(this));
	}

	friend bool operator<(S const& lhs, S const& rhs) {
		_ASSERT(tc::end(g_sets) != g_sets.find(std::addressof(lhs)));
		_ASSERT(tc::end(g_sets) != g_sets.find(std::addressof(rhs)));
		return true;
	}
};

S createS(int) {
	return S{};
}
}

#include "minmax.h"

static_assert(
	std::is_same<
		tc::common_type_t<int, short>,
		int
	>::value
);

static_assert(
	std::is_same<
		tc::common_type_t<tc::size_proxy<int>, short>,
		short
	>::value
);

static_assert(
	std::is_same<
		tc::common_type_t<int, tc::size_proxy<short>>,
		int
	>::value
);

static_assert(
	std::is_same<
		tc::common_type_t<int, tc::size_proxy<short>&>,
		int
	>::value
);

static_assert(
	std::is_same<
		tc::common_type_t<int, tc::size_proxy<short>&&>,
		int
	>::value
);

static_assert(
	std::is_same<
		tc::common_type_t<int, tc::size_proxy<short>, tc::size_proxy<long>>,
		int
	>::value
);

/*
// must not compile
static_assert(
	std::is_same<
		tc::common_type_t<int, unsigned int>,
		unsigned int
	>::value
);
*/

/*
// must not compile
static_assert(
	std::is_same<
		tc::common_type_t<char, char16_t>,
		int
	>::value
);
*/

/*
	must not compile (slicing)
static_assert(
	std::is_same<
		tc::common_type_t<B,A>,
		A
	>::value
);
*/

static_assert(
	std::is_same<
		decltype(tc::min(std::declval<tc::size_proxy<long>>(), std::declval<short>())),
		short
	>::value
);

UNITTESTDEF(minTest) {
	tc::vector<int> vecn;

	static_assert(
		std::is_same<
			tc::common_reference_prvalue_as_val_t<decltype(tc::size(vecn)),short>,
			short
		>::value
	);

	boost::implicit_cast<tc::common_reference_prvalue_as_val_t<decltype(tc::size(vecn)),int>>(tc::size(vecn));

	tc::min(tc::size(vecn),2);

	int a = 3;
	int b = 4;
	_ASSERTEQUAL(tc::min(a,b), 3);
	tc::min(a,b) = 7;
	_ASSERTEQUAL(a, 7);

	_ASSERTEQUAL(tc::min(a,2), 2);
	_ASSERTEQUAL(tc::min(5,b), 4);

	_ASSERTEQUAL(tc::min(1,a,b), 1);

	static_assert(
		std::is_same<
			decltype(tc::min(std::declval<std::int16_t>(), std::declval<std::int32_t>()))
			, std::int32_t
		>::value
	);

	static_assert(
		std::is_same<
			decltype(tc::min(std::declval<std::uint16_t>(), std::declval<std::int32_t>()))
			, std::int32_t
		>::value
	);

	{
		S s2[2];

		tc::for_each(
			tc::make_range_of_iterators(
				tc::transform(
					tc::transform(
						tc::iota(0,1),
						[&](int n) noexcept -> S&& {
							return std::move(s2[n]);
						}
					),
					tc::fn_min()
				)
			),
			[&](auto it) noexcept {
				auto_cref( elem, *it );
				boost::ignore_unused(elem);
			}
		);
	}

	{
		S s;
		tc::min(s,S{}).foo();
	}


	tc::projected(tc::fn_min(), &createS)(0,1).foo();


	{
		S s2[2];
		tc::projected(
			tc::fn_min(),
			[&](int n) noexcept -> S&& {
				return std::move(s2[n]);
			}
		)(0,1).foo();
	}

	{
		tc::projected(
			tc::fn_min(),
			[](S&& s) noexcept ->S&& {
				return static_cast<S&&>(s);
			}
		)(createS(1), createS(2));
	}

	_ASSERT(tc::empty(g_sets));

	static_assert(
		std::is_same<
			decltype(tc::min(std::declval<int>(),std::declval<long>())),
			tc::common_type_t<int,long>
		>::value
	);

	static_assert(
		std::is_same<
			decltype(tc::min(std::declval<long>(),std::declval<int>())),
			tc::common_type_t<long,int>
		>::value
	);

	static_assert(
		std::is_same<
			decltype(tc::min(std::declval<unsigned long>(),std::declval<unsigned int>())),
			unsigned long
		>::value
	);

	static_assert(
		std::is_same<
			decltype(tc::min(std::declval<unsigned int>(),std::declval<unsigned long>())),
			unsigned long
		>::value
	);
}

static_assert(!tc::is_safely_constructible<std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>, wchar_t const* const&, wchar_t const* const&>::value);
