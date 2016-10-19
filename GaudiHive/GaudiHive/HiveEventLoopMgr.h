#ifndef GAUDIHIVE_HIVEEVENTLOOPMGR_H
#define GAUDIHIVE_HIVEEVENTLOOPMGR_H 1

// Framework include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

// std includes
#include <atomic>

//include boost
#include <boost/dynamic_bitset.hpp>

// include tbb
#include "tbb/concurrent_vector.h"
#include "tbb/concurrent_queue.h"

// typedef for the event and algo state
typedef boost::dynamic_bitset<> state_type;

// Forward declarations
class IIncidentSvc;
class IDataManagerSvc;
class IDataProviderSvc;

namespace tbb {
  class task_scheduler_init;
}

class HiveEventLoopMgr : public MinimalEventLoopMgr   {
public:

protected:
  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc>  m_evtDataMgrSvc;
  /// Reference to the Event Data Service's IDataProviderSvc interface
  SmartIF<IDataProviderSvc> m_evtDataSvc;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector>     m_evtSelector;
  /// Event Iterator
  IEvtSelector::Context*    m_evtSelContext;
  /// Event selector
  std::string       m_evtsel;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc>  m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc>   m_histoPersSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IHiveWhiteBoard>  m_whiteboard;
  /// Reference to the Algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;
  /// Name of the Hist Pers type
  std::string       m_histPersName;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;
  /// Algorithm Execution State Mgr
  SmartIF<IAlgExecStateSvc>      m_aess;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool              m_endEventFired;
  /// Flag to disable warning messages when using external input
  bool              m_warnings;
  
  // Variables for the concurrency  
  /// Maximum number of parallel running algorithms
  unsigned int m_max_parallel;
  /// Pointer to tbb task scheduler
  tbb::task_scheduler_init* m_tbb_scheduler_init;  
  /// Get the input and output collections
  void find_dependencies();
  /// The termination requirement
  state_type m_termination_requirement;
  /// All requirements
  std::vector<state_type> m_all_requirements;
  /// Register of input products
  std::map<DataObjID,unsigned int> m_product_indices;
  /// Total number of algos in flight across all events
  std::atomic_uint m_total_algos_in_flight;
  /// Total number of algos
  unsigned int  m_numberOfAlgos;
  /// Dump the algorithm queues
  bool m_DumpQueues;
  /// Number of events in parallel
  unsigned int m_evts_parallel;
  /// Total numbers of threads
  unsigned int m_num_threads;
  /// Clone algorithms to run them simultaneously
  bool m_CloneAlgorithms;
  /// Algorithms Inputs
  // keep room for a class hashing strings instead of strings
  typedef std::vector<std::vector<std::string>> algosDependenciesCollection;
  // We just need the dependencies and not the algo names.
  algosDependenciesCollection m_AlgosDependencies;
  // Number of products to deal with
  unsigned int m_nProducts;

public:
  /// Standard Constructor
  HiveEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  ~HiveEventLoopMgr() override;
  /// Create event address using event selector
  StatusCode getEventRoot(IOpaqueAddress*& refpAddr);  
  
  /// implementation of IService::initialize
  StatusCode initialize() override;
  /// implementation of IService::reinitialize
  StatusCode reinitialize() override;
  /// implementation of IService::stop
  StatusCode stop() override;
  /// implementation of IService::finalize
  StatusCode finalize() override;
  /// implementation of IService::nextEvent
  StatusCode nextEvent(int maxevt) override;
  /// implementation of IEventProcessor::executeEvent(void* par)
  StatusCode executeEvent(void* par) override;
  /// implementation of IEventProcessor::executeRun()
  StatusCode executeRun(int maxevt) override;

  /// Decrement the number of algos in flight and put algo back in manager - maybe private
  void taskFinished(IAlgorithm*& algo);

};
#endif // GAUDIHIVE_HIVEEVENTLOOPMGR_H
