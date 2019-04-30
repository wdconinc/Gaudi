#ifndef FUNCTIONAL_DETAILS_H
#define FUNCTIONAL_DETAILS_H

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// TODO: fwd declare instead?
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/detected.h"

// Range V3
#include <range/v3/view/const.hpp>
#include <range/v3/view/zip.hpp>

namespace Gaudi::Functional::details {

  // CRJ : Stuff for zipping
  namespace zip {

    /// Print the parameter
    template <typename OS, typename Arg>
    void printSizes( OS& out, Arg&& arg ) {
      out << "SizeOf'" << System::typeinfoName( typeid( Arg ) ) << "'=" << std::forward<Arg>( arg ).size();
    }

    /// Print the parameters
    template <typename OS, typename Arg, typename... Args>
    void printSizes( OS& out, Arg&& arg, Args&&... args ) {
      printSizes( out, arg );
      out << ", ";
      printSizes( out, args... );
    }

    /// Resolve case there is only one container in the range
    template <typename A>
    inline bool check_sizes( const A& ) noexcept {
      return true;
    }

    /// Compare sizes of two containers
    template <typename A, typename B>
    inline bool check_sizes( const A& a, const B& b ) noexcept {
      return a.size() == b.size();
    }

    /// Compare sizes of 3 or more containers
    template <typename A, typename B, typename... C>
    inline bool check_sizes( const A& a, const B& b, const C&... c ) noexcept {
      return ( check_sizes( a, b ) && check_sizes( b, c... ) );
    }

    /// Verify the data container sizes have the same sizes
    template <typename... Args>
    inline decltype( auto ) verifySizes( Args&... args ) {
      if ( UNLIKELY( !check_sizes( args... ) ) ) {
        std::ostringstream mess;
        mess << "Zipped containers have different sizes : ";
        printSizes( mess, args... );
        throw GaudiException( mess.str(), "Gaudi::Functional::details::zip::verifySizes", StatusCode::FAILURE );
      }
    }

    /// Zips multiple containers together to form a single range
    template <typename... Args>
    inline decltype( auto ) range( Args&&... args ) {
#ifndef NDEBUG
      verifySizes( args... );
#endif
      return ranges::view::zip( std::forward<Args>( args )... );
    }

    /// Zips multiple containers together to form a single const range
    template <typename... Args>
    inline decltype( auto ) const_range( Args&&... args ) {
#ifndef NDEBUG
      verifySizes( args... );
#endif
      return ranges::view::const_( ranges::view::zip( std::forward<Args>( args )... ) );
    }
  } // namespace zip

  /////////////////////////////////////////
  namespace details2 {
    // note: boost::optional in boost 1.66 does not have 'has_value()'...
    // that requires boost 1.68 or later... so for now, use operator bool() instead ;-(
    template <typename T>
    using is_optional_ = decltype( bool( std::declval<T>() ), std::declval<T>().value() );
  } // namespace details2
  template <typename Arg>
  using is_optional = typename Gaudi::cpp17::is_detected<details2::is_optional_, Arg>;

  template <typename Arg>
  using require_is_optional = std::enable_if_t<is_optional<Arg>::value>;

  template <typename Arg>
  using require_is_not_optional = std::enable_if_t<!is_optional<Arg>::value>;

  namespace details2 {
    template <typename T, typename = void>
    struct remove_optional {
      using type = T;
    };

    template <typename T>
    struct remove_optional<T, std::enable_if_t<is_optional<T>::value>> {
      using type = typename T::value_type;
    };
  } // namespace details2

  template <typename T>
  using remove_optional_t = typename details2::remove_optional<T>::type;

  constexpr struct invoke_optionally_t {
    template <typename F, typename Arg, typename = require_is_not_optional<Arg>>
    decltype( auto ) operator()( F&& f, Arg&& arg ) const {
      return std::invoke( std::forward<F>( f ), std::forward<Arg>( arg ) );
    }
    template <typename F, typename Arg, typename = require_is_optional<Arg>>
    void operator()( F&& f, Arg&& arg ) const {
      if ( arg ) std::invoke( std::forward<F>( f ), *std::forward<Arg>( arg ) );
    }
  } invoke_optionally{};
  /////////////////////////////////////////

