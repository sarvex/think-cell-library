
// think-cell public library
//
// Copyright (C) 2016-2021 think-cell Software GmbH
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#include "range.h"
#include "container.h" // tc::vector
#include "subrange.h"
#include "union_adaptor.h"
#include "range.t.h"
#include "unique_range_adaptor.h"
#include "intersection_adaptor.h"
#include "interval.h"

#ifdef TC_PRIVATE
#include "Library/Utilities/Hash.h"
#endif

#include <type_traits>

namespace {
	STATICASSERTSAME(tc::make_subrange_result_t<tc::ptr_range<char const>&>, tc::ptr_range<char const>);

	using SRVI = tc::make_subrange_result_t<tc::vector<int>&>;
	using CSRVI = tc::make_subrange_result_t<tc::vector<int> const&>;

	using SSRVI = tc::make_subrange_result_t<tc::make_subrange_result_t<tc::vector<int>&>>;
	using CSSRVI = tc::make_subrange_result_t<tc::make_subrange_result_t<tc::vector<int> const&>>;

	UNITTESTDEF( subrange_array ) {

		int arr[4] = {1,2,3,4};
		auto arr_rng = tc::slice_by_interval(arr, tc::make_interval(1, 3));

		void(tc::implicit_cast<tc::subrange<tc::iterator_base<int *>>>(arr_rng));
		void(tc::implicit_cast<tc::subrange<tc::iterator_base<int const*>>>(arr_rng));
	}


	UNITTESTDEF( const_subrange_char_ptr ) {
		tc::subrange<tc::iterator_base<char const*>>{"test"};
	}

	//-------------------------------------------------------------------------------------------------------------------------

	using SRVI = tc::make_subrange_result_t<tc::vector<int>&>;
	using CSRVI = tc::make_subrange_result_t<tc::vector<int> const&>;

	using SSRVI = tc::make_subrange_result_t<tc::make_subrange_result_t<tc::vector<int>&>>;
	using CSSRVI = tc::make_subrange_result_t<tc::make_subrange_result_t<tc::vector<int> const&>>;

	[[maybe_unused]] void ref_test(SRVI & rng) noexcept {
		CSRVI const_rng(rng);
		SRVI non_const_rng(rng);
		UNUSED_TEST_VARIABLE(const_rng);
		UNUSED_TEST_VARIABLE(non_const_rng);
	}

	UNITTESTDEF( const_subrange ) {
		tc::vector<int> v;
		auto srvi = tc::slice(v);
		auto csrvi = tc::slice(tc::as_const(v));

		(void) srvi;
		(void) csrvi;

		SRVI non_const_rng(srvi);
		UNUSED_TEST_VARIABLE(non_const_rng);
		CSRVI const_rng(srvi);
		UNUSED_TEST_VARIABLE(const_rng);
	}

	UNITTESTDEF( sub_subrange_rvalue ) {

		tc::vector<int> v;

		auto srvi = tc::slice(v);
		auto csrvi = tc::slice(tc::as_const(v));

		auto ssrvi = tc::slice(tc::slice(v));
		auto cssrvi = tc::slice(tc::slice(tc::as_const(v)));

		STATICASSERTSAME(decltype(ssrvi), decltype(srvi), "Sub-sub-range does not flatten to sub-range (decltype)");
		STATICASSERTSAME(decltype(cssrvi), decltype(csrvi), "const sub-sub-range does not flatten to const sub-range (decltype)");
	}

	UNITTESTDEF( sub_subrange_lvalue ) {

		tc::vector<int> v;

		auto srvi = tc::slice(v);
		auto csrvi = tc::slice(tc::as_const(v));

		auto ssrvi = tc::slice(srvi);
		auto cssrvi = tc::slice(csrvi);

		// sanity checks
		STATICASSERTSAME(decltype(srvi), SRVI, "make_subrange_result gives wrong result");
		STATICASSERTSAME(decltype(csrvi), CSRVI, "make_subrange_result gives wrong result");
		STATICASSERTSAME(decltype(ssrvi), SSRVI, "make_subrange_result gives wrong result");
		STATICASSERTSAME(decltype(cssrvi), CSSRVI, "make_subrange_result gives wrong result");

		// the actual test
		STATICASSERTSAME(decltype(ssrvi), decltype(srvi), "Sub-sub-range does not flatten to sub-range");
		STATICASSERTSAME(decltype(cssrvi), decltype(csrvi), "const sub-sub-range does not flatten to const sub-range");
	}

