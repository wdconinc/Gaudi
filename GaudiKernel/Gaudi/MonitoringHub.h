/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <deque>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <typeinfo>

namespace Gaudi::Monitoring {
  /// Central entity in a Gaudi application that manages monitoring objects (i.e. counters, histograms, etc.).
  ///
  /// The Gaudi::Monitoring::Hub delegates the actual reports to services implementing the Gaudi::Monitoring::Hub::Sink
  /// interface.
  struct Hub {
    using json = nlohmann::json;

    /** Wrapper class for arbitrary monitoring objects.
     *
     * Mainly contains a pointer to the actual data with component, name and type metadata
     * Any object having a toJSON method can be used as internal data and wrapped into an Entity
     *
     * This toJSON method should generate a json dictionnary with a "type" entry of type string
     * and as many others as entries as needed. Entity producers are thus free to add their own entries
     * provided they provide a type one. If the type value contains a ':' character, then the part
     * preceding it will be considered as a namespace. The rest is free text.
     * The type in json should match the type member of the Entity. It is used by Sink instances
     * to decide if they have to handle a given entity or not.
     * It can also be used to know which fields to expect in the json dictionnary
     */
    struct Entity {
      template <typename T>
      Entity( std::string component, std::string name, std::string type, const T& ent )
          : component{std::move( component )}
          , name{std::move( name )}
          , type{std::move( type )}
          , ptr{&ent}
          , getJSON{[]( const void* ptr ) { return reinterpret_cast<const T*>( ptr )->toJSON(); }} {}
      /// name of the component owning the Entity
      std::string component;
      /// name of the entity
      std::string name;
      /// type of the entity, see comment above concerning its format and usage
      std::string type;
      /// pointer to the actual data inside this Entity
      const void* ptr{nullptr};
      /// function converting the internal data to json. Due to type erasure, it needs to be a member of this struct
      json ( *getJSON )( const void* );
      json toJSON() const { return ( *getJSON )( ptr ); }
    };

    /// Interface reporting services must implement.
    struct Sink {
      virtual void registerEntity( Entity ent ) = 0;
      virtual ~Sink()                           = default;
    };

    template <typename T>
    void registerEntity( std::string c, std::string n, std::string t, const T& ent ) {
      registerEntity( {std::move( c ), std::move( n ), std::move( t ), ent} );
    }
    void registerEntity( Entity ent ) {
      std::for_each( begin( m_sinks ), end( m_sinks ),
                     [ent]( auto sink ) { sink->registerEntity( std::move( ent ) ); } );
      m_entities.emplace_back( std::move( ent ) );
    }

    void addSink( Sink* sink ) {
      std::for_each( begin( m_entities ), end( m_entities ),
                     [sink]( Entity ent ) { sink->registerEntity( std::move( ent ) ); } );
      m_sinks.push_back( sink );
    }
    void removeSink( Sink* sink ) {
      auto it = std::find( begin( m_sinks ), end( m_sinks ), sink );
      if ( it != m_sinks.end() ) m_sinks.erase( it );
    }

  private:
    std::deque<Sink*>  m_sinks;
    std::deque<Entity> m_entities;
  };
} // namespace Gaudi::Monitoring
