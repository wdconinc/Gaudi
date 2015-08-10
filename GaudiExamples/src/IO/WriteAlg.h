#ifndef GAUDIEXAMPLE_WRITEALG_H
#define GAUDIEXAMPLE_WRITEALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiExamples/Counter.h"

/** @class WriteAlg WriteAlg.h

    WriteAlg class for the RootIOExample

    @author Markus Frank
*/

class WriteAlg : public Algorithm {
 protected:
  /// Reference to run records data service
  IDataProviderSvc* m_recordSvc;

  /// Reference to event counter
  Gaudi::Examples::Counter* m_evtCount;

  /// Register data leaf
  StatusCode put(IDataProviderSvc* s, const std::string& path, DataObject* pObj);

 public:
  /// Constructor: A constructor of this form must be provided.
  WriteAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : Algorithm(name, pSvcLocator), m_recordSvc(nullptr), m_evtCount(nullptr) { }
  /// Standard Destructor
  virtual ~WriteAlg() { }
  /// Initialize
  virtual StatusCode initialize();
  /// Finalize
  virtual StatusCode finalize();
  /// Event callback
  virtual StatusCode execute();
};

#endif // GAUDIEXAMPLE_WRITEALG_H
