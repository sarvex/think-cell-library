
// think-cell public library
//
// Copyright (C) 2016-2021 think-cell Software GmbH
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include "assert_defs.h"
#include "range_fwd.h"
#include "range_adaptor.h"
#include "index_range.h"
#include "meta.h"
#include "size.h"
#include "utility.h"
#include "invoke_with_constant.h"
#include "accumulate.h"
#include "transform.h"
#include "counting_range.h"
#include "variant.h"
#include "quantifier.h"
#include "empty.h"
#include "tuple.h"

namespace tc {
	namespace no_adl {
		template<typename TypeListRng, typename Enable=void>
		struct has_concat_iterator final: std::false_type {};

		template<typename TypeListRng>
		struct has_concat_iterator<TypeListRng, std::enable_if_t<tc::type::all_of<TypeListRng, tc::is_range_with_iterators>::value>> final : std::integral_constant<bool,
			tc::has_common_reference_prvalue_as_val<tc::type::transform_t<TypeListRng, tc::range_reference_t>>::value
		> {};
	}

	namespace concat_adaptor_adl {
		template<
			bool HasIterator,
			typename... Rng
		>
		struct concat_adaptor_impl;
	}

	template<typename... Rng>
	using concat_adaptor = concat_adaptor_adl::concat_adaptor_impl< no_adl::has_concat_iterator<tc::type::list<Rng...>>::value, Rng... >;


	namespace no_adl {
		template<typename Rng>
		struct is_concat_range final: std::false_type {};

		template<typename ... Rng>
		struct is_concat_range<tc::concat_adaptor<Rng ...>> final: std::true_type {};
	}
	using no_adl::is_concat_range;

	namespace concat_detail {
		namespace no_adl {
			template<typename ConcatAdaptor, typename Sink, typename = tc::remove_cvref_t<ConcatAdaptor>>
			struct has_for_each;

			template<typename ConcatAdaptor, typename Sink, typename... Rng>
			struct has_for_each<ConcatAdaptor, Sink, tc::concat_adaptor<Rng...>> : std::bool_constant<
				(tc::has_for_each<tc::apply_cvref_t<Rng, ConcatAdaptor>, tc::decay_t<Sink> const&>::value && ...)
			> {};

			template<typename Sink>
			struct sink {
				static_assert(tc::is_decayed<Sink>::value);
				using guaranteed_break_or_continue = guaranteed_break_or_continue_t<Sink>;
				Sink m_sink;

				template<typename RngAdaptor>
				constexpr auto operator()(RngAdaptor&& rngadaptor) const& return_MAYTHROW(
					tc::for_each(std::forward<RngAdaptor>(rngadaptor).base_range(), m_sink)
				)
			};
		}

		template<typename Sink>
		constexpr auto make_sink(Sink&& sink) noexcept { // not inline in for_each_impl because of MSVC
			return no_adl::sink<tc::decay_t<Sink>>{std::forward<Sink>(sink)};
		}
	}

	namespace concat_adaptor_adl {
		template<
			typename... Rng
		>
		struct [[nodiscard]] concat_adaptor_impl<false, Rng...>
		{
			static_assert(1<sizeof...(Rng)); // singleton range will be forwarded instead of packed into concat_adaptor
		//protected: // public because for_each_impl is non-friend for _MSC_VER
			tc::tuple<tc::range_adaptor_base_range<Rng>...> m_tupleadaptbaserng;

		public:
			template<typename... Rhs>
			constexpr concat_adaptor_impl(tc::aggregate_tag_t, Rhs&&... rhs) noexcept
				: m_tupleadaptbaserng{{ {{tc::aggregate_tag, std::forward<Rhs>(rhs)}}... }}
			{}

			using index_seq = std::make_index_sequence<sizeof...(Rng)>;

			template<typename ConcatRng, std::size_t... I>
			friend constexpr auto forward_base_ranges_as_tuple(ConcatRng&& rng, std::index_sequence<I...>) noexcept;

