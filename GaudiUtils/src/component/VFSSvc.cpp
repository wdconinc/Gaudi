#include "VFSSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IAlgTool.h"

DECLARE_COMPONENT(VFSSvc)

//------------------------------------------------------------------------------
VFSSvc::VFSSvc(const std::string& name, ISvcLocator* svc):
  base_class(name,svc)
{
  declareProperty("FileAccessTools",m_urlHandlersNames,
                  "List of tools implementing the IFileAccess interface.");

  declareProperty("FallBackProtocol",m_fallBackProtocol = "file",
                  "URL prefix to use if the prefix is not present.");
}
//------------------------------------------------------------------------------
StatusCode VFSSvc::initialize() {
  StatusCode sc = Service::initialize();
  if (sc.isFailure()) return sc;


  m_toolSvc = serviceLocator()->service("ToolSvc");
  if (!m_toolSvc){
    error() << "Cannot locate ToolSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  IAlgTool *tool;
  for(const auto& i : m_urlHandlersNames) {
    // retrieve the tool and the pointer to the interface
    sc = m_toolSvc->retrieve(i,IAlgTool::interfaceID(),tool,nullptr,true);
    if (sc.isFailure()){
      error() << "Cannot get tool " << i << endmsg;
      return sc;
    }
    m_acquiredTools.push_back(tool); // this is one tool that we will have to release
    auto hndlr = SmartIF<IFileAccess>(tool);
    if (!hndlr){
      error() << i << " does not implement IFileAccess" << endmsg;
      return StatusCode::FAILURE;
    }
    // We do not need to increase the reference count for the IFileAccess interface
    // because we hold the tool by its IAlgTool interface.
    // loop over the list of supported protocols and add them to the map (for quick access)
    for ( const auto& prot : hndlr->protocols() ) m_urlHandlers[prot] = hndlr.get();
  }

  // Now let's check if we can handle the fallback protocol
  if ( m_urlHandlers.find(m_fallBackProtocol) == m_urlHandlers.end() ) {
    error() << "No handler specified for fallback protocol prefix "
        << m_fallBackProtocol << endmsg;
    return StatusCode::FAILURE;
  }

  // Note: the list of handled protocols will be filled only if requested

  return sc;
}
//------------------------------------------------------------------------------
StatusCode VFSSvc::finalize() {
  m_urlHandlers.clear(); // clear the map
  m_protocols.clear();

  if (m_toolSvc) {
    // release the acquired tools (from the last acquired one)
    while ( !m_acquiredTools.empty()  ){
      m_toolSvc->releaseTool(m_acquiredTools.back()).ignore();
      m_acquiredTools.pop_back();
    }
    m_toolSvc.reset() ; // release the tool service
  }
  return Service::finalize();
}
//------------------------------------------------------------------------------
std::unique_ptr<std::istream> VFSSvc::open(const std::string &url){

  // get the url prefix endpos
  auto pos = url.find("://");

  if (std::string::npos == pos) { // no url prefix
    return m_urlHandlers[m_fallBackProtocol]->open(url);
  }

  std::string url_prefix(url,0,pos);
  if ( m_urlHandlers.find(url_prefix) == m_urlHandlers.end() ) {
    // if we do not have a handler for the URL prefix,
    // use the fall back one and pass only the path
    return m_urlHandlers[m_fallBackProtocol]->open(url.substr(pos+3));
  }
  return m_urlHandlers[url_prefix]->open(url);
}
//------------------------------------------------------------------------------
namespace {
  /// small helper to select  the first element of a pair
  /// (e.g. the key of a map value type)
  constexpr struct select1st_t
  {
    template <typename S, typename T>
    const S& operator() (const std::pair<S,T> &x) const { return x.first; }
  } select1st {} ;
}

const std::vector<std::string> &VFSSvc::protocols() const
{
  if (m_protocols.empty()){
    // prepare the list of handled protocols
    std::transform( m_urlHandlers.begin(),m_urlHandlers.end(),
                    std::back_inserter(const_cast<VFSSvc*>(this)->m_protocols),
                    select1st );
  }
  return m_protocols;
}