  template <
      typename Out1, typename Out2,
      typename = std::enable_if_t<std::is_constructible<Out1, Out2>::value && std::is_base_of<DataObject, Out1>::value>>
  Out1* put( const DataObjectHandle<Out1>& out_handle, Out2&& out ) {
    return out_handle.put( std::make_unique<Out1>( std::forward<Out2>( out ) ) );
  }

  template <typename Out1, typename Out2, typename = std::enable_if_t<std::is_constructible<Out1, Out2>::value>>
  void put( const DataObjectHandle<AnyDataWrapper<Out1>>& out_handle, Out2&& out ) {
    out_handle.put( std::forward<Out2>( out ) );
  }

  // optional put
  template <typename OutHandle, typename OptOut, typename = require_is_optional<OptOut>>
  void put( const OutHandle& out_handle, OptOut&& out ) {
    if ( out ) put( out_handle, *std::forward<OptOut>( out ) );
  }
  /////////////////////////////////////////
  // adapt to differences between eg. std::vector (which has push_back) and KeyedContainer (which has insert)
  // adapt to getting a T, and a container wanting T* by doing new T{ std::move(out) }
  // adapt to getting a optional<T>

  constexpr struct insert_t {
    // for Container<T*>, return T
    template <typename Container>
    using c_remove_ptr_t = std::remove_pointer_t<typename Container::value_type>;

    template <typename Container, typename Value>
    auto operator()( Container& c, Value&& v ) const -> decltype( c.push_back( v ) ) {
      return c.push_back( std::forward<Value>( v ) );
    }

    template <typename Container, typename Value>
    auto operator()( Container& c, Value&& v ) const -> decltype( c.insert( v ) ) {
      return c.insert( std::forward<Value>( v ) );
    }

    // Container<T*> with T&& as argument
    template <typename Container, typename = std::enable_if_t<std::is_pointer<typename Container::value_type>::value>>
    auto operator()( Container& c, c_remove_ptr_t<Container>&& v ) const {
      return operator()( c, new c_remove_ptr_t<Container>{std::move( v )} );
    }

  } insert{};

  /////////////////////////////////////////

  constexpr struct deref_t {
    template <typename In, typename = std::enable_if_t<!std::is_pointer<In>::value>>
    const In& operator()( const In& in ) const {
      return in;
    }

    template <typename In>
    const In& operator()( const In* in ) const {
      assert( in != nullptr );
      return *in;
    }
  } deref{};

  /////////////////////////////////////////
  // if Container is a pointer, then we're optional items
  namespace details2 {
    template <typename Container, typename Value>
    void push_back( Container& c, const Value& v, std::true_type ) {
      c.push_back( v );
    }
    template <typename Container, typename Value>
    void push_back( Container& c, const Value& v, std::false_type ) {
      c.push_back( &v );
    }

    template <typename In>
    struct get_from_handle {
      template <template <typename> class Handle, typename I,
                typename = std::enable_if_t<std::is_convertible<I, In>::value>>
      auto operator()( const Handle<I>& h ) -> const In& {
        return *h.get();
      }
      template <template <typename> class Handle, typename I,
                typename = std::enable_if_t<std::is_convertible<I*, In>::value>>
      auto operator()( const Handle<I>& h ) -> const In {
        return h.getIfExists();
      } // In is-a pointer
    };

    template <typename T>
    T* deref_if( T* const t, std::false_type ) {
      return t;
    }
    template <typename T>
    T& deref_if( T* const t, std::true_type ) {
      return *t;
    }
  } // namespace details2

  template <typename Container>
  class vector_of_const_ {
    static constexpr bool is_pointer = std::is_pointer<Container>::value;
    using val_t                      = std::add_const_t<std::remove_pointer_t<Container>>;
    using ptr_t                      = std::add_pointer_t<val_t>;
    using ref_t                      = std::add_lvalue_reference_t<val_t>;
    using ContainerVector            = std::vector<ptr_t>;
    ContainerVector m_containers;

  public:
    using value_type = std::conditional_t<is_pointer, ptr_t, val_t>;
    using size_type  = typename ContainerVector::size_type;
    class iterator {
      using it_t = typename ContainerVector::const_iterator;
      it_t m_i;
      friend class vector_of_const_;
      iterator( it_t iter ) : m_i( iter ) {}
      using ret_t = std::conditional_t<is_pointer, ptr_t, ref_t>;

