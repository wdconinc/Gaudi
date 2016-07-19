#ifndef __Functional_details_h__
#define __Functional_details_h__

#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "boost/optional.hpp"

namespace Gaudi { namespace Functional { namespace details {

    // implementation of C++17 std::as_const, see http://en.cppreference.com/w/cpp/utility/as_const
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

    /////////////////////////////////////////

    template <typename Out1, typename Out2,
              typename = typename std::enable_if<std::is_constructible<Out1,Out2>::value>::type>
    Out1* put( DataObjectHandle<Out1>& out_handle, Out2&& out ) {
        return out_handle.put( new Out1( std::forward<Out2>(out) ) );
    }

    template <typename Out1, typename Out2,
              typename = typename std::enable_if<std::is_constructible<Out1,Out2>::value>::type>
    void put( AnyDataHandle<Out1>& out_handle, Out2&& out ) {
        out_handle.put( std::forward<Out2>(out) );
    }

    // optional put
    template <typename OutHandle, typename Out>
    void put( OutHandle& out_handle, boost::optional<Out>&& out) {
       if (out) put(out_handle,std::move(*out));
    }

    namespace details2 {
        template< typename T > struct remove_optional      {typedef T type;};
        template< typename T > struct remove_optional< boost::optional<T> >  {typedef T type;};
        // template< typename T > struct remove_optional< std::optional<T> >  {typedef T type;};

    }
    template <typename T> using remove_optional_t = typename details2::remove_optional<T>::type;
    template< typename T> struct is_optional : std::false_type {};
    template< typename T> struct is_optional< boost::optional<T> > : std::true_type {};
    // C++17: template <typename T> constexpr bool is_optional_v = is_optional<T>::value;

    /////////////////////////////////////////
    template <typename Container>
    class vector_of_ {
        using ContainerVector = std::vector<Container*>;
        ContainerVector m_containers;
    public:
        using value_type = Container;
        using size_type  = typename ContainerVector::size_type;
        class iterator {
             typename ContainerVector::const_iterator m_i;
             friend class vector_of_;
             iterator(typename ContainerVector::const_iterator iter) : m_i(iter) {}
         public:
             friend bool operator!=(const iterator& lhs, const iterator& rhs) { return lhs.m_i != rhs.m_i; }
             const Container& operator*() const { return **m_i; }
             iterator& operator++() { ++m_i; return *this; }
             iterator& operator--() { --m_i; return *this; }
        };
        vector_of_() = default;
        vector_of_(size_type size) : m_containers(size) {}
        void reserve(size_type size) { m_containers.reserve(size); }
        void push_back(Container& c) { m_containers.push_back(&c); } // note: does not copy its argument, so we're not really a container...
        iterator begin() const { return m_containers.begin(); }
        iterator end() const { return m_containers.end(); }
        size_type size() const { return m_containers.size(); }
        Container& operator[](size_type i) { return *m_containers[i]; }
    };

    template <typename Container>
    using vector_of_const_ = vector_of_<const Container>;
    // using vector_of_const_ = vector_of_<typename std::add_const<Container>::type>;

    /////////////////////////////////////////

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
