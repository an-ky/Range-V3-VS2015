/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_RANGE_CONCEPTS_HPP
#define RANGES_V3_RANGE_CONCEPTS_HPP

#include <utility>
#include <type_traits>
#include <initializer_list>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/size.hpp>
#include <range/v3/utility/iterator_concepts.hpp>
#include <range/v3/utility/iterator_traits.hpp>

#ifndef RANGES_NO_STD_FORWARD_DECLARATIONS
// Non-portable forward declarations of standard containers
RANGES_BEGIN_NAMESPACE_STD
    template<class Key, class Compare /*= less<Key>*/, class Alloc /*= allocator<Key>*/>
    class set;

    template<class Key, class Compare /*= less<Key>*/, class Alloc /*= allocator<Key>*/>
    class multiset;

    template<class Key, class Hash /*= hash<Key>*/, class Pred /*= equal_to<Key>*/, class Alloc /*= allocator<Key>*/>
    class unordered_set;

    template<class Key, class Hash /*= hash<Key>*/, class Pred /*= equal_to<Key>*/, class Alloc /*= allocator<Key>*/>
    class unordered_multiset;
RANGES_END_NAMESPACE_STD
#else
#include <set>
#include <unordered_set>
#endif

namespace ranges
{
    inline namespace v3
    {
        /// \addtogroup group-concepts
        /// @{
        namespace concepts
        {
            ///
            /// Range concepts below
            ///

            struct Range
            {
                // Associated types
#ifdef WORKAROUND_SFINAE_ALIAS_DECLTYPE
                template<typename>
                using iterator_t_helper_void_t = void;
                template<typename, typename = void> struct iterator_t_helper {};
                template<typename T> struct iterator_t_helper<T, iterator_t_helper_void_t<decltype(begin(val<T>()))>> {
                    using type = decltype(begin(val<T>()));
                };
                template<typename T>
                using iterator_t = meta::_t<iterator_t_helper<T>>;

                template<typename>
                using sentinel_t_helper_void_t = void;
                template<typename, typename = void> struct sentinel_t_helper {};
                template<typename T> struct sentinel_t_helper<T, sentinel_t_helper_void_t<decltype(end(val<T>()))>> {
                    using type = decltype(end(val<T>()));
                };
                template<typename T>
                using sentinel_t = meta::_t<sentinel_t_helper<T>>;
#else
                template<typename T>
                using iterator_t = decltype(begin(val<T>()));

                template<typename T>
                using sentinel_t = decltype(end(val<T>()));
#endif

                template<typename T>
                using difference_t = concepts::WeaklyIncrementable::difference_t<iterator_t<T>>;

                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<IteratorRange>(begin(t), end(t))
                    ));
            };