    public:
      using iterator_category = typename it_t::iterator_category;
      using value_type        = typename it_t::iterator_category;
      using reference         = typename it_t::reference;
      using pointer           = typename it_t::pointer;
      using difference_type   = typename it_t::difference_type;

      friend bool operator!=( const iterator& lhs, const iterator& rhs ) { return lhs.m_i != rhs.m_i; }
      friend bool operator==( const iterator& lhs, const iterator& rhs ) { return lhs.m_i == rhs.m_i; }
      friend auto operator-( const iterator& lhs, const iterator& rhs ) { return lhs.m_i - rhs.m_i; }
      ret_t       operator*() const { return details2::deref_if( *m_i, std::integral_constant<bool, !is_pointer>{} ); }
      iterator&   operator++() {
        ++m_i;
        return *this;
      }
      iterator& operator--() {
        --m_i;
        return *this;
      }
      bool     is_null() const { return !*m_i; }
      explicit operator bool() const { return !is_null(); }
    };
    vector_of_const_() = default;
    void reserve( size_type size ) { m_containers.reserve( size ); }
    template <typename T> // , typename = std::is_convertible<T,std::conditional_t<is_pointer,ptr_t,val_t>>
    void push_back( T&& container ) {
      details2::push_back( m_containers, std::forward<T>( container ), std::integral_constant<bool, is_pointer>{} );
    } // note: does not copy its argument, so we're not really a container...
    iterator  begin() const { return m_containers.begin(); }
    iterator  end() const { return m_containers.end(); }
    size_type size() const { return m_containers.size(); }

    template <typename X = Container>
    std::enable_if_t<!std::is_pointer<X>::value, ref_t> operator[]( size_type i ) const {
      return *m_containers[i];
    }

    template <typename X = Container>
    std::enable_if_t<std::is_pointer<X>::value, ptr_t> operator[]( size_type i ) const {
      return m_containers[i];
    }

    template <typename X = Container>
    std::enable_if_t<!std::is_pointer<X>::value, ref_t> at( size_type i ) const {
      return *m_containers[i];
    }

    template <typename X = Container>
    std::enable_if_t<std::is_pointer<X>::value, ptr_t> at( size_type i ) const {
      return m_containers[i];
    }

    bool is_null( size_type i ) const { return !m_containers[i]; }
  };

  /////////////////////////////////////////
  namespace detail2 { // utilities for detected_or_t{,_} usage
    template <typename Tr>
    using BaseClass_t = typename Tr::BaseClass;
    template <typename Tr, typename T>
    using OutputHandle_t = typename Tr::template OutputHandle<T>;
    template <typename Tr, typename T>
    using InputHandle_t = typename Tr::template InputHandle<T>;
  } // namespace detail2

  // check whether Traits::BaseClass is a valid type,
  // if so, define BaseClass_t<Traits> as being Traits::BaseClass
  // else   define                     as being GaudiAlgorithm
  template <typename Tr>
  using BaseClass_t = Gaudi::cpp17::detected_or_t<GaudiAlgorithm, detail2::BaseClass_t, Tr>;

  // check whether Traits::{Input,Output}Handle<T> is a valid type,
  // if so, define {Input,Output}Handle_t<Traits,T> as being Traits::{Input,Output}Handle<T>
  // else   define                                  as being DataObject{Read,,Write}Handle<T>
  template <typename Tr, typename T>
  using OutputHandle_t = Gaudi::cpp17::detected_or_t<DataObjectWriteHandle<T>, detail2::OutputHandle_t, Tr, T>;
  template <typename Tr, typename T>
  using InputHandle_t = Gaudi::cpp17::detected_or_t<DataObjectReadHandle<T>, detail2::InputHandle_t, Tr, T>;

  template <typename Traits>
  inline constexpr bool isLegacy =
      std::is_base_of_v<Gaudi::details::LegacyAlgorithmAdapter, details::BaseClass_t<Traits>>;

  /////////

  template <typename Handles>
  Handles make_vector_of_handles( IDataHandleHolder* owner, const std::vector<std::string>& init ) {
    Handles handles;
    handles.reserve( init.size() );
    std::transform( init.begin(), init.end(),
                    std::back_inserter( handles ), [&]( const std::string& loc ) -> typename Handles::value_type {
                      return {loc, owner};
                    } );
    return handles;
  }