		protected:
			template<typename IntConstant>
			auto BaseRangeSize(IntConstant nconstIndex) const& noexcept {
				return tc::size_raw(tc::get<nconstIndex()>(m_tupleadaptbaserng).base_range());
			}
		public:
			template< ENABLE_SFINAE, std::enable_if_t<std::conjunction<tc::has_size<SFINAE_TYPE(Rng)>...>::value>* = nullptr >
			auto size() const& noexcept {
				return 
					tc::accumulate(
						tc::transform(
							std::index_sequence_for<Rng...>(),
							[&](auto nconstIndex) noexcept { return BaseRangeSize(nconstIndex); }
						),
						tc::explicit_cast<tc::common_type_t<decltype(tc::size_raw(std::declval<Rng>()))...>>(0),
						tc::fn_assign_plus()
					);
			}

			bool empty() const& noexcept {
				return tc::all_of(m_tupleadaptbaserng, [](auto const& adaptbaserng) noexcept { return tc::empty(adaptbaserng.base_range()); });
			}
		};

		// Using return_decltype_MAYTHROW here and in concat_detail::no_adl::sink::operator() results
		// in MSVC 19.15 error C1202: Recursive type or function dependency context too complex.
		template<typename Self, typename Sink, typename std::enable_if_t<concat_detail::no_adl::has_for_each<Self, Sink>::value>* = nullptr>
		constexpr auto for_each_impl(Self&& self, Sink&& sink) return_MAYTHROW(
			tc::for_each(
				std::forward<Self>(self).m_tupleadaptbaserng,
				concat_detail::make_sink(std::forward<Sink>(sink))
			)
		)

		template<typename Self, typename Sink, typename std::enable_if_t<tc::is_concat_range<tc::remove_cvref_t<Self>>::value>* = nullptr>
		constexpr auto for_each_reverse_impl(Self&& self, Sink&& sink) MAYTHROW {
			return tc::for_each(
				tc::reverse(std::forward<Self>(self).m_tupleadaptbaserng),
				[&](auto&& rngadaptor) MAYTHROW {
					return tc::for_each(tc::reverse(tc_move_if_owned(rngadaptor).base_range()), sink);
				}
			);
		}

		template<typename ConcatRng, std::size_t... I>
		[[nodiscard]] constexpr auto forward_base_ranges_as_tuple(ConcatRng&& rng, std::index_sequence<I...>) noexcept {
			return tc::forward_as_tuple(tc::get<I>(std::forward<ConcatRng>(rng).m_tupleadaptbaserng).base_range()...);
		}

		struct concat_end_index final : tc::equality_comparable<concat_end_index> {
			friend bool operator==(concat_end_index const&, concat_end_index const&) noexcept { return true; }
		};

		template<
			typename... Rng
		>
		struct [[nodiscard]] concat_adaptor_impl<true, Rng...> :
			concat_adaptor_impl<false, Rng...>,
			tc::range_iterator_from_index<
				concat_adaptor_impl<true, Rng...>,
				std::variant<
					tc::index_t<std::remove_reference_t<
						Rng
					>>...,
					concat_end_index
				>
			>
		{
		private:
			using this_type = concat_adaptor_impl;

		public:
			using typename this_type::range_iterator_from_index::index;
			static constexpr bool c_bHasStashingIndex=std::disjunction<tc::has_stashing_index<std::remove_reference_t<Rng>>...>::value;

			using difference_type = tc::common_type_t<typename boost::range_difference<Rng>::type...>;

			using concat_adaptor_impl<false, Rng...>::concat_adaptor_impl;

		private:
			template<std::size_t Index>
			index create_begin_index(std::integral_constant<std::size_t, Index>) const& noexcept {
				static_assert(0 <= Index && Index <= sizeof...(Rng));
				if constexpr (sizeof...(Rng) == Index) {
					return index(std::in_place_index<Index>, concat_end_index());
				} else {
					return index(std::in_place_index<Index>, tc::get<Index>(this->m_tupleadaptbaserng).base_begin_index());
				}
			}

			template<int Index>
			index create_end_index(std::integral_constant<int, Index>) const& noexcept {
				static_assert(0 <= Index);
				static_assert(Index < sizeof...(Rng));
				return index(std::in_place_index<Index>, tc::get<Index>(this->m_tupleadaptbaserng).base_end_index());
			}

			template<int IndexFrom>
			void correct_index(index& idx) const& noexcept {
				tc::for_each(
					tc::make_integer_sequence<std::size_t, IndexFrom, sizeof...(Rng)>(),
					[&](auto nconstIndex) noexcept {
						if (tc::at_end_index( tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(idx))) {
							idx = create_begin_index(std::integral_constant<std::size_t, nconstIndex() + 1>());
							return tc::continue_;
						} else {
							return tc::break_;
						}
					}
				);
			}

			STATIC_FINAL(begin_index)() const& noexcept -> index {
				return modified(
					create_begin_index(std::integral_constant<std::size_t, 0>()),
					correct_index<0>(_)
				);
			}

			STATIC_FINAL(end_index)() const& noexcept -> index {
				return create_begin_index(std::integral_constant<std::size_t, sizeof...(Rng)>());
			}

			STATIC_FINAL(at_end_index)(index const& idx) const& noexcept -> bool {
				return sizeof...(Rng) == idx.index();
			}

			STATIC_FINAL(increment_index)(index& idx) const& noexcept -> void {
				_ASSERT(!this->at_end_index(idx));

				tc::invoke_with_constant<std::index_sequence_for<Rng...>>(
					[&](auto nconstIndex) noexcept { 
						tc::increment_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(idx));
						correct_index<nconstIndex()>(idx);
					},
					idx.index()
				);
			}