	UNITTESTDEF( sub_subrange_index ) {

		TEST_init_hack(tc::vector, int, v, {1,2,3,4,5,6,7,8,9});
		TEST_init_hack(tc::vector, int, exp36, {4,5,6});

		auto sr = tc::slice(v);
		auto csr = tc::slice(tc::as_const(v));

		// use range_difference to specify bounds
		auto ssr1 = tc::slice_by_interval(sr, tc::make_interval(3, 6));
		auto cssr1 = tc::slice_by_interval(csr, tc::make_interval(3, 6));

		STATICASSERTSAME(decltype(ssr1), decltype(sr), "Sub-sub-range does not flatten to sub-range");
		STATICASSERTSAME(decltype(cssr1), decltype(csr), "const sub-sub-range does not flatten to const sub-range");

	}

	UNITTESTDEF( sub_subrange_crazy_manual ) {

		tc::vector<int> v;

		// don't try that at work! - Seriously: subrange is build to work correctly in a variety of situations,
		// but normally it should not be necessary to manually utter the type. Use slice and make_subrange_result instead.
		// if you do need to say the type (e.g. when deducting sth.)  ...
		//_ASSERT(false);
		tc::subrange<tc::vector<int>&>{v}; // create a sub range from unrelated range
		static_cast<void>(tc::subrange<tc::vector<int>&>(tc::subrange<tc::vector<int>&>(v))); // copy sub range
	}

	UNITTESTDEF(union_range_tests) {
		TEST_RANGE_EQUAL(
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3,4)),
			tc::union_range(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,4)),
				as_constexpr(tc::make_array(tc::aggregate_tag, 2,3))
			)
		);

		TEST_RANGE_EQUAL(
			as_constexpr(tc::make_array(tc::aggregate_tag, 4,3,2,1,0)),
			tc::union_range(
				as_constexpr(tc::make_array(tc::aggregate_tag, 4,2,0)),
				as_constexpr(tc::make_array(tc::aggregate_tag, 3,2,1)),
				[](int lhs, int rhs) noexcept {return tc::compare(rhs,lhs);}
			)
		);

		_ASSERTEQUAL(
			*tc::upper_bound<tc::return_border>(
				tc::union_range(
					as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,4)),
					as_constexpr(tc::make_array(tc::aggregate_tag, 2,3))
				),
				2
			),
			3
		);

		{
			auto rng = tc::union_range(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3,3,4,4,5)),
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,1,1,3,4,4,4))
			);

			TEST_RANGE_EQUAL(
				rng,
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,1,1,2,3,3,4,4,4,5))
			);

			{
				auto it = tc::lower_bound<tc::return_border>(rng,3);
				_ASSERTEQUAL(*it,3);
				_ASSERTEQUAL(*boost::prior(it), 2);
				_ASSERTEQUAL(*modified(it, ++_), 3);
	}
			{
				auto it = tc::upper_bound<tc::return_border>(rng,1);
				_ASSERTEQUAL(*it,2);
				_ASSERTEQUAL(*--it, 1);
				_ASSERTEQUAL(*--it, 1);
				_ASSERTEQUAL(*--it, 1);
			}
		}
		{
			auto rng = tc::union_range(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,1,1,1,1,1,1,1,1)),
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,1))
			);
			auto it = tc::lower_bound<tc::return_border>(rng,1);
			_ASSERTEQUAL(*it,1);
		}
	}