  ///////////////////////
  // given a pack, return a corresponding tuple
  template <typename... In>
  struct filter_evtcontext_t {
    using type = std::tuple<In...>;

    static_assert( !std::disjunction<std::is_same<EventContext, In>...>::value,
                   "EventContext can only appear as first argument" );

    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, Handles& handles ) {
      return std::apply( [&]( const auto&... handle ) { return algo( details::deref( handle.get() )... ); }, handles );
    }
    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, const EventContext&, Handles& handles ) {
      return std::apply( [&]( const auto&... handle ) { return algo( details::deref( handle.get() )... ); }, handles );
    }
  };

  // except when it starts with EventContext, then drop it
  template <typename... In>
  struct filter_evtcontext_t<EventContext, In...> {
    using type = std::tuple<In...>;

    static_assert( !std::disjunction<std::is_same<EventContext, In>...>::value,
                   "EventContext can only appear as first argument" );

    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, Handles& handles ) {
      return std::apply(
          [&]( const auto&... handle ) {
            return algo( Gaudi::Hive::currentContext(), details::deref( handle.get() )... );
          },
          handles );
    }

    template <typename Algorithm, typename Handles>
    static auto apply( const Algorithm& algo, const EventContext& ctx, Handles& handles ) {
      return std::apply( [&]( const auto&... handle ) { return algo( ctx, details::deref( handle.get() )... ); },
                         handles );
    }
  };

  template <typename... In>
  using filter_evtcontext = typename filter_evtcontext_t<In...>::type;

  template <typename OutputSpec, typename InputSpec, typename Traits_>
  class DataHandleMixin;

  template <typename Out, typename In, typename Tr>
  void updateHandleLocation( DataHandleMixin<Out, In, Tr>& parent, const std::string& prop,
                             const std::string& newLoc ) {
    auto sc = parent.setProperty( prop, newLoc );
    if ( sc.isFailure() ) throw GaudiException( "Could not set Property", prop + " -> " + newLoc, sc );
  }

  template <typename Out, typename In, typename Tr>
  void updateHandleLocations( DataHandleMixin<Out, In, Tr>& parent, const std::string& prop,
                              const std::vector<std::string>& newLocs ) {
    std::ostringstream ss;
    GaudiUtils::details::ostream_joiner( ss << '[', newLocs, ", ", []( std::ostream & os, const auto& i ) -> auto& {
      return os << "'" << i << "'";
    } ) << ']';
    auto sc = parent.setProperty( prop, ss.str() );
    if ( sc.isFailure() ) throw GaudiException( "Could not set Property", prop + " -> " + ss.str(), sc );
  }

  template <typename... Out, typename... In, typename Traits_>
  class DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value, "BaseClass must inherit from Algorithm" );

    template <typename IArgs, typename OArgs, std::size_t... I, std::size_t... J>
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const IArgs& inputs, std::index_sequence<I...>,
                     const OArgs& outputs, std::index_sequence<J...> )
        : BaseClass_t<Traits_>( name, pSvcLocator )
        , m_inputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( inputs ) )... )
        , m_outputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<J>( outputs ) )... ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 );
    }

  public:
    constexpr static std::size_t N_in  = sizeof...( In );
    constexpr static std::size_t N_out = sizeof...( Out );

    using KeyValue  = std::pair<std::string, std::string>;
    using KeyValues = std::pair<std::string, std::vector<std::string>>;

    // generic constructor:  N -> M
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N_in>& inputs,
                     const std::array<KeyValue, N_out>& outputs )
        : DataHandleMixin( name, pSvcLocator, inputs, std::index_sequence_for<In...>{}, outputs,
                           std::index_sequence_for<Out...>{} ) {}

    // special cases: forward to the generic case...
    // 1 -> 1
    DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& input, const KeyValue& output )
        : DataHandleMixin( name, locator, std::array<KeyValue, 1>{input}, std::array<KeyValue, 1>{output} ) {}
    // 1 -> N
    DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& input,
                     const std::array<KeyValue, N_out>& outputs )
        : DataHandleMixin( name, locator, std::array<KeyValue, 1>{input}, outputs ) {}
    // N -> 1
    DataHandleMixin( const std::string& name, ISvcLocator* locator, const std::array<KeyValue, N_in>& inputs,
                     const KeyValue& output )
        : DataHandleMixin( name, locator, inputs, std::array<KeyValue, 1>{output} ) {}

    template <std::size_t N = 0>
    const std::string& inputLocation() const {
      return std::get<N>( m_inputs ).objKey();
    }
    constexpr unsigned int inputLocationSize() const { return N_in; }

    template <std::size_t N = 0>
    const std::string& outputLocation() const {
      return std::get<N>( m_outputs ).objKey();
    }
    constexpr unsigned int outputLocationSize() const { return N_out; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::InputHandle_t<Traits_, In>...>   m_inputs;
    std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
  };

  template <typename Traits_>
  class DataHandleMixin<void, std::tuple<>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value, "BaseClass must inherit from Algorithm" );

  public:
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator ) : BaseClass_t<Traits_>( name, pSvcLocator ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 );
    }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<> m_inputs;
  };

  template <typename... In, typename Traits_>
  class DataHandleMixin<void, std::tuple<In...>, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value, "BaseClass must inherit from Algorithm" );

    template <typename IArgs, std::size_t... I>
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const IArgs& inputs, std::index_sequence<I...> )
        : BaseClass_t<Traits_>( name, pSvcLocator )
        , m_inputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<I>( inputs ) )... ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 );
    }

  public:
    using KeyValue                    = std::pair<std::string, std::string>;
    using KeyValues                   = std::pair<std::string, std::vector<std::string>>;
    constexpr static std::size_t N_in = sizeof...( In );

    // generic constructor:  N -> 0
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N_in>& inputs )
        : DataHandleMixin( name, pSvcLocator, inputs, std::index_sequence_for<In...>{} ) {}

    // special cases: forward to the generic case...
    // 1 -> 0
    DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& input )
        : DataHandleMixin( name, locator, std::array<KeyValue, 1>{input} ) {}

    template <std::size_t N = 0>
    const std::string& inputLocation() const {
      return std::get<N>( m_inputs ).objKey();
    }
    constexpr unsigned int inputLocationSize() const { return N_in; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::InputHandle_t<Traits_, In>...> m_inputs;
  };

  template <typename... Out, typename Traits_>
  class DataHandleMixin<std::tuple<Out...>, void, Traits_> : public BaseClass_t<Traits_> {
    static_assert( std::is_base_of<Algorithm, BaseClass_t<Traits_>>::value, "BaseClass must inherit from Algorithm" );

    template <typename OArgs, std::size_t... J>
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const OArgs& outputs,
                     std::index_sequence<J...> )
        : BaseClass_t<Traits_>( name, pSvcLocator )
        , m_outputs( std::tuple_cat( std::forward_as_tuple( this ), std::get<J>( outputs ) )... ) {
      // make sure this algorithm is seen as reentrant by Gaudi
      this->setProperty( "Cardinality", 0 );
    }

  public:
    constexpr static std::size_t N_out = sizeof...( Out );
    using KeyValue                     = std::pair<std::string, std::string>;
    using KeyValues                    = std::pair<std::string, std::vector<std::string>>;

    // generic constructor:  0 -> N
    DataHandleMixin( const std::string& name, ISvcLocator* pSvcLocator, const std::array<KeyValue, N_out>& outputs )
        : DataHandleMixin( name, pSvcLocator, outputs, std::index_sequence_for<Out...>{} ) {}

    // 0 -> 1
    DataHandleMixin( const std::string& name, ISvcLocator* locator, const KeyValue& output )
        : DataHandleMixin( name, locator, std::array<KeyValue, 1>{output} ) {}

    template <std::size_t N = 0>
    const std::string& outputLocation() const {
      return std::get<N>( m_outputs ).objKey();
    }
    constexpr unsigned int outputLocationSize() const { return N_out; }

  protected:
    bool isReEntrant() const override { return true; }

    std::tuple<details::OutputHandle_t<Traits_, Out>...> m_outputs;
  };

  /////////////////
  template <typename Fun, typename Container, typename... Args>
  constexpr void applyPostProcessing( const Fun&, Container&, Args... ) {
    static_assert( sizeof...( Args ) == 0, "Args should not be used!" );
  }

  template <typename Fun, typename Container>
  auto applyPostProcessing( const Fun& fun, Container& c ) -> decltype( fun.postprocess( c ), void() ) {
    fun.postprocess( c );
  }

} // namespace Gaudi::Functional::details

#endif