            struct OutputRange
              : refines<Range(_1)>
            {
                template<typename T, typename V>
                auto requires_(T&&, V&&) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<OutputIterator, Range::iterator_t<T>, V>()
                    ));
            };

            struct InputRange
              : refines<Range>
            {
                // Associated types
                template<typename T>
                using category_t = concepts::WeakInputIterator::category_t<iterator_t<T>>;

                template<typename T>
                using value_t = concepts::Readable::value_t<iterator_t<T>>;

                template<typename T>
                using reference_t = concepts::Readable::reference_t<iterator_t<T>>;

                template<typename T>
                using rvalue_reference_t = concepts::Readable::rvalue_reference_t<iterator_t<T>>;

                template<typename T>
                using common_reference_t = concepts::Readable::common_reference_t<iterator_t<T>>;

                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<InputIterator>(begin(t))
                    ));
            };

            struct ForwardRange
              : refines<InputRange>
            {
                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<ForwardIterator>(begin(t))
                    ));
            };

            struct BidirectionalRange
              : refines<ForwardRange>
            {
                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<BidirectionalIterator>(begin(t))
                    ));
            };

            struct RandomAccessRange
              : refines<BidirectionalRange>
            {
                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<RandomAccessIterator>(begin(t))
                    ));
            };

            struct BoundedRange
              : refines<Range>
            {
                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::same_type(begin(t), end(t))
                    ));
            };

            struct SizedRange
              : refines<Range>
            {
#ifdef WORKAROUND_SFINAE_ALIAS_DECLTYPE
                template<typename>
                using size_t_helper_void_t = void;
                template<typename, typename = void> struct size_t_helper {};
                template<typename T> struct size_t_helper<T, size_t_helper_void_t<decltype(size(val<T>()))>> {
                    using type = decltype(size(val<T>()));
                };
                template<typename T>
                using size_t = meta::_t<size_t_helper<T>>;
#else
                template<typename T>
                using size_t = decltype(size(val<T>()));
#endif

                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Integral>(size(t)),
                        concepts::is_true(is_sized_range<T>())
                    ));
            };

            /// INTERNAL ONLY
            /// A type is SizedRangeLike_ if it is Range and ranges::size
            /// can be called on it and it returns an Integral
            struct SizedRangeLike_
              : refines<Range>
            {
                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Integral>(size(t))
                    ));
            };

            /// INTERNAL ONLY
            /// A type is ContainerLike_ if it is Range and the const-ness of its
            /// reference type is sensitive to the const-ness of the Container
            struct ContainerLike_
              : refines<InputRange>
            {
                template<typename T>
                auto requires_(T&&) -> decltype(
                    concepts::valid_expr(
                        concepts::is_false(
                            std::is_same<reference_t<detail::as_ref_t<T>>,
                                         reference_t<detail::as_cref_t<T>>>())
                    ));
            };

            ///
            /// View concepts below
            ///

            struct View
              : refines<Range>
            {
                template<typename T>
                auto requires_(T&&) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<SemiRegular, uncvref_t<T>>(),
                        concepts::is_true(is_view<T>())
                    ));
            };

            struct OutputView
              : refines<View(_1), OutputRange>
            {};

            struct InputView
              : refines<View, InputRange>
            {};

            struct ForwardView
              : refines<InputView, ForwardRange>
            {};

            struct BidirectionalView
              : refines<ForwardView, BidirectionalRange>
            {};

            struct RandomAccessView
              : refines<BidirectionalView, RandomAccessRange>
            {};

            // Additional concepts for checking additional orthogonal properties
            struct BoundedView
              : refines<View, BoundedRange>
            {};

            struct SizedView
              : refines<View, SizedRange>
            {};
        }

        template<typename T>
        using Range = concepts::models<concepts::Range, T>;

        template<typename T, typename V>
        using OutputRange = concepts::models<concepts::OutputRange, T, V>;

        template<typename T>
        using InputRange = concepts::models<concepts::InputRange, T>;

        template<typename T>
        using ForwardRange = concepts::models<concepts::ForwardRange, T>;

        template<typename T>
        using BidirectionalRange = concepts::models<concepts::BidirectionalRange, T>;

        template<typename T>
        using RandomAccessRange = concepts::models<concepts::RandomAccessRange, T>;

        template<typename T>
        using BoundedRange = concepts::models<concepts::BoundedRange, T>;

        template<typename T>
        using SizedRange = concepts::models<concepts::SizedRange, T>;

        /// INTERNAL ONLY
        template<typename T>
        using SizedRangeLike_ = concepts::models<concepts::SizedRangeLike_, T>;

        /// INTERNAL ONLY
        template<typename T>
        using ContainerLike_ = concepts::models<concepts::ContainerLike_, T>;

        template<typename T>
        using View = concepts::models<concepts::View, T>;

        template<typename T, typename V>
        using OutputView = concepts::models<concepts::OutputView, T, V>;

        template<typename T>
        using InputView = concepts::models<concepts::InputView, T>;

        template<typename T>
        using ForwardView = concepts::models<concepts::ForwardView, T>;

        template<typename T>
        using BidirectionalView = concepts::models<concepts::BidirectionalView, T>;

        template<typename T>
        using RandomAccessView = concepts::models<concepts::RandomAccessView, T>;

        // Extra concepts:
        template<typename T>
        using BoundedView = concepts::models<concepts::BoundedView, T>;

        template<typename T>
        using SizedView = concepts::models<concepts::SizedView, T>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // range_concept
        template<typename T>
        using range_concept =
            concepts::most_refined<
                meta::list<
                    concepts::RandomAccessRange,
                    concepts::BidirectionalRange,
                    concepts::ForwardRange,
                    concepts::InputRange>, T>;

        template<typename T>
        using range_concept_t =
            meta::_t<range_concept<T>>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // bounded_range_concept
        template<typename T>
        using bounded_range_concept =
            concepts::most_refined<
                meta::list<
                    concepts::BoundedRange,
                    concepts::Range>, T>;

        template<typename T>
        using bounded_range_concept_t =
            meta::_t<bounded_range_concept<T>>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // sized_range_concept
        template<typename T>
        using sized_range_concept =
            concepts::most_refined<
                meta::list<
                    concepts::SizedRange,
                    concepts::Range>, T>;

        template<typename T>
        using sized_range_concept_t =
            meta::_t<sized_range_concept<T>>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // bounded_view_concept
        template<typename T>
        using bounded_view_concept =
            concepts::most_refined<
                meta::list<
                    concepts::BoundedView,
                    concepts::View>, T>;

        template<typename T>
        using bounded_view_concept_t = meta::_t<bounded_view_concept<T>>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // sized_view_concept
        template<typename T>
        using sized_view_concept =
            concepts::most_refined<
                meta::list<
                    concepts::SizedView,
                    concepts::View>, T>;

        template<typename T>
        using sized_view_concept_t = meta::_t<sized_view_concept<T>>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // view_concept
        template<typename T>
        using view_concept =
            concepts::most_refined<
                meta::list<
                    concepts::View,
                    concepts::Range>, T>;

        template<typename T>
        using view_concept_t = meta::_t<view_concept<T>>;

        /// @}

        /// \cond
        namespace detail
        {
            // Something is a view if it's a Range and either:
            //  - It doesn't look like a container, or
            //  - It's derived from view_base
            template<typename T>
            struct is_view_impl_
              : std::integral_constant<
                    bool,
#ifdef WORKAROUND_SFINAE_CONSTEXPR
                    Range<T>::value && (!ContainerLike_<T>::value || DerivedFrom<T, view_base>::value)
#else
                    Range<T>() && (!ContainerLike_<T>() || DerivedFrom<T, view_base>())
#endif
                >
            {};

            template<typename T, std::size_t N>
            struct is_view_impl_<T[N]>
              : std::false_type
            {};

            // Something is a sized range if it looks like a sized range; i.e.,
            // if size(rng) compiles and returns an Integral
            template<typename T>
            struct is_sized_range_impl_
              : std::integral_constant<
                    bool,
#ifdef WORKAROUND_SFINAE_CONSTEXPR
                    (SizedRangeLike_<T>::value)
#else
                    (SizedRangeLike_<T>())
#endif
                >
            {};

            template<typename T, std::size_t N>
            struct is_sized_range_impl_<T[N]>
              : std::true_type
            {};
        }
        /// \endcond

        /// \addtogroup group-concepts
        /// @{

        // Specialize this if the default is wrong.
        template<typename T, typename Enable>
        struct is_sized_range
          : meta::if_<
                std::is_same<T, uncvref_t<T>>,
                detail::is_sized_range_impl_<T>,
                is_sized_range<uncvref_t<T>>>
        {};

        // Specialize this if the default is wrong.
        template<typename T, typename Enable>
        struct is_view
          : meta::if_<
                std::is_same<T, uncvref_t<T>>,
                detail::is_view_impl_<T>,
                is_view<uncvref_t<T>>>
        {};

        // By default, the is_view default heuristic guesses wrong for these container types:
        template<typename T>
        struct is_view<std::initializer_list<T>>
          : std::false_type
        {};

        template<class Key, class Compare, class Alloc>
        struct is_view<std::set<Key, Compare, Alloc>>
          : std::false_type
        {};

        template<class Key, class Compare, class Alloc>
        struct is_view<std::multiset<Key, Compare, Alloc>>
          : std::false_type
        {};

        template<class Key, class Hash, class Pred, class Alloc>
        struct is_view<std::unordered_set<Key, Hash, Pred, Alloc>>
          : std::false_type
        {};

        template<class Key, class Hash, class Pred, class Alloc>
        struct is_view<std::unordered_multiset<Key, Hash, Pred, Alloc>>
          : std::false_type
        {};

        /// @}
    }
}

#endif