#ifdef _CHECKS
	struct GenRange final {
		using reference=int;
		using const_reference=int;
		template<typename Func>
		auto operator()(Func func) const& noexcept -> tc::break_or_continue {
			RETURN_IF_BREAK(func(1));
			RETURN_IF_BREAK(func(3));
			RETURN_IF_BREAK(func(5));
			return tc::continue_;
		}
	};

	struct GenRangeMutable final {
		using reference=double;
		using const_reference=double;
		tc::vector<double> m_vecf = tc::vector<double>({1,3,5});

		template<typename Func>
		auto operator()(Func func) & noexcept -> tc::break_or_continue {
			return tc::for_each(m_vecf, std::ref(func));
		}

		template<typename Func>
		auto operator()(Func func) const& noexcept -> tc::break_or_continue {
			return tc::for_each(m_vecf, std::ref(func));
		}
	};


#endif

	UNITTESTDEF(union_range_generator) {
		TEST_RANGE_EQUAL(
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3,5,6)),
			tc::union_range(
				GenRange(),
				as_constexpr(tc::make_array(tc::aggregate_tag, 2,6))
			)
		);

		GenRangeMutable rngMut;
		tc::vector<double> vecf;
		tc::for_each(
			tc::union_range(
				rngMut,
				vecf
			),
			[](double& d) noexcept {d += 0.1;}
		);

		TEST_RANGE_EQUAL(
			as_constexpr(tc::make_array(tc::aggregate_tag, 1.1,3.1,5.1)),
			rngMut
		);



	}

	UNITTESTDEF(subrange_with_tranform) {

		tc::vector<int> vecn{1,2,3};
		auto rgntrnsfn = tc::transform(vecn, [](int n) noexcept {return n*n;});

		TEST_RANGE_EQUAL(
			tc::slice(rgntrnsfn, tc::begin(rgntrnsfn), tc::end(rgntrnsfn)),
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,4,9))
		);

		TEST_RANGE_EQUAL(
			as_constexpr(tc::make_array(tc::aggregate_tag,  1, 2, 3 )),
			tc::untransform(tc::slice(rgntrnsfn, tc::begin(rgntrnsfn), tc::end(rgntrnsfn)))
		);

		TEST_RANGE_EQUAL(
			tc::single(2),
			tc::untransform(tc::equal_range(tc::transform(vecn, [](int n) noexcept {return n*n; }), 4))
		);

		{
			auto rng = tc::transform(tc::filter(vecn, [](int n) noexcept {return n<3;}), [](int n) noexcept {return n*n; });

			TEST_RANGE_EQUAL(
				tc::single(2),
				tc::untransform(tc::equal_range(rng, 4))
			);
		}

		{
			// r-value transform with l-value range
			auto&& rng = tc::untransform(tc::transform(vecn, [](int n) noexcept {return n*n;}));
			_ASSERTEQUAL(tc::size(vecn), 3);
			_ASSERTEQUAL(std::addressof(*tc::begin(rng)), std::addressof(*tc::begin(vecn)));
			static_assert(std::is_lvalue_reference<decltype(rng)>::value);
			// rewrite the following line to ::value once VC++ compiler bug is resolved https://developercommunity.visualstudio.com/content/problem/1088659/class-local-to-function-template-erroneously-cant.html
			static_assert(!std::is_const_v<std::remove_reference_t<decltype(rng)>>);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}

		{
			// r-value transform with l-value const range
			auto&& rng = tc::untransform(tc::transform(tc::as_const(vecn), [](int n) noexcept {return n*n;}));
			_ASSERTEQUAL(tc::size(vecn), 3);
			_ASSERTEQUAL(std::addressof(*tc::begin(rng)), std::addressof(*tc::begin(vecn)));
			static_assert(std::is_lvalue_reference<decltype((rng))>::value);
			static_assert(std::is_const<std::remove_reference_t<decltype(rng)>>::value);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}

		{
			// r-value transform with r-value range
			auto rng = tc::untransform(tc::transform(tc::vector<int>{1, 2, 3}, [](int n) noexcept {return n*n;}));
			STATICASSERTSAME(decltype(rng),tc::vector<int>);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}

		{
			// l-value transform of l-value-range
			auto trnsfrng = tc::transform(vecn, [](int n) noexcept {return n*n;});

			auto&& rng = tc::untransform(trnsfrng);
			_ASSERTEQUAL(tc::size(vecn), 3);
			_ASSERTEQUAL(std::addressof(*tc::begin(rng)), std::addressof(*tc::begin(vecn)));
			static_assert(std::is_lvalue_reference<decltype(rng)>::value);
			// rewrite the following line to ::value once VC++ compiler bug is resolved https://developercommunity.visualstudio.com/content/problem/1088659/class-local-to-function-template-erroneously-cant.html
			static_assert(!std::is_const_v<std::remove_reference_t<decltype(rng)>>);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}

		{
			// l-value transform of const l-value-range
			auto const trnsfrng = tc::transform(tc::as_const(vecn), [](int n) noexcept {return n*n;});

			auto&& rng = tc::untransform(trnsfrng);
			_ASSERTEQUAL(tc::size(vecn), 3);
			_ASSERTEQUAL(std::addressof(*tc::begin(rng)), std::addressof(*tc::begin(vecn)));
			static_assert(std::is_lvalue_reference<decltype(rng)>::value);
			static_assert(std::is_const<std::remove_reference_t<decltype(rng)>>::value);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}

		{
			// l-value transform of const l-value-range
			auto trnsfrng = tc::transform(tc::as_const(vecn), [](int n) noexcept {return n*n;});

			auto&& rng = tc::untransform(trnsfrng);
			_ASSERTEQUAL(tc::size(vecn), 3);
			_ASSERTEQUAL(std::addressof(*tc::begin(rng)), std::addressof(*tc::begin(vecn)));
			static_assert(std::is_lvalue_reference<decltype(rng)>::value);
			static_assert(std::is_const<std::remove_reference_t<decltype(rng)>>::value);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}

		{
			// l-value transform of r-value range
			auto trnsfrng = tc::transform(tc::vector<int>{1, 2, 3}, [](int n) noexcept {return n*n; });
			auto&& rng = tc::untransform(trnsfrng);
			_ASSERTEQUAL(
				std::addressof(*tc::begin(trnsfrng).element_base()),
				std::addressof(*tc::begin(rng))
			);
			static_assert(std::is_lvalue_reference<decltype(rng)>::value);
			// rewrite the following line to ::value once VC++ compiler bug is resolved https://developercommunity.visualstudio.com/content/problem/1088659/class-local-to-function-template-erroneously-cant.html
			static_assert(!std::is_const_v<std::remove_reference_t<decltype(rng)>>);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}
		{
			// const l-value transform of r-value range
			auto const trnsfrng = tc::transform(tc::vector<int>{1, 2, 3}, [](int n) noexcept {return n*n; });
			auto&& rng = tc::untransform(trnsfrng);
			_ASSERTEQUAL(
				std::addressof(*tc::begin(trnsfrng).element_base()),
				std::addressof(*tc::begin(rng))
			);
			static_assert(std::is_lvalue_reference<decltype(rng)>::value);
			static_assert(std::is_const<std::remove_reference_t<decltype(rng)>>::value);
			TEST_RANGE_EQUAL(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3)),
				rng
			);
		}
	}

	UNITTESTDEF(Unique_Ranges) {
		tc::vector<int> vecn{1,2,3,3,5,5,7};

		{
			std::initializer_list<int> const rngExpected[] = { // extends life-time of underlying arrays
				{1},
				{2},
				{3,3},
				{5,5},
				{7}
			};

			{
				auto it = tc::begin(rngExpected);
				tc::for_each(
					tc::adjacent_unique_range(vecn),
					[&](auto const& subrng) noexcept {
						TEST_RANGE_EQUAL(subrng, *it);
						++it;
					}
				);
				_ASSERTEQUAL(tc::end(rngExpected), it);
			}

			{
				auto it = tc::begin(rngExpected);
				tc::for_each(
					tc::adjacent_unique_range(tc::as_const(vecn)),
					[&](auto const& subrng) noexcept {
						TEST_RANGE_EQUAL(subrng, *it);
						++it;
					}
				);
				_ASSERTEQUAL(tc::end(rngExpected), it);
			}

			{
				auto it = tc::begin(rngExpected);
				tc::for_each(
				tc::adjacent_unique_range(vecn),
					[&](auto const& subrng) noexcept {
						TEST_RANGE_EQUAL(subrng, *it);
						++it;
					}
				);
				_ASSERTEQUAL(tc::end(rngExpected), it);
			}

		}

		auto Pred = [](int lhs, int rhs) noexcept {
			return std::abs(lhs-rhs) <=1;
		};

		{
			std::initializer_list<int> const rngExpected[] = { // extends life-time of underlying arrays
				{1,2},
				{3,3},
				{5,5},
				{7}
			};
			auto it = tc::begin(rngExpected);
			tc::for_each(
				tc::front_unique_range(vecn, Pred),
				[&](auto const& subrng) noexcept {
					TEST_RANGE_EQUAL(subrng, *it);
					++it;
				}
			);
			_ASSERTEQUAL(tc::end(rngExpected), it);
		}

		{
			std::initializer_list<int> const rngExpected[] = { // extends life-time of underlying arrays
				{1,2,3,3},
				{5,5},
				{7}
			};
			auto it = tc::begin(rngExpected);
			tc::for_each(
				tc::adjacent_unique_range(vecn, Pred),
				[&](auto const& subrng) noexcept {
					TEST_RANGE_EQUAL(subrng, *it);
					++it;
				}
			);
			_ASSERTEQUAL(tc::end(rngExpected), it);
		}
	}

	UNITTESTDEF(difference_range) {
		TEST_RANGE_EQUAL(
			tc::difference(
				as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,2,4,7,8,8,11,11)),
				as_constexpr(tc::make_array(tc::aggregate_tag, 2,3,4,8,8))
			),
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,7,11,11))
		);
	}

	UNITTESTDEF(difference_unordered_set) {
		tc::unordered_set<int> setn1;
		tc::cont_assign(setn1, as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3,4,5,6,7,8,9,10,11)));

		tc::unordered_set<int> setn2;
		tc::cont_assign(setn2, as_constexpr(tc::make_array(tc::aggregate_tag, 10,9,8,7,6,5,4,3,2,1)));

		TEST_RANGE_EQUAL(
			tc::set_difference(setn1, setn2),
			tc::single(11)
		);

		tc::vector<int> vecn;
		tc::for_each(tc::set_intersect(setn1, setn2), [&](int n) noexcept {
			vecn.emplace_back(n);
		});

		TEST_RANGE_EQUAL(
			tc::sort(vecn),
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3,4,5,6,7,8,9,10))
		);
	}

	UNITTESTDEF(tc_unique) {
		TEST_RANGE_EQUAL(
			tc::adjacent_unique(as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,2,4,7,8,8,11,11))),
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,4,7,8,11))
		);

		tc::vector<int> vecn{1,1,2,2};
		auto rngunique = tc::adjacent_unique(vecn);
		auto it = tc::begin(rngunique);
		auto itvec = it.element_base();
		_ASSERT(1==*itvec);
		_ASSERT(1 == *(++itvec));
		_ASSERT(2 == *(++itvec));

		_ASSERT(1 == *it);
		_ASSERT(2 == *(++it));
		_ASSERT(1 == *(--it));
		_ASSERTEQUAL(tc::begin(rngunique), it);

	}