			STATIC_FINAL_MOD(
				template<
					ENABLE_SFINAE BOOST_PP_COMMA()
					std::enable_if_t<
						std::conjunction<tc::has_decrement_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value &&
						std::conjunction<tc::has_end_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value &&
						tc::is_equality_comparable<SFINAE_TYPE(index)>::value
					>* = nullptr
				>,
				decrement_index
			)(index& idx) const& noexcept -> void {
				tc::invoke_with_constant<std::make_index_sequence<sizeof...(Rng)+1>>(
					[&](auto nconstIndexStart) noexcept {
						tc::for_each(
							tc::make_reverse_integer_sequence<int, 0, nconstIndexStart() + 1>(),
							[&](auto nconstIndex) noexcept {
								if constexpr (sizeof...(Rng) == nconstIndex()) {
									 idx = create_end_index(std::integral_constant<int, nconstIndex() - 1>());
									 return tc::continue_;
								} else {
									auto& idxCurrent = tc::get<nconstIndex()>(idx);
									if constexpr (0 == nconstIndex()) {
										_ASSERT( tc::get<0>(this->m_tupleadaptbaserng).base_begin_index() != idxCurrent );
									} else if (tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_begin_index() == idxCurrent) {
										idx = create_end_index(std::integral_constant<int, nconstIndex() - 1>());
										return tc::continue_;
									}
									// Remember early out above
									tc::decrement_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), idxCurrent);
									return tc::break_;
								}
							}
						);
					},
					idx.index()
				);
			}

