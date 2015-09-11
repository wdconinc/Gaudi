//====================================================================
//	EventSelectorDataStream.cpp
//--------------------------------------------------------------------
//
//	Package    : EventSelectorDataStream  (The LHCb Event Selector Package)
//
//
//	Author     : M.Frank
//      Created    : 4/10/00
//	Changes    : R. Lambert 2009-09-04
//
//====================================================================
#define GAUDISVC_EVENTSELECTOR_EVENTSELECTORDATASTREAM_CPP 1
// Include files
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IPersistencySvc.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/EventSelectorDataStream.h"


// Output friend
MsgStream& operator<<(MsgStream& s, const EventSelectorDataStream& obj)    {
  s << "Stream:"   << obj.name() << " Def:" << obj.definition();
  return s;
}

// Output friend
std::ostream& operator<<(std::ostream& s, const EventSelectorDataStream& obj)    {
  s << "Stream:"   << obj.name() << " Def:" << obj.definition();
  return s;
}

// Standard Constructor
EventSelectorDataStream::EventSelectorDataStream(const std::string& nam, const std::string& def, ISvcLocator* svcloc)
: m_name{ nam },
  m_definition{ def },
  m_pSelector(0),
  m_pSvcLocator(svcloc),
  m_initialized{  false }
{
}

// Standard Constructor
EventSelectorDataStream::~EventSelectorDataStream()   {
  setSelector(0);
}

// Set selector
void EventSelectorDataStream::setSelector(IEvtSelector* pSelector)   {
  if ( pSelector   )  pSelector->addRef();
  if ( m_pSelector )  m_pSelector->release();
  m_pSelector = pSelector;
}

// Allow access to individual properties by name
StringProperty* EventSelectorDataStream::property(const std::string& nam)    {
  for ( auto& i : m_properties ) {
    if ( i.name() == nam ) return &i;
  }
  return nullptr;
}

// Allow access to individual properties by name
const StringProperty* EventSelectorDataStream::property(const std::string& nam)   const  {
  for ( auto& i : m_properties ) { 
    if ( i.name() == nam ) return &i;
  }
  return nullptr;
}

// Parse input criteria
StatusCode EventSelectorDataStream::initialize()   {
  bool isData = true;
  std::string auth, dbtyp, collsvc, item, crit, sel, svc, stmt;
  std::string cnt    = "/Event";
  std::string db     = "<Unknown>";

  auto eds = m_pSvcLocator->service<IDataManagerSvc>("EventDataSvc");
  if( !eds ) {
    std::cout << "ERROR: Unable to localize interface IDataManagerSvc from service EventDataSvc"
              << std::endl;
    return StatusCode::FAILURE;
  }
  else {
    cnt = eds->rootName();
  }
  m_selectorType = m_criteria = m_dbName= "";
  m_properties.clear();

  using Parser = Gaudi::Utils::AttribStringParser;
  for (auto attrib: Parser(m_definition)) {
    long hash = -1;
    switch( ::toupper(attrib.tag[0]) )    {
    case 'A':
      auth = std::move(attrib.value);
      break;
    case 'C':
      svc  = "EvtTupleSvc";
      isData = false;
      /* no break */
    case 'E':
      hash = attrib.value.find('#');
      if ( hash > 0 )   {
        cnt  = attrib.value.substr(0, hash);
        item = attrib.value.substr(hash + 1);
      }
      else    {
        cnt  = std::move(attrib.value);
        item = "Address";
      }
      break;
    case 'D':
      m_criteria     = "FILE " + attrib.value;
      m_dbName = std::move(attrib.value);
      break;
    case 'F':
      switch( ::toupper(attrib.tag[1]) )    {
      case 'I':
        m_criteria   = "FILE " + attrib.value;
	m_dbName = std::move(attrib.value);
        break;
      case 'U':
        stmt = std::move(attrib.value);
        break;
      default:
        break;
      }
      break;
    case 'J':
      m_criteria     = "JOBID " + attrib.value;
      m_dbName = std::move(attrib.value);
      dbtyp          = "SICB";
      break;
    case 'T':
      switch( ::toupper(attrib.tag[1]) )    {
      case 'Y':
        dbtyp = std::move(attrib.value);
        break;
      default:
        break;
      }
      break;
    case 'S':
      switch( ::toupper(attrib.tag[1]) )    {
      case 'E':
        sel = std::move(attrib.value);
        break;
      case 'V':
        svc = std::move(attrib.value);
	collsvc = svc;
        break;
      default:
        break;
      }
      break;
    default:
      m_properties.emplace_back(attrib.tag, attrib.value);
      break;
    }
  }
  if ( !isData )    { // Unfortunately options do not come in order...
    m_selectorType = "EventCollectionSelector";
    svc  = "EvtTupleSvc";
  }
  else if ( dbtyp.compare(0,4,"POOL") == 0 )    {
    m_selectorType = "PoolDbEvtSelector";
  }
  else if ( svc.empty() ) {
    m_selectorType = "DbEvtSelector";
  }
  else  {
    m_selectorType = svc;
  }
  StatusCode status = StatusCode::SUCCESS;
  if ( svc.empty() && !dbtyp.empty() )    {
    auto ipers = m_pSvcLocator->service<IPersistencySvc>("EventPersistencySvc");
    if ( ipers )   {
      IConversionSvc* icnvSvc = nullptr;
      status = ipers->getService(dbtyp, icnvSvc);
      if ( status.isSuccess() )   {
        IService* isvc = nullptr;
        status = icnvSvc->queryInterface(IService::interfaceID(), pp_cast<void>(&isvc));
        if ( status.isSuccess() )   {
          svc = isvc->name();
          isvc->release();
        }
      }
    }
  }
  m_properties.emplace_back( "Function",      stmt);
  m_properties.emplace_back( "CnvService",    svc);
  m_properties.emplace_back( "Authentication",auth);
  m_properties.emplace_back( "Container",     cnt);
  m_properties.emplace_back( "Item",          item);
  m_properties.emplace_back( "Criteria",      sel);
  m_properties.emplace_back( "DbType",        dbtyp);
  m_properties.emplace_back( "DB",            m_criteria);
  if ( !isData && !collsvc.empty() )    {
    m_properties.emplace_back( "DbService",   collsvc);
  }

  m_initialized = status.isSuccess();
  return status;
}

// Parse input criteria
StatusCode EventSelectorDataStream::finalize()   {
  setSelector(0);
  m_properties.clear();
  m_initialized = false;
  return StatusCode::SUCCESS;
}