#ifdef _CHECKS
	struct MovingInt final {
		int m_n;

		MovingInt(int n) noexcept : m_n(n)
		{}

		MovingInt(MovingInt&& other) noexcept : m_n(other.m_n)
		{
			other.m_n = 0;
		}

		MovingInt(MovingInt const&) = delete;
		MovingInt& operator=(MovingInt& other) = delete;

		MovingInt& operator=(MovingInt&& other) & noexcept {
			m_n = other.m_n;
			if (&other.m_n != &m_n) {
				other.m_n = 0;
			}
			return *this;
		}

		operator int() const& noexcept {return m_n;}
	};
#endif

	UNITTESTDEF(tc_unique_inplace) {
		tc::vector<MovingInt> vecmn; // list initializtion not possible with move-only element type
		tc::cont_emplace_back(vecmn, 1);
		tc::cont_emplace_back(vecmn, 1);
		tc::cont_emplace_back(vecmn, 1);
		tc::cont_emplace_back(vecmn, 2);
		tc::cont_emplace_back(vecmn, 2);
		tc::cont_emplace_back(vecmn, 2);
		tc::cont_emplace_back(vecmn, 3);
		tc::cont_emplace_back(vecmn, 4);
		tc::cont_emplace_back(vecmn, 4);
		tc::adjacent_unique_inplace(vecmn);

		TEST_RANGE_EQUAL(
			vecmn,
			as_constexpr(tc::make_array(tc::aggregate_tag, 1,2,3,4))
		);
	}