MODIFY_WARNINGS(((suppress)(4544))) // 'Func2': default template argument ignored on this template declaration
			STATIC_FINAL(dereference_index)(index const& idx) const& noexcept -> decltype(auto) {
				return tc::invoke_with_constant<std::index_sequence_for<Rng...>>(
					[&](auto nconstIndex) noexcept  -> decltype(auto) { // return_decltype leads to ICE 
						return tc::dereference_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(idx));
					},
					idx.index()
				);
			}

			STATIC_FINAL(dereference_index)(index const& idx) & noexcept -> decltype(auto) {
				return tc::invoke_with_constant<std::index_sequence_for<Rng...>>(
					[&](auto nconstIndex) noexcept -> decltype(auto) { // return_decltype leads to ICE 
						return tc::dereference_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(idx));
					},
					idx.index()
				);
			}

			STATIC_FINAL_MOD(
				template<
					ENABLE_SFINAE BOOST_PP_COMMA()
					std::enable_if_t<
						std::conjunction<tc::has_distance_to_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value &&
						std::conjunction<tc::has_end_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value &&
						std::conjunction<tc::has_advance_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value
					>* = nullptr
				>, advance_index
			)(index& idx, difference_type d) const& noexcept -> void {
				tc::invoke_with_constant<std::make_index_sequence<sizeof...(Rng)+1>>(
					[&](auto nconstIndexStart) noexcept {
						if (d < 0) {
							tc::for_each(
								tc::make_reverse_integer_sequence<int, 0, nconstIndexStart() + 1>(),
								[&](auto nconstIndex) noexcept {
									if constexpr (sizeof...(Rng) == nconstIndex()) {
										idx = create_end_index(std::integral_constant<int, nconstIndex() - 1>());
										return tc::continue_;
									} else {
										// As of Visual Studio compiler 19.15.26726, obtaining the range difference_type here as:
										//		using range_difference_t =
										//			typename boost::range_difference<decltype(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range())>::type;
										// triggers a compiler error, because VS does not properly exclude false statements from compilation when using
										// an 'if constexpr' inside a lambda, which means VS attempts to eval the type when nconstIndex==sizeof...(Rng).
										// Breaking the evaluation of the difference_type in two steps seems to work though.
										using range_t=decltype(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range());
										if constexpr(0 == nconstIndex()) {
											tc::advance_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(),
												tc::get<nconstIndex()>(idx),
												tc::explicit_cast<typename boost::range_difference<range_t>::type>(d)
											);
											return tc::break_;
										} else {
											auto const dToBegin = tc::distance_to_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(),
												tc::get<nconstIndex()>(idx),
												tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_begin_index()
											);

											if (!(d < dToBegin)) {
												tc::advance_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(),
													tc::get<nconstIndex()>(idx),
													tc::explicit_cast<typename boost::range_difference<range_t>::type>(d)
												);
												return tc::break_;
											} else {
												d -= dToBegin;
												idx = create_end_index(std::integral_constant<int, nconstIndex() - 1>());
												return tc::continue_;
											}
										}
									}
								}
							);
						} else {
							tc::for_each(
								tc::make_integer_sequence<std::size_t, nconstIndexStart(), sizeof...(Rng)>(),
								[&](auto nconstIndex) noexcept {
									using range_difference_t = typename boost::range_difference<decltype(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range())>::type;
									if constexpr (nconstIndex() == sizeof...(Rng)-1) {
										tc::advance_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(),
											tc::get<nconstIndex()>(idx),
											tc::explicit_cast<range_difference_t>(d)
										);
										correct_index<nconstIndex()>(idx);
										return tc::break_;
									} else {
										auto const dToEnd = tc::distance_to_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(),
											tc::get<nconstIndex()>(idx),
											tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_end_index()
										);
										if (d < dToEnd) {
											tc::advance_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(),
												tc::get<nconstIndex()>(idx),
												tc::explicit_cast<range_difference_t>(d)
											);
											return tc::break_;
										} else {
											d -= dToEnd;
											idx = create_begin_index(std::integral_constant<std::size_t, nconstIndex() + 1>());
											return tc::continue_;
										}
									}
								}
							);
						}
					},
					idx.index()
				);
			}

			STATIC_FINAL_MOD(
				template<
					ENABLE_SFINAE BOOST_PP_COMMA()
					std::enable_if_t<
						std::conjunction<tc::has_distance_to_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value &&
						std::conjunction<tc::has_end_index<std::remove_reference_t<SFINAE_TYPE(Rng)>>...>::value
					>* = nullptr
				>,
				distance_to_index
			)(index const& idxLhs, index const& idxRhs) const& noexcept -> difference_type {
				if (idxLhs.index() == idxRhs.index()) {
					return tc::invoke_with_constant<std::make_index_sequence<sizeof...(Rng)+1>>(
						[&](auto nconstIndex) noexcept -> difference_type {
							if constexpr (nconstIndex() == sizeof...(Rng)) {
								return 0;
							} else {
								return tc::distance_to_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(idxLhs), tc::get<nconstIndex()>(idxRhs));
							}
						},
						idxLhs.index()
					);
				} else {
					auto positive_distance = [&](index const& lhs, index const& rhs) noexcept {
						return tc::accumulate(
							tc::transform(
								tc::iota(lhs.index() + 1, rhs.index()),
								[&](auto nIndex) noexcept {
									return tc::invoke_with_constant<std::index_sequence_for<Rng...>>(
										[&](auto nconstIndex) noexcept { return tc::explicit_cast<difference_type>(this->BaseRangeSize(nconstIndex)); },
										nIndex
									);
								}
							),
							tc::invoke_with_constant<std::index_sequence_for<Rng...>>(
								[&](auto nconstIndex) noexcept -> difference_type {
									return tc::distance_to_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(lhs), tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_end_index());
								},
								lhs.index()
							) + 
							tc::invoke_with_constant<std::make_index_sequence<sizeof...(Rng)+1>>(
								[&](auto nconstIndex) noexcept -> difference_type {
									if constexpr(nconstIndex() == sizeof...(Rng)) {
										return 0;
									} else {
										return tc::distance_to_index(tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_range(), tc::get<nconstIndex()>(this->m_tupleadaptbaserng).base_begin_index(), tc::get<nconstIndex()>(rhs));
									}
								},
								rhs.index()
							),
							tc::fn_assign_plus()
						);
					};

					if (idxLhs.index() < idxRhs.index()) {
						return positive_distance(idxLhs, idxRhs);
					} else {
						return -positive_distance(idxRhs, idxLhs);
					}
				}
			}
		};
	}

	namespace no_adl {
		template<bool HasIterator, typename... Rng>
		struct constexpr_size_base<tc::concat_adaptor_adl::concat_adaptor_impl<HasIterator, Rng...>, tc::void_t<typename tc::constexpr_size<Rng>::type...>>
			: INTEGRAL_CONSTANT((... + tc::constexpr_size<Rng>::value))
		{};

		template<typename ConcatAdaptor, typename... Rng>
		struct range_value<ConcatAdaptor, tc::concat_adaptor_adl::concat_adaptor_impl<false, Rng...>, tc::void_t<tc::common_range_value_t<Rng...>>> final {
			using type = tc::common_range_value_t<Rng...>;
		};

		template<typename... Rng>
		struct is_index_valid_for_move_constructed_range<tc::concat_adaptor_adl::concat_adaptor_impl<true, Rng...>>
			: std::conjunction<std::disjunction<std::is_lvalue_reference<Rng>, tc::is_index_valid_for_move_constructed_range<Rng>>...>
		{};
	}

	namespace no_adl {
		struct fn_concat_impl final {
			[[nodiscard]] constexpr auto operator()() const& noexcept {
				return tc::empty_range();
			}

			template<typename Rng>
			[[nodiscard]] constexpr decltype(auto) operator()(Rng&& rng) const& noexcept {
				return std::forward<Rng>(rng);
			}

			template<typename... Rng, std::enable_if_t<1<sizeof...(Rng)>* = nullptr>
			[[nodiscard]] constexpr auto operator()(Rng&&... rng) const& noexcept {
				return tc::concat_adaptor<std::remove_cv_t<Rng>...>(tc::aggregate_tag, std::forward<Rng>(rng)...);
			}
		};
	}

	namespace concat_detail {
		template<typename Rng>
		[[nodiscard]] constexpr auto forward_range_as_tuple(Rng&& rng) noexcept {
			if constexpr( tc::is_safely_convertible<Rng&&, tc::empty_range>::value ) {
				return tc::tuple<>{};
			} else if constexpr( tc::is_concat_range<tc::remove_cvref_t<Rng>>::value ) {
				return forward_base_ranges_as_tuple(std::forward<Rng>(rng), typename tc::remove_cvref_t<Rng>::index_seq());
			} else {
				return tc::forward_as_tuple(std::forward<Rng>(rng));
			}
		}
	}

	template<typename... Rng>
	[[nodiscard]] constexpr decltype(auto) concat(Rng&&... rng) noexcept {
		return tc::apply(tc::no_adl::fn_concat_impl(), tc::tuple_cat(tc::concat_detail::forward_range_as_tuple(std::forward<Rng>(rng))...));
	}
}
