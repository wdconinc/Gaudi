//  ====================================================================
//  StoreSnifferAlg.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"

using namespace std;

/**@class StoreSnifferAlg
  *
  * Small algorithm, which traverses the data store and
  * prints a summary of the leafs accessed during the run.
  *
  * @author:  M.Frank
  * @version: 1.0
  */
class StoreSnifferAlg : public Algorithm {
public:

  SmartIF<IDataManagerSvc> m_mgr;

  struct LeafInfo {
    int  count;
    int  id;
    CLID clid;
  };
  typedef map<string, LeafInfo> SniffInfo;
  typedef map<string,map<int,int> > Correlations;

  SniffInfo    m_info, m_curr;
  Correlations m_corr, m_links;

  /// Standard algorithm constructor
  StoreSnifferAlg(const string& name, ISvcLocator* pSvc) : Algorithm(name, pSvc)  
  { }
  /// Standard Destructor
  ~StoreSnifferAlg() override = default;

  size_t explore(IRegistry* pObj)    {
    if ( pObj )    {
      SmartIF<IDataManagerSvc> mgr(eventSvc());
      if ( mgr )    {
        vector<IRegistry*> leaves;
        StatusCode sc = m_mgr->objectLeaves(pObj, leaves);
        if ( sc.isSuccess() )  {
          for (auto& pReg : leaves ) {
            const string& id = pReg->identifier();
            /// We are only interested in leaves with an object
            if ( pReg->address() && pReg->object() )  {
              auto j=m_info.find(id);
              if ( j == m_info.end() )   {
                m_info[id] = LeafInfo();
                j = m_info.find(id);
                (*j).second.count = 0;
                (*j).second.id    = m_info.size();
                (*j).second.clid  = pReg->object()->clID();
              }
              m_curr[id].id    = m_info[id].id;
              m_curr[id].count = explore(pReg);
            }
          }
          return leaves.size();
        }
      }
    }
    return 0;
  }

  /// Initialize
  StatusCode initialize()   override {
    m_info.clear();
    m_mgr = eventSvc();
    return StatusCode::SUCCESS;
  }

  /// Finalize
  StatusCode finalize() override {
    MsgStream log(msgSvc(), name());
    log << MSG::ALWAYS << "== BEGIN ============= Access list content:" << m_info.size() << endmsg;
    for(const auto&  i : m_info) {
      const LeafInfo& info = i.second;
      log << "== ITEM == " << right << setw(4) << dec << info.id << " clid:"
          << right << setw(8) << hex << info.clid << " Count:"
          << right << setw(6) << dec << info.count << " "
          << i.first+":"
          << endmsg;
      auto c=m_corr.find(i.first);
      if ( c != m_corr.end() ) {
        int cnt = 0;
        log << "== CORRELATIONS:" << (*c).second.size() << endmsg;
        for(const auto&  k : c->second) {
          if ( k.second > 0 ) {
            log << dec << k.first << ":" << k.second << "  ";
            if ( ++cnt == 10 ) {
              cnt = 0;
              log << endmsg;
            }
          }
        }
        if ( cnt > 0 ) log << endmsg;
      }
      auto l=m_links.find(i.first);
      if ( l != m_links.end() ) {
        int cnt = 0;
        log << "== LINKS:" << l->second.size() << endmsg;
        for(const auto& k : l->second) {
          if ( k.second > 0 ) {
            log << dec << k.first << ":" << k.second << "  ";
            if ( ++cnt == 10 ) {
              cnt = 0;
              log << endmsg;
            }
          }
        }
        if ( cnt > 0 ) log << endmsg;
      }
    }
    log << MSG::ALWAYS << "== END =============== Access list content:" << m_info.size() << endmsg;
    m_info.clear();
    m_mgr = nullptr;
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  StatusCode execute() override   {
    SmartDataPtr<DataObject> root(eventSvc(),"/Event");
    if ( root )    {
      m_curr.clear();
      auto& evnt = m_curr["/Event"];
      evnt.count = explore(root->registry());
      evnt.clid  = root->clID();
      evnt.id    = m_curr.size();
      for(const auto & i : m_curr ) m_info[i.first].count++;
      for(const auto & i : m_info ) {
        const string& nam = i.first;
        // const LeafInfo& leaf = (*i).second;
        auto c=m_corr.find(nam);
        if ( c == m_corr.end() )  {
          m_corr[nam] = { };
          c = m_corr.find(nam);
        }
        for(const auto& l : m_curr) {
          const LeafInfo& li = l.second;
          auto k = c->second.find(li.id);
          if ( k==c->second.end() ) c->second[li.id] = 0;
          ++(c->second[li.id]);
        }

        c=m_links.find(nam);
        if ( c == m_links.end() )  {
          m_links[nam] = { };
          c = m_links.find(nam);
        }
        if ( m_curr.find(nam) != m_curr.end() ) {
          SmartDataPtr<DataObject> obj(eventSvc(),nam);
          if ( obj ) {
            LinkManager* m = obj->linkMgr();
            for(long l=0; l<m->size(); ++l) {
              auto* lnk=m->link(l);
              auto il=m_curr.find(lnk->path());
              // cout << "Link:" << lnk->path() << " " << (char*)(il != m_curr.end() ? "Found" : "Not there") << endl;
              if ( il != m_curr.end() ) {
                if ( lnk->object() ) {
                  const LeafInfo& li = (*il).second;
                  auto k = c->second.find(li.id);
                  if ( k==c->second.end() ) c->second[li.id] = 0;
                  ++c->second[li.id];
                }
              }
            }
          }
        }
      }
      return StatusCode::SUCCESS;
    }
    return StatusCode::SUCCESS;
  }
};

DECLARE_COMPONENT(StoreSnifferAlg)
