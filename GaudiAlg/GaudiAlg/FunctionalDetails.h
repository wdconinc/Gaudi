#ifndef __Functional_details_h__
#define __Functional_details_h__

#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

namespace Gaudi { namespace Functional { namespace details {

    // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

    //TODO: constrain to the case where Out2 is convertible to Out1?
    template <typename Out1, typename Out2>
    void put( DataObjectHandle<Out1>& out_handle, Out2&& out ) {
        out_handle.put( new Out1( std::forward<Out2>(out) ) );
    }

    //TODO: constrain to the case where Out2 is convertible to Out1?
    template <typename Out1, typename Out2>
    void put( AnyDataHandle<Out1>& out_handle, Out2&& out ) {
        out_handle.put( std::forward<Out2>(out) );
    }

    // detect whether a traits class defines the requested type,
    //   if so, use it,
    //   otherwise use the default
    //
    // based on http://en.cppreference.com/w/cpp/experimental/is_detected
    // and the libstdc++ source, specificially libstdc++-v3/include/std/type_traits

    namespace detail2 {
#ifdef HAVE_CPP17
        template<typename...> using void_t = void;
#else
        template <typename...> struct void_t_ { using type = void; };
        template <typename... T> using void_t = typename void_t_<T...>::type;
#endif

        /// Implementation of the detection idiom (negative case).
        template<typename Default, typename AlwaysVoid,
                 template<typename...> class Op, typename... Args>
        struct detector {
            using type = Default;
        };

        /// Implementation of the detection idiom (positive case).
        template<typename Default,
                 template<typename...> class Op, typename... Args>
        struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
            using type = Op<Args...>;
        };
    }

    // Op<Args...> if that is a valid type, otherwise Default.
    template<typename Default,
             template<typename...> class Op, typename... Args>
    using detected_or_t = typename detail2::detector<Default, void, Op, Args...>::type;

     // Op<Args...> if that is a valid type, otherwise Default<Args...>.
    template<template<typename...> class Default,
             template<typename...> class Op, typename... Args>
    using detected_or_t_ = detected_or_t<Default<Args...>, Op, Args...>;

    ///////////////
    namespace detail2 { // utilities for detected_or_t{,_} usage
        template <typename Tr> using BaseClass_  = typename Tr::BaseClass;
        template <typename Tr, typename T> using DataObjectHandle_ = DataObjectHandle<T>;
        template <typename Tr, typename T> using OutputHandle_  = typename Tr::template OutputHandle<T>;
        template <typename Tr, typename T> using InputHandle_   = typename Tr::template InputHandle<T>;
    }

    // check whether Traits::BaseClass is a valid type,
    // if so, define BaseClass_t<Traits> as being Traits::BaseClass
    // else   define                     as being GaudiAlgorithm
    template <typename Tr> using BaseClass_t = detected_or_t< GaudiAlgorithm, detail2::BaseClass_, Tr >;

    // check whether Traits::{Input,Output}Handle<T> is a valid type,
    // if so, define {Input,Output}Handle_t<Traits,T> as being Traits::{Input,Output}Handle<T>
    // else   define                                  as being DataHandle<T>
    template <typename Tr, typename T> using OutputHandle_t = detected_or_t_< detail2::DataObjectHandle_, detail2::OutputHandle_, Tr, T>;
    template <typename Tr, typename T> using InputHandle_t  = detected_or_t_< detail2::DataObjectHandle_, detail2::InputHandle_,  Tr, T>;

    /////////

    namespace details2 {
        template <std::size_t N, typename Tuple >
        using element_t = typename std::tuple_element<N, Tuple>::type;

        template <typename Tuple, typename KeyValues, std::size_t... I>
        Tuple make_tuple_of_handles_helper( IDataHandleHolder* o, const KeyValues& initvalue, Gaudi::DataHandle::Mode m, std::index_sequence<I...> ) {
            return std::make_tuple( element_t<I,Tuple>{std::get<I>(initvalue).second, m, o} ... );
        }
        template <typename KeyValues, typename Properties,  std::size_t... I>
        void declare_tuple_of_properties_helper(Algorithm* owner, const KeyValues& inputs, Properties& props,  std::index_sequence<I...>) {
            std::initializer_list<int>{
                (owner->declareProperty( std::get<I>(inputs).first,
                                         std::get<I>(props)         ),0)...
            };
        }
    }

    template <typename Tuple, typename KeyValues >
    Tuple make_tuple_of_handles( IDataHandleHolder* owner, const KeyValues& initvalue, Gaudi::DataHandle::Mode mode ) {
        return details2::make_tuple_of_handles_helper<Tuple>( owner, initvalue, mode, std::make_index_sequence<std::tuple_size<Tuple>::value>{} );
    }

    template <typename KeyValues, typename Properties>
    void declare_tuple_of_properties(Algorithm* owner, const KeyValues& inputs, Properties& props) {
        static_assert( std::tuple_size<KeyValues>::value == std::tuple_size<Properties>::value, "Inconsistent lengths" );
        constexpr auto N = std::tuple_size<KeyValues>::value;
        details2::declare_tuple_of_properties_helper( owner, inputs, props, std::make_index_sequence<N>{} );
    }

    template <typename Handles>
    Handles make_vector_of_handles(IDataHandleHolder* owner, const std::vector<std::string>& init, Gaudi::DataHandle::Mode mode) {
         Handles handles; handles.reserve(init.size());
         std::transform( init.begin(), init.end(), std::back_inserter(handles),
                         [&](const std::string& loc) -> typename Handles::value_type
                         { return {loc,mode, owner}; });
         return handles;
    }

} } }

#endif