#ifdef TC_PRIVATE
	UNITTESTDEF(hash_string_range) {
		std::basic_string<char> strNarrow = "test";
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, char>(strNarrow)), (tc::hash_range<SHash::hash64_t, char>(tc::as_pointers(strNarrow))));
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, char>(strNarrow)), (tc::hash_range<SHash::hash64_t, char>("test")));
		std::basic_string<tc::char16> strWide = UTF16("test");
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, tc::char16>(strWide)), (tc::hash_range<SHash::hash64_t, tc::char16>(tc::as_pointers(strWide))));
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, tc::char16>(strWide)), (tc::hash_range<SHash::hash64_t, tc::char16>(UTF16("test"))));
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, tc::char16>(tc::make_vector(strWide))), (tc::hash_range<SHash::hash64_t, tc::char16>(tc::as_pointers(strWide))));
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, char>(strNarrow)), (tc::hash<SHash::hash64_t, std::basic_string<char>>(strNarrow)));
		_ASSERTEQUAL((tc::hash_range<SHash::hash64_t, tc::char16>(strWide)), (tc::hash<SHash::hash64_t, std::basic_string<tc::char16>>(strWide)));
	}
#endif

	UNITTESTDEF( take_first_sink ) {
		auto const rngn = [](auto sink) noexcept {
			for(int i=0;;++i) { RETURN_IF_BREAK(tc::continue_if_not_break(sink, i)); }
		};

		_ASSERTEQUAL(tc::for_each(tc::begin_next<tc::return_take>(rngn), [](int) noexcept {}), tc::continue_);
		_ASSERTEQUAL(tc::for_each(tc::begin_next<tc::return_take>(rngn), [](int) noexcept { return tc::break_; }), tc::break_);
		_ASSERT(tc::equal(as_constexpr(tc::make_array(tc::aggregate_tag, 0,1,2,3)), tc::begin_next<tc::return_take>(rngn, 4)));

		auto rngch = tc::concat("123", [](auto&& sink) noexcept { return tc::for_each("456", std::forward<decltype(sink)>(sink)); });

		struct assert_no_single_char_sink /*final*/ {
			auto operator()(char) const& noexcept { _ASSERTFALSE; return tc::construct_default_or_terminate<tc::break_or_continue>(); }
			auto chunk(tc::ptr_range<char const> str) const& noexcept { return INTEGRAL_CONSTANT(tc::continue_)(); }
		};
		_ASSERTEQUAL(tc::for_each(tc::begin_next<tc::return_take>(rngch, 4), assert_no_single_char_sink()), tc::continue_);
		_ASSERT(tc::equal("1234", tc::begin_next<tc::return_take>(rngch, 4)));
	}

	UNITTESTDEF( drop_first_sink ) {
		auto const rngn = [](auto sink) noexcept {
			for(int i=0; i < 7;++i) { RETURN_IF_BREAK(tc::continue_if_not_break(sink, i)); }
			return tc::continue_;
		};

		_ASSERTEQUAL(tc::for_each(tc::begin_next<tc::return_drop>(rngn), [](int) noexcept {}), tc::continue_);
		_ASSERTEQUAL(tc::for_each(tc::begin_next<tc::return_drop>(rngn), [](int) noexcept { return tc::break_; }), tc::break_);
		_ASSERT(tc::equal(as_constexpr(tc::make_array(tc::aggregate_tag, 4,5,6)), tc::begin_next<tc::return_drop>(rngn, 4)));

		auto rngch = tc::begin_next<tc::return_drop>(tc::concat("123", [](auto&& sink) noexcept { return tc::for_each("456", std::forward<decltype(sink)>(sink)); }), 2);

		struct assert_no_single_char_sink /*final*/ {
			auto operator()(char) const& noexcept { _ASSERTFALSE; return tc::construct_default_or_terminate<tc::break_or_continue>(); }
			auto chunk(tc::ptr_range<char const> str) const& noexcept { return INTEGRAL_CONSTANT(tc::continue_)(); }
		};
		_ASSERTEQUAL(tc::for_each(rngch, assert_no_single_char_sink()), tc::continue_);
		_ASSERT(tc::equal("3456", rngch));
	}

	UNITTESTDEF( front ) {
		_ASSERTEQUAL(tc::front<tc::return_value>(tc::single(1)), 1);
		_ASSERTEQUAL(*tc::front<tc::return_element>(tc::single(1)), 1);
		_ASSERTEQUAL(tc::front<tc::return_value_or_none>(tc::make_empty_range<int>()), std::nullopt);
	}

	UNITTESTDEF( ptr_range_from_subrange ) {
		std::basic_string<char> str("1234");
		_ASSERTEQUAL( tc::begin(tc::as_pointers(tc::begin_next<tc::return_drop>(str, 2))), tc::begin(tc::begin_next<tc::return_drop>(tc::as_pointers(str), 2)) );
		_ASSERTEQUAL( tc::end(tc::as_pointers(tc::begin_next<tc::return_take>(str, 2))), tc::end(tc::begin_next<tc::return_take>(tc::as_pointers(str), 2)) );
	}
