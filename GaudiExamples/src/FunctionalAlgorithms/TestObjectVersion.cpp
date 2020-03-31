/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiAlg/Consumer.h>
#include <GaudiAlg/Producer.h>
#include <GaudiKernel/KeyedContainer.h>

namespace Gaudi {
  namespace Examples {
    namespace TestObjectVersion {
      // using ObjectType = DataObject;
      using ObjectType = KeyedContainer<KeyedObject<std::size_t>>;

      struct CreateObject : Gaudi::Functional::Producer<CreateObject, ObjectType()> {
        CreateObject( const std::string& name, ISvcLocator* svcLoc )
            : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/SomeData" ) ) {}
        ObjectType operator()() const{
          ObjectType o;
          o.setVersion( 42 );
          info() << "Created object with version " << static_cast<int>( o.version() ) << endmsg;
          return o;
        }
      };
      DECLARE_COMPONENT( CreateObject )

      struct UseObject : Gaudi::Functional::Consumer<UseObject, void( const ObjectType& )> {
        UseObject( const std::string& name, ISvcLocator* svcLoc )
            : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/SomeData" ) ) {}
        void operator()( const ObjectType& o ) const {
          info() << "Retrieved object with version " << static_cast<int>( o.version() ) << endmsg;
          if ( o.version() != 42 ) throw GaudiException( "Wrong object version", name(), StatusCode::FAILURE );
        }
      };
      DECLARE_COMPONENT( UseObject )
    } // namespace TestObjectVersion
  }   // namespace Examples
} // namespace Gaudi