#ifndef __clang__
	namespace {
		struct STestPtrs {
			constexpr STestPtrs(char const* itBegin,char const* itEnd): m_itBegin(itBegin),m_itEnd(itEnd) {}

			char const* m_itBegin;
			char const* m_itEnd;
		};
	}

	UNITTESTDEF( constexpr_ptr_to_string_literal_runtime_bug ) {
		// https://developercommunity.visualstudio.com/content/problem/900648/codegen-constexpr-pointer-to-trailing-zero-on-stri.html
		// Visual Studio 2017/2019 will fire at run time if we disable string pooling by setting /GF-.
		constexpr char const* str = "abcde";
		constexpr STestPtrs ptrs(tc::begin(str), tc::end(str));
		static_assert(tc::begin(str)==ptrs.m_itBegin);
		static_assert(tc::end(str)==ptrs.m_itEnd);
		_ASSERTEQUAL(tc::begin(str), ptrs.m_itBegin); // run time compare
		_ASSERTEQUAL(tc::end(str), ptrs.m_itEnd);  // run time compare
	}
#endif
}

STATICASSERTEQUAL( sizeof(tc::ptr_range<int>), 2 * sizeof(int*) );
static_assert( std::is_nothrow_default_constructible<tc::ptr_range<int>>::value );
static_assert( std::is_trivially_destructible<tc::ptr_range<int>>::value );
static_assert( std::is_trivially_copy_constructible<tc::ptr_range<int>>::value );
static_assert( std::is_trivially_move_constructible<tc::ptr_range<int>>::value );
static_assert( std::is_trivially_copy_assignable<tc::ptr_range<int>>::value );
static_assert( std::is_trivially_move_assignable<tc::ptr_range<int>>::value );
static_assert( std::is_trivially_copyable<tc::ptr_range<int>>::value );

namespace {
	UNITTESTDEF( subrange_index_translation ) {
		TEST_RANGE_EQUAL("est", []() noexcept {
			std::basic_string<char> str = "Test";
			return tc::begin_next<tc::return_drop>(tc_move(str));
		}());
	}
}